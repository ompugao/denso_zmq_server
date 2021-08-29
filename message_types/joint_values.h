#pragma once
#include <msgpack.hpp>

namespace message_types {
class JointValues {
public:
    JointValues() {
    }
    virtual ~JointValues() {
    }
    std::vector<double> values;
public:
    MSGPACK_DEFINE(values);
};
} // namespace message_types
