
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <atomic>

#include "message_types/joint_values.h"
#include "context.h"
#include "publisher.h"

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
    auto pub = context->advertise("ipc://@denso/hoge");
    // auto pub = context->advertise("tcp://*:4242");
    for (int i = 0; i < 100; i++) {
        message_types::JointValues msg;
        msg.values.resize(6);
        for (auto&& v : msg.values) {
            v = 2;
        }
        pub->publish(msg);
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    context->spin();
    return 0;
    }
