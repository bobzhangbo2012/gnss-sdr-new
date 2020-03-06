# Copyright (C) 2012-2020  (see AUTHORS file for a list of contributors)
#
# GNSS-SDR is a software-defined Global Navigation Satellite Systems receiver
#
# This file is part of GNSS-SDR.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


# Optional drivers
set(OPT_DRIVER_SOURCES "")
set(OPT_DRIVER_HEADERS "")

# if(ENABLE_RAW_UDP AND PCAP_FOUND)
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} custom_udp_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} custom_udp_signal_source.h)
# endif()


# if(ENABLE_PLUTOSDR)
#     ##############################################
#     # ADALM-PLUTO (Analog Devices Inc.)
#     ##############################################
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} plutosdr_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} plutosdr_signal_source.h)
# endif()


# if(ENABLE_FMCOMMS2)
#     ###############################################
#     # FMCOMMS2 based SDR Hardware
#     ###############################################
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} fmcomms2_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} fmcomms2_signal_source.h)
# endif()


# if(ENABLE_AD9361)
#     ###############################################
#     # AD9361 DIRECT TO FPGA Hardware
#     ###############################################
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} ad9361_fpga_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} ad9361_fpga_signal_source.h)
# endif()


# if(ENABLE_GN3S)
#     ##############################################
#     # GN3S (USB dongle)
#     ##############################################
#     if(NOT GRGN3S_FOUND)
#         message(" gr-gn3s not found, install it from https://github.com/gnss-sdr/gr-gn3s")
#         message(FATAL_ERROR "gr-gn3s required for building gnss-sdr with this option enabled")
#     endif()
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} gn3s_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} gn3s_signal_source.h)
# endif()


# if(ENABLE_FLEXIBAND AND TELEORBIT_FOUND)
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} flexiband_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} flexiband_signal_source.h)
# endif()


# if(ENABLE_ARRAY)
#     ##############################################
#     # DBFCTTC GNSS EXPERIMENTAL ARRAY PROTOTYPE
#     ##############################################
#     if(NOT GRDBFCTTC_FOUND)
#         message(" gr-dbfcttc not found, install it from https://github.com/gnss-sdr/gr-dbfcttc")
#         message(FATAL_ERROR "gr-dbfcttc required for building gnss-sdr with this option enabled")
#     endif()
#     set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} raw_array_signal_source.cc)
#     set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} raw_array_signal_source.h)
# endif()


# if(ENABLE_OSMOSDR)
#     ################################################################################
#     # OsmoSDR - https://osmocom.org/projects/gr-osmosdr/
#     ################################################################################
#     if(GROSMOSDR_FOUND)
#         set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} osmosdr_signal_source.cc)
#         set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} osmosdr_signal_source.h)
#     endif()
# endif()


if(ENABLE_UHD)
    set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} uhd_signal_sink.cc)
    set(OPT_DRIVER_HEADERS ${OPT_DRIVER_HEADERS} uhd_signal_sink.h)
endif()


set(SIGNAL_SINK_ADAPTER_SOURCES
    # file_signal_source.cc
    # gen_signal_source.cc
    # nsr_file_signal_source.cc
    # spir_file_signal_source.cc
    # spir_gss6450_file_signal_source.cc
    # rtl_tcp_signal_source.cc
    # labsat_signal_source.cc
    # two_bit_cpx_file_signal_source.cc
    # two_bit_packed_file_signal_source.cc
   
    ${OPT_DRIVER_SOURCES}
)

set(SIGNAL_SINK_ADAPTER_HEADERS
    # file_signal_source.h
    # gen_signal_source.h
    # nsr_file_signal_source.h
    # spir_file_signal_source.h
    # spir_gss6450_file_signal_source.h
    # rtl_tcp_signal_source.h
    # labsat_signal_source.h
    # two_bit_cpx_file_signal_source.h
    # two_bit_packed_file_signal_source.h
   
    ${OPT_DRIVER_HEADERS}
)

list(SORT SIGNAL_SINK_ADAPTER_HEADERS)
list(SORT SIGNAL_SINK_ADAPTER_SOURCES)

source_group(Headers FILES ${SIGNAL_SINK_ADAPTER_HEADERS})

add_library(signal_sink_adapters
    ${SIGNAL_SINK_ADAPTER_SOURCES}
    ${SIGNAL_SINK_ADAPTER_HEADERS}
)

target_include_directories(signal_sink_adapters
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/core/interfaces
        ${CMAKE_SOURCE_DIR}/src/core/receiver
        ${CMAKE_SOURCE_DIR}/src/algorithms/libs
)

target_link_libraries(signal_sink_adapters
    PUBLIC
        Boost::headers
        Gnuradio::runtime
        Gnuradio::blocks
        # signal_sink_gr_blocks
        signal_sink_libs
    PRIVATE
        gnss_sdr_flags
        core_system_parameters
        Gflags::gflags
        Glog::glog
        Volk::volk
)

if(ENABLE_RAW_UDP AND PCAP_FOUND)
    target_link_libraries(signal_sink_adapters
        PRIVATE
            Pcap::pcap
    )
endif()

if(ENABLE_UHD)
    target_link_libraries(signal_sink_adapters
        PUBLIC
            Gnuradio::uhd
        PRIVATE
            Uhd::uhd
    )
endif()

# if(ENABLE_OSMOSDR AND GROSMOSDR_FOUND)
#     target_link_libraries(signal_source_adapters
#         PUBLIC
#             Gnuradio::osmosdr
#     )
# endif()

# if(ENABLE_AD9361 AND LIBIIO_FOUND)
#     target_link_libraries(signal_source_adapters
#         PRIVATE
#             Iio::iio
#     )
# endif()

# if(ENABLE_FLEXIBAND AND TELEORBIT_FOUND)
#     target_link_libraries(signal_source_adapters
#         PRIVATE
#             Gnuradio::teleorbit
#     )
# endif()

# if(ENABLE_GN3S AND GRGN3S_FOUND)
#     target_link_libraries(signal_source_adapters
#         PRIVATE
#             Gnuradio::gn3s
#     )
# endif()

# if(ENABLE_ARRAY AND GRDBFCTTC_FOUND)
#     target_link_libraries(signal_source_adapters
#         PRIVATE
#             Gnuradio::dbfcttc
#     )
# endif()

target_compile_definitions(signal_sink_adapters
    PRIVATE -DGNSSSDR_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}"
)

if(ENABLE_CLANG_TIDY)
    if(CLANG_TIDY_EXE)
        set_target_properties(signal_sink_adapters
            PROPERTIES
                CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
        )
    endif()
endif()

set_property(TARGET signal_sink_adapters APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
