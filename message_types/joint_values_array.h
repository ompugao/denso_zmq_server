#pragma once
#include <msgpack.hpp>

namespace message_types {
class JointValuesArray {
public:
    JointValuesArray() {
    }
    virtual ~JointValuesArray() {
    }
    std::vector<std::vector<double>> values;
public:
    MSGPACK_DEFINE(values);
};
} // namespace message_types
