#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include <zmqpp/zmqpp.hpp>
#include <msgpack.hpp>

namespace middleware {
class Publisher {
public:
    Publisher() {
    }
    Publisher(zmqpp::context& context, const std::string& endpoint): endpoint_(endpoint) {
        zmqpp::socket_type type = zmqpp::socket_type::publish;
        socket_ = boost::make_shared<zmqpp::socket>(context, type);
        socket_->bind(endpoint);
    }

    virtual ~Publisher() {
        if (!!socket_) {
            socket_->disconnect(endpoint_);
        }
    }

    template<typename Msg>
    void publish(const Msg& msg) {
        if (!socket_) {
            std::cerr << "publisher not initialized" << std::endl;
            return;
        }
        zmqpp::message message;
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, msg);
        message.add_raw(buffer.data(), buffer.size());

        socket_->send(message);
    }
protected:
    boost::shared_ptr<zmqpp::socket> socket_;
    const std::string endpoint_;
};
} // namespace middleware
