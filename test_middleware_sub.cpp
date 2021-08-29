
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <atomic>

#include "message_types/joint_values.h"
#include "context.h"
#include "subscriber.h"
void _CallbackJointValues(const boost::shared_ptr<message_types::JointValues>& msg) {
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << ": ";
    for (auto&& v : msg->values) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
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
    boost::shared_ptr<middleware::SubscriberHandle> sub_ = context->subscribe<message_types::JointValues>("ipc://@denso/hoge", 
                    _CallbackJointValues, /*blocking=*/ false);
    // boost::shared_ptr<middleware::SubscriberHandle> sub_ = context->subscribe<message_types::JointValues>("tcp://localhost:4242", 
    //                 _CallbackJointValues, /*blocking=*/ false);
    std::cout << "spin!" << std::endl;
    context->spin();
    return 0;
    }
