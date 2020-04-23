/*!
 * \file tcp_communication.cc
 * \brief Implementation of the TCP communication class
 * \author David Pubill, 2011. dpubill(at)cttc.es
 *
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * -------------------------------------------------------------------------
 */

#include "tcp_communication.h"
#include "tcp_packet_data.h"
#include <iostream>
#include <stdexcept>
#include <string>


Tcp_Communication::Tcp_Communication() : tcp_socket_(io_context_) {}  // NOLINT


int Tcp_Communication::listen_tcp_connection(size_t d_port_, size_t d_port_ch0_)
{
    try
        {
            // Specify IP type and port
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), d_port_);
            boost::asio::ip::tcp::acceptor acceptor(io_context_, endpoint);

            if (d_port_ == d_port_ch0_)
                {
                    std::cout << "Server ready. Listening for TCP connections..." << std::endl;
                }

            // Reuse the IP address for each connection
            acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

            // Listen for a connection and accept it
            acceptor.listen(12);
            acceptor.accept(tcp_socket_);

            std::cout << "Socket accepted on port " << d_port_ << std::endl;
        }

    catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }

    return false;
}


void Tcp_Communication::send_receive_tcp_packet_galileo_e1(boost::array<float, NUM_TX_VARIABLES_GALILEO_E1> buf, Tcp_Packet_Data* tcp_data_)
{
    int controlc = 0;
    boost::array<float, NUM_RX_VARIABLES> readbuf{};
    float d_control_id_ = buf.data()[0];

    try
        {
            // Send a TCP packet
            tcp_socket_.write_some(boost::asio::buffer(buf));

            // Read the received TCP packet
            tcp_socket_.read_some(boost::asio::buffer(readbuf));

            //! Control. The GNSS-SDR program ends if an error in a TCP packet is detected.
            if (d_control_id_ != readbuf.data()[0])
                {
                    throw std::runtime_error("Packet error!");
                }

            // Recover the variables received
            tcp_data_->proc_pack_code_error = readbuf.data()[1];
            tcp_data_->proc_pack_carr_error = readbuf.data()[2];
            tcp_data_->proc_pack_carrier_doppler_hz = readbuf.data()[3];
        }

    catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << ". Please press Ctrl+C to end the program." << std::endl;
            std::cin >> controlc;
        }
}


void Tcp_Communication::send_receive_tcp_packet_gps_l1_ca(boost::array<float, NUM_TX_VARIABLES_GPS_L1_CA> buf, Tcp_Packet_Data* tcp_data_)
{
    int controlc = 0;
    boost::array<float, NUM_RX_VARIABLES> readbuf{};
    float d_control_id_ = buf.data()[0];

    try
        {
            // Send a TCP packet
            tcp_socket_.write_some(boost::asio::buffer(buf));

            // Read the received TCP packet
            tcp_socket_.read_some(boost::asio::buffer(readbuf));

            //! Control. The GNSS-SDR program ends if an error in a TCP packet is detected.
            if (d_control_id_ != readbuf.data()[0])
                {
                    throw std::runtime_error("Packet error!");
                }

            // Recover the variables received
            tcp_data_->proc_pack_code_error = readbuf.data()[1];
            tcp_data_->proc_pack_carr_error = readbuf.data()[2];
            tcp_data_->proc_pack_carrier_doppler_hz = readbuf.data()[3];
        }

    catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << ". Please press Ctrl+C to end the program." << std::endl;
            std::cin >> controlc;
        }
}


void Tcp_Communication::close_tcp_connection(size_t d_port_)
{
    // Close the TCP connection
    tcp_socket_.close();
    std::cout << "Socket closed on port " << d_port_ << std::endl;
}
