#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include <zmqpp/zmqpp.hpp>
#include <msgpack.hpp>

namespace middleware {
class SubscriberBase {
public:
    SubscriberBase() {
    }
    virtual ~SubscriberBase() {
    }
    virtual void receive() = 0;
};

template<typename Msg> 
class Subscriber : public SubscriberBase {
public:
    using CallbackFn = boost::function<void(const boost::shared_ptr<Msg>&)>;
    Subscriber(zmqpp::context& zmqcontext, const std::string& endpoint, 
            CallbackFn callback,
            const bool blocking = false) :
        SubscriberBase(), endpoint_(endpoint), callbackfn_(callback), blocking_(blocking) {
        zmqpp::socket_type type = zmqpp::socket_type::subscribe;
        socket_ = boost::make_shared<zmqpp::socket>(zmqcontext, type);
        //zmqpp::socket socket(zmqcontext, type);
        // Subscribe to the default channel
        socket_->subscribe("");
        // Connect to the publisher
        socket_->connect(endpoint);
    }
    virtual ~Subscriber() {
        // Unreachable, but for good measure
        if (!!socket_) {
            socket_->disconnect(endpoint_);
        }
    }
    void receive() override {
        zmqpp::message message;
        try {
            //std::cout << "receiving" << std::endl;
            bool received = socket_->receive(message, !blocking_);
            if (!blocking_ && !received) {
                //std::cout << "??" << std::endl;
                return;
            }
        } catch (zmqpp::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        boost::shared_ptr<Msg> data(new Msg());
        msgpack::unpacked unpacked_data;
        msgpack::unpack(unpacked_data,
                static_cast<const char*>(message.raw_data(0)),
                message.size(0));
        unpacked_data.get().convert(*data);
        callbackfn_(data);
    }
protected:
    boost::shared_ptr<zmqpp::socket> socket_;
    CallbackFn callbackfn_;
    const std::string endpoint_;
    const bool blocking_;
};
}
