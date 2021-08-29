#include <iostream>
#include <vector>
#include <string>
#include <denso_control/rc8_controller_interface.h>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <atomic>

#include "message_types/joint_values.h"
#include "context.h"
#include "subscriber.h"
#include "publisher.h"

class DENSOZMQServer {
public:
    DENSOZMQServer(const boost::shared_ptr<middleware::Context>& context, const std::string& ip_address):
        context_(context), b_stop_thread_(false) {
        denso_controller_.reset(new denso_control::RC8ControllerInterface(ip_address));
        sub_ = context->subscribe<message_types::JointValues>("ipc://@denso/command", 
                    boost::bind(&DENSOZMQServer::_CallbackJointValues, this, _1), /*blocking=*/ false);
        if (!denso_controller_->connect()) {
            return;
        }
        if (denso_controller_->getErrorCode() != 0) {
            denso_controller_->clearError();
        }
        thread_.reset(new boost::thread(boost::bind(&DENSOZMQServer::_ControlThread, this)));
    }
    virtual ~DENSOZMQServer() {
        b_stop_thread_ = true;
        if (!!thread_) {
            thread_->join();
        }
        sub_.reset();
        if (denso_controller_->getErrorCode() != 0) {
            denso_controller_->clearError();
        }
        if (denso_controller_->motorsON()) {
            denso_controller_->stopMotors();
        }
        if (denso_controller_->getErrorCode() != 0) {
            denso_controller_->clearError();
        }
        std::cout << "disconnect" << std::endl;
        denso_controller_->disconnect();
    }
    void _CallbackJointValues(const boost::shared_ptr<message_types::JointValues>& msg) {
    }
    void _ControlThread() {
        std::vector<double> js_position_(6), cmd_pos(6);

        bool result = denso_controller_->startMotors(100.0);
        denso_controller_->update();
        denso_controller_->getJointPositions(js_position_);
        denso_controller_->getJointPositions(cmd_pos);


        std::chrono::high_resolution_clock::time_point tloopstart_;
        while(context_->isok()) {
            tloopstart_ = std::chrono::high_resolution_clock::now();
            std::cout << std::fixed << std::chrono::duration_cast<std::chrono::milliseconds>(tloopstart_.time_since_epoch()).count() << std::endl;
            if (denso_controller_->getErrorCode() != 0) {
                break;
            }
            if (denso_controller_->motorsON()) {
                denso_controller_->setJointPositions(cmd_pos);
            }
            denso_controller_->update();
            denso_controller_->getJointPositions(js_position_);

            std::this_thread::sleep_until(tloopstart_ + std::chrono::milliseconds(8));
        }

    }
    boost::shared_ptr<middleware::Context> context_;
    boost::shared_ptr<middleware::SubscriberHandle> sub_;
    //boost::unique_ptr<SubscriberBase> sub_;
    boost::shared_ptr<boost::thread> thread_;
    std::atomic<bool> b_stop_thread_;
    boost::shared_ptr<denso_control::RC8ControllerInterface> denso_controller_;
};

int main(int argc, char * argv[])
{

    int dummyargc = 0;
    ros::init(dummyargc, nullptr, "denso_zmq_server",  ros::init_options::NoSigintHandler);
    ros::Time::init();
    struct sched_param param;
    // do not use 99 for sched_priority
    // https://rt.wiki.kernel.org/index.php/HOWTO:_Build_an_RT-application#Priority_99
    param.sched_priority = 97;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        ROS_WARN("failed to set scheduler priority");
        return 1;
    }
            
    std::string ip_address;
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
        // First parameter describes option name/short name
        // The second is parameter to option
        // The third is description
        ("help,h", "help")
        ("ip_address,a", po::value(&ip_address)->default_value("192.168.0.21"), "ip address of a denso controller")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {  
        std::cout << desc << std::endl;
        return 0;
    }

    if (!vm.count("ip_address")) {
        std::cout << "IP address is not set" << std::endl;
        return 1;
    }
    boost::shared_ptr<middleware::Context> context(new middleware::Context());
    DENSOZMQServer server(context, vm["ip_address"].as<std::string>());

    context->spin();
    //std::vector<double> js_position_(6), cmd_pos(6);
    return 0;
}
