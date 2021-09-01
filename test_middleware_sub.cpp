
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <atomic>

#include "message_types/joint_values.h"
#include "message_types/joint_values_array.h"
#include "middleware/context.h"
#include "middleware/subscriber.h"

std::chrono::high_resolution_clock::time_point t_last =std::chrono::high_resolution_clock::now();

void _CallbackJointValues(const boost::shared_ptr<message_types::JointValuesArray>& msg) {
    auto t = std::chrono::high_resolution_clock::now();
    // std::cout << (static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t - t_last).count()) / 1e6);// << ": ";
    //for (auto&& v : msg->values) {
    //    std::cout << v << ", ";
    //}
    //std::cout << std::endl;
    auto diff = t - msg->timepoint;
    std::cout << (std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1e6) << ", ";// << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << std::endl;
    t_last = t;
}
int main(int argc, char * argv[])
{

    struct sched_param param;
    // do not use 99 for sched_priority
    // https://rt.wiki.kernel.org/index.php/HOWTO:_Build_an_RT-application#Priority_99
    param.sched_priority = 97;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        std::cerr << ("failed to set scheduler priority") << std::endl;
        return 1;
    }
            
    boost::shared_ptr<middleware::Context> context(new middleware::Context());
    boost::shared_ptr<middleware::SubscriberHandle> sub_ = context->subscribe<message_types::JointValuesArray>("ipc://@denso/joint_values_array", 
                    _CallbackJointValues, /*blocking=*/ false);
    context->spin();
    boost::thread thread(boost::bind(&middleware::Context::spin, context));
    thread.join();
    return 0;
}
