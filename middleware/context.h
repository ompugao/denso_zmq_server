#pragma once
#include <atomic>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <csignal>

#include <zmqpp/zmqpp.hpp>
#include "subscriber.h"
#include "publisher.h"

namespace middleware {

static std::atomic<bool> g_shutdown(false);
void signal_handler(int signal);

class Context;

class SubscriberHandle {
public:
    SubscriberHandle() {
    }
    SubscriberHandle(const boost::shared_ptr<Context>& context, const boost::shared_ptr<SubscriberBase>& sub);
    virtual ~SubscriberHandle();
protected:
    boost::shared_ptr<Context> context_;
    boost::shared_ptr<SubscriberBase> sub_;
};

class Context : public boost::enable_shared_from_this<Context> {
public:
    Context() {
        std::signal(SIGINT, signal_handler);
    }
    virtual ~Context() {
    }
    template<typename Msg> 
    boost::shared_ptr<SubscriberHandle> subscribe(const std::string& endpoint, 
            boost::function<void(const boost::shared_ptr<Msg>&)> callback,
            const bool blocking = false) {
        boost::shared_ptr<SubscriberBase> sub = boost::make_shared<Subscriber<Msg>>(this->zmqcontext_, endpoint, callback, blocking);
        this->subscribers_.push_back(sub);
        return boost::make_shared<SubscriberHandle>(shared_from_this(), sub);
    }

    boost::shared_ptr<Publisher> advertise(const std::string& endpoint) {
        return boost::make_shared<Publisher>(this->zmqcontext_, endpoint);
    }

    bool isok() {
        return !g_shutdown;
    }
    void spin();
    void spinonce();
    friend class SubscriberHandle;
protected:
    zmqpp::context zmqcontext_;
    std::vector<boost::shared_ptr<SubscriberBase>> subscribers_;
};


} // namespace middleware
