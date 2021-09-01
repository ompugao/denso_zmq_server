#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <boost/thread.hpp>
#include <atomic>

#include "message_types/service_msgs.h"
#include "middleware/context.h"
#include "middleware/publisher.h"

int main(int argc, char * argv[])
{
    boost::shared_ptr<middleware::Context> context(new middleware::Context());
    auto client = context->serviceClient("ipc://@denso/test_service");
    for (int i = 0; i < 1000; i++) {
        message_types::TestServiceRequest req;
        message_types::TestServiceResponse res;
        req.values.push_back(i);
        bool ok = client->call(req, res);
        if (ok) {
            std::cout << res.message << std::endl;
        }
    }

    context->spin();
    return 0;
}
