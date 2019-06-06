/*!
 * \file monitor_pvt_udp_source.h
 * \brief Interface of a class that recovers serialized Monitor_Pvt
 * objects from a udp socket
 * \author Álvaro Cebrián Juan, 2019. acebrianjuan(at)gmail.com
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <https://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#ifndef MONITOR_PVT_UDP_SOURCE_H_
#define MONITOR_PVT_UDP_SOURCE_H_

#include "monitor_pvt.h"
#include <boost/asio.hpp>

class Monitor_Pvt_Udp_Source
{
public:
    Monitor_Pvt_Udp_Source(const unsigned short& port);
    bool read_monitor_pvt(Monitor_Pvt& monitor_pvt);
    void print_variable_names_column();
    void print_values_column();

private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket;
    boost::system::error_code error;
    boost::asio::ip::udp::endpoint endpoint;
    Monitor_Pvt monitor_pvt;
};

#endif /* MONITOR_PVT_UDP_SOURCE_H_ */
