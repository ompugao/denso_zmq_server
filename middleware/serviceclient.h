#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include <zmqpp/zmqpp.hpp>
#include <msgpack.hpp>
#include <stdexcept>

namespace middleware {
class ServiceException : public std::runtime_error {
public:
    ServiceException(const std::string& s) : std::runtime_error(s) {
    }
    virtual ~ServiceException() {
    }
};

class ServiceClient {
public:
    ServiceClient() {
    }
    ServiceClient(zmqpp::context& context, const std::string& endpoint): endpoint_(endpoint) {
        zmqpp::socket_type type = zmqpp::socket_type::request;
        socket_ = boost::make_shared<zmqpp::socket>(context, type);
        socket_->connect(endpoint);
    }

    virtual ~ServiceClient() {
        if (!!socket_) {
            socket_->disconnect(endpoint_);
        }
    }

    template<typename Request, typename Response>
    bool call(const Request& req, Response& res) {
        if (!socket_) {
            std::cerr << "serviceclient not initialized" << std::endl;
            return false;
        }
        zmqpp::message message, resmessage;
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, req);
        message.add_raw(buffer.data(), buffer.size());

        socket_->send(message);

        bool received = socket_->receive(resmessage, /*nonblocking*/ false);
        if (!received) {
            throw ServiceException("failed to receive response");
        }

        msgpack::unpacked unpacked_data;
        msgpack::unpack(unpacked_data,
                static_cast<const char*>(resmessage.raw_data(0)),
                resmessage.size(0));
        unpacked_data.get().convert(res);
        bool ret = *static_cast<const bool*>(resmessage.raw_data(1));
        return ret;
    }

protected:
    boost::shared_ptr<zmqpp::socket> socket_;
    const std::string endpoint_;
};
} // namespace middleware

