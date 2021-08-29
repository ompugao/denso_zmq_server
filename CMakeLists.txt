project(sandbox)
cmake_minimum_required(VERSION 3.16)


if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(denso_control)

if(denso_control_FOUND)
    include_directories(${denso_control_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

    list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(ZeroMQ REQUIRED)
    find_package(Boost REQUIRED COMPONENTS system)
    find_package(MsgPack REQUIRED)

    add_library(middleware SHARED middleware/context.cpp)
    target_link_libraries(middleware ${Boost_LIBRARIES} ${ZeroMQ_LIBRARY} zmqpp)
    include_directories(${ZeroMQ_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    add_executable(densobcapserver main.cpp)
    target_link_libraries(densobcapserver ${denso_control_LIBRARIES} middleware)
    add_definitions(-DDENSO_CONTROL_FOUND)

    add_executable(test_middleware_sub test_middleware_sub.cpp)
    target_link_libraries(test_middleware_sub middleware)
    add_executable(test_middleware_pub test_middleware_pub.cpp)
    target_link_libraries(test_middleware_pub middleware)

    #add_executable(disconnect disconnect.cpp)
    #target_link_libraries(disconnect ${denso_control_LIBRARIES})
    #add_definitions(-DDENSO_CONTROL_FOUND)
endif()

