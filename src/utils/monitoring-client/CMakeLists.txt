# cmake_minimum_required (VERSION 3.9)
project (monitoring-client CXX)

set(CMAKE_CXX_STANDARD 11)

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost COMPONENTS system serialization REQUIRED)
if(NOT Boost_FOUND)
     message(FATAL_ERROR "Fatal error: Boost required.")
endif(NOT Boost_FOUND)


set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)


# find_package(Protobuf REQUIRED)
# if(${Protobuf_VERSION} VERSION_LESS "3.0.0")
#      message(FATAL_ERROR "Fatal error: Protocol Buffers >= v3.0.0 required.")
# endif()


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/gnss_synchro.proto ${CMAKE_CURRENT_SOURCE_DIR}/monitor_pvt.proto)

# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/monitor_pvt.proto)


set(MONITORING_CLIENT_SOURCES gnss_synchro_udp_source.cc)
set(MONITORING_CLIENT_HEADERS gnss_synchro_udp_source.h)
set(MONITOR_PVT_UDP_SOURCES monitor_pvt_udp_source.cc)
set(MONITOR_PVT_UDP_HEADERS monitor_pvt_udp_source.h)
set(MONITOR_PVT_HEADERS monitor_pvt.h)

add_library(monitoring_lib ${MONITORING_CLIENT_SOURCES} ${MONITORING_CLIENT_HEADERS} ${PROTO_SRCS})
source_group(Headers FILES ${MONITORING_CLIENT_HEADERS} ${PROTO_HDRS})
# add_library(monitoring_lib ${CMAKE_SOURCE_DIR}/src/utils/monitoring-client/gnss_synchro_udp_source.cc ${PROTO_SRCS})

target_link_libraries(monitoring_lib
    PUBLIC
        ${Boost_LIBRARIES}
        # Boost::boost
        # Boost::system
        ${CURSES_LIBRARIES}
        protobuf::libprotobuf
        pthread
)

target_include_directories(monitoring_lib
    PUBLIC
        ${CURSES_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        ${Boost_INCLUDE_DIRS}
)

add_executable(monitoring-client ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)

target_link_libraries(monitoring-client PUBLIC monitoring_lib)