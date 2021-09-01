#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include <zmqpp/zmqpp.hpp>
#include <msgpack.hpp>

namespace middleware {
class ServiceServerBase {
public:
    ServiceServerBase() {
    }
    virtual ~ServiceServerBase() {
    }
    virtual void receive() = 0;
};

template<typename Request, typename Response> 
class ServiceServer : public ServiceServerBase {
public:
    using CallbackFn = boost::function<bool(const boost::shared_ptr<Request>&, boost::shared_ptr<Response>&)>;
    ServiceServer(zmqpp::context& zmqcontext, const std::string& endpoint,
            CallbackFn callback) :
        ServiceServerBase(), callbackfn_(callback), endpoint_(endpoint) {
        zmqpp::socket_type type = zmqpp::socket_type::rep;
        socket_ = boost::make_shared<zmqpp::socket>(zmqcontext, type);
        socket_->bind(endpoint);
    }
    virtual ~ServiceServer() {
        // Unreachable, but for good measure
        if (!!socket_) {
            socket_->disconnect(endpoint_);
        }
    }
    void receive() override {
        zmqpp::message message;
        try {
            bool received = socket_->receive(message, /*nonblocking*/ true);
            if (!received) {
                return;
            }
        } catch (zmqpp::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        boost::shared_ptr<Request> req(new Request());
        msgpack::unpacked unpacked_data;
        msgpack::unpack(unpacked_data,
                static_cast<const char*>(message.raw_data(0)),
                message.size(0));
        unpacked_data.get().convert(*req);

        boost::shared_ptr<Response> res(new Response());
        bool ok = callbackfn_(req, res);

        zmqpp::message resmessage;
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *res);
        resmessage.add_raw(buffer.data(), buffer.size());
        //resmessage.add_raw(reinterpret_cast<void const*>(&ok), sizeof(bool));
        resmessage << ok;

        socket_->send(resmessage);
    }
protected:
    boost::shared_ptr<zmqpp::socket> socket_;
    CallbackFn callbackfn_;
    const std::string endpoint_;
};
}

