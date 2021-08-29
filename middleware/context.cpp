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

} // namespace middleware
