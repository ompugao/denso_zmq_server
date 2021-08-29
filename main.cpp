#include <iostream>
#include <vector>
#include <string>
#include <denso_control/rc8_controller_interface.h>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/array.hpp>
#include <atomic>

#include "message_types/joint_values.h"
#include "middleware/context.h"
#include "middleware/subscriber.h"
#include "middleware/publisher.h"

template<int NUM_JOINTS> 
class DENSOZMQServer {
public:
    DENSOZMQServer(const boost::shared_ptr<middleware::Context>& context, const std::string& ip_address):
        context_(context), b_stop_thread_(false), command_queue_(3) {
        denso_controller_.reset(new denso_control::RC8ControllerInterface(ip_address));
        pub_ = context->advertise("ipc://@denso/joint_values");
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

protected:
    void _CallbackJointValues(const boost::shared_ptr<message_types::JointValues>& msg) {
        boost::array<double, NUM_JOINTS> cmd;
        if (msg->values.size() != NUM_JOINTS) {
            std::cerr << "Invalid number of joint values" << std::endl;
            return;
        }
        std::copy(std::begin(msg->values), std::end(msg->values), cmd.begin());
        while(!command_queue_.bounded_push(cmd)) {
            std::this_thread::yield();
        };
    }
    void _ControlThread() {
        message_types::JointValues joint_values;
        boost::array<double, NUM_JOINTS> cmd_pos_arr;
        std::vector<double> cmd_pos(NUM_JOINTS);
        joint_values.values.resize(NUM_JOINTS);

        bool result = denso_controller_->startMotors(100.0);
        denso_controller_->update();
        denso_controller_->getJointPositions(joint_values.values);
        denso_controller_->getJointPositions(cmd_pos);

        pub_->publish(joint_values);
        std::chrono::high_resolution_clock::time_point tloopstart_;
        while(context_->isok()) {
            if (b_stop_thread_) {
                break;
            }
            tloopstart_ = std::chrono::high_resolution_clock::now();
            std::cout << std::fixed << std::chrono::duration_cast<std::chrono::milliseconds>(tloopstart_.time_since_epoch()).count() << std::endl;
            if (command_queue_.pop(cmd_pos_arr)) {
                std::copy(cmd_pos_arr.begin(), cmd_pos_arr.end(), cmd_pos.begin());
            }
            if (denso_controller_->getErrorCode() != 0) {
                std::this_thread::yield();
                break;
            }
            if (denso_controller_->motorsON()) {
                denso_controller_->setJointPositions(cmd_pos);
            }
            denso_controller_->update();
            denso_controller_->getJointPositions(joint_values.values);

            pub_->publish(joint_values);
            std::this_thread::sleep_until(tloopstart_ + std::chrono::milliseconds(8));
        }
    }
    boost::shared_ptr<middleware::Context> context_;
    boost::shared_ptr<middleware::SubscriberHandle> sub_;
    boost::shared_ptr<middleware::Publisher> pub_;
    //boost::unique_ptr<SubscriberBase> sub_;
    boost::shared_ptr<boost::thread> thread_;
    std::atomic<bool> b_stop_thread_;
    boost::shared_ptr<denso_control::RC8ControllerInterface> denso_controller_;
    boost::lockfree::queue<boost::array<double, NUM_JOINTS>> command_queue_;
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
    DENSOZMQServer<6> server(context, vm["ip_address"].as<std::string>());

    context->spin();
    return 0;
}
