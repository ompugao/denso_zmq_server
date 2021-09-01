#pragma once
#include <atomic>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <csignal>

#include <zmqpp/zmqpp.hpp>
#include "subscriber.h"
#include "publisher.h"
#include "serviceserver.h"
#include "serviceclient.h"

namespace middleware {

namespace init_options {
enum InitOptions {
    NoSignalHandler = 0x01
};
} // namespace init_options
using InitOptions = init_options::InitOptions;

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

class ServiceServerHandle {
public:
    ServiceServerHandle() {
    }
    ServiceServerHandle(const boost::shared_ptr<Context>& context, const boost::shared_ptr<ServiceServerBase>& sub);
    virtual ~ServiceServerHandle();
protected:
    boost::shared_ptr<Context> context_;
    boost::shared_ptr<ServiceServerBase> server_;
};

class Context : public boost::enable_shared_from_this<Context> {
public:
    Context(int options=0) {
        if (!(options && InitOptions::NoSignalHandler)) {
            std::signal(SIGINT, signal_handler);
        }
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

    template<typename Request, typename Response> 
    boost::shared_ptr<ServiceServerHandle> advertiseServer(const std::string& endpoint, 
            boost::function<bool(const boost::shared_ptr<Request>&, boost::shared_ptr<Response>&)> callback) {
        boost::shared_ptr<ServiceServerBase> server = boost::make_shared<ServiceServer<Request, Response>>(this->zmqcontext_, endpoint, callback);
        this->serviceservers_.push_back(server);
        return boost::make_shared<ServiceServerHandle>(shared_from_this(), server);
    }

    boost::shared_ptr<ServiceClient> serviceClient(const std::string& endpoint) {
        return boost::make_shared<ServiceClient>(this->zmqcontext_, endpoint);
    }

    bool isok() {
        return !g_shutdown;
    }
    void spinonce();
    void spin();
    friend class SubscriberHandle;
    friend class ServiceServerHandle;
protected:
    zmqpp::context zmqcontext_;
    std::vector<boost::shared_ptr<SubscriberBase>> subscribers_;
    std::vector<boost::shared_ptr<ServiceServerBase>> serviceservers_;
};


} // namespace middleware
