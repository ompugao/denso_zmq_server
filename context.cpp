#include "context.h"
#include <algorithm>
#include <thread>

namespace middleware {

void signal_handler(int signal)
{
    std::cerr << "signal handler called" << std::endl;
    g_shutdown = true;
}

void Context::spin() {
    while (isok()) {
        for (auto&& sub : subscribers_) {
            sub->receive();
        }
        std::this_thread::yield();
    }
}

SubscriberHandle::~SubscriberHandle() {
    if (!context_) {
        std::cout << "not well-defined subscirberhandle" << std::endl;
        return;
    }
    std::cout << "unsubscribing..." << std::endl;
    std::vector<boost::shared_ptr<SubscriberBase>>::iterator itr =
        std::find(context_->subscribers_.begin(), context_->subscribers_.end(), sub_);
    if (itr != context_->subscribers_.end()) {
        context_->subscribers_.erase(itr);
    }
    sub_.reset();
};

} // namespace middleware
