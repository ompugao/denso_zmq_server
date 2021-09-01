#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <atomic>

#include "message_types/service_msgs.h"
#include "middleware/context.h"
#include "middleware/subscriber.h"

bool callback(const boost::shared_ptr<message_types::TestServiceRequest>& req,
        boost::shared_ptr<message_types::TestServiceResponse>& res) {
    for (auto&& e : req->values) {
        std::cout << e << ",";
    }
    std::cout << std::endl;
    res->message = "good!";
    return true;
}

int main(int argc, char * argv[])
{

    boost::shared_ptr<middleware::Context> context(new middleware::Context());
    auto server = context->advertiseServer<message_types::TestServiceRequest, message_types::TestServiceResponse>("ipc://@denso/test_service", callback);
    context->spin();
    boost::thread thread(boost::bind(&middleware::Context::spin, context));
    thread.join();
    return 0;
}
