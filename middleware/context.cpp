#include "context.h"
#include <algorithm>
#include <thread>

namespace middleware {

void signal_handler(int signal)
{
    std::cerr << "signal handler called" << std::endl;
    g_shutdown = true;
}

void Context::spinonce() {
    for (auto&& sub : subscribers_) {
        sub->receive();
    }
    for (auto&& server : serviceservers_) {
        server->receive();
    }
}

void Context::spin() {
    while (isok()) {
        spinonce();
        // std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
}

SubscriberHandle::SubscriberHandle(const boost::shared_ptr<Context>& context, const boost::shared_ptr<SubscriberBase>& sub): context_(context), sub_(sub) {
}
SubscriberHandle::~SubscriberHandle() {
    if (!context_) {
        return;
    }
    std::vector<boost::shared_ptr<SubscriberBase>>::iterator itr =
        std::find(context_->subscribers_.begin(), context_->subscribers_.end(), sub_);
    if (itr != context_->subscribers_.end()) {
        std::cout << "unsubscribing.." << std::endl;
        context_->subscribers_.erase(itr);
    }
    sub_.reset();
};

ServiceServerHandle::ServiceServerHandle(const boost::shared_ptr<Context>& context, const boost::shared_ptr<ServiceServerBase>& server): context_(context), server_(server) {
}
ServiceServerHandle::~ServiceServerHandle() {
    if (!context_) {
        return;
    }
    std::vector<boost::shared_ptr<ServiceServerBase>>::iterator itr =
        std::find(context_->serviceservers_.begin(), context_->serviceservers_.end(), server_);
    if (itr != context_->serviceservers_.end()) {
        context_->serviceservers_.erase(itr);
    }
    server_.reset();
};

} // namespace middleware
