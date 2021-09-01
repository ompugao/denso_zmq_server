#pragma once
#include <msgpack.hpp>
#include <chrono>

namespace message_types {
class JointValuesArray {
public:
    JointValuesArray() {
    }
    virtual ~JointValuesArray() {
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
    std::vector<std::vector<double>> values;
public:
    MSGPACK_DEFINE(timepoint, values);
};
} // namespace message_types
