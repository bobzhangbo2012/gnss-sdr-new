#ifndef GNSS_SYNCHRO_UDP_SOURCE_H_
#define GNSS_SYNCHRO_UDP_SOURCE_H_

#include <boost/asio.hpp>
#include "monitor_pvt.h"
#include "monitor_pvt.pb.h"
#include "gnss_synchro.pb.h"  // This file is created automatically
                              // by the Protocol Buffers compiler

class Gnss_Synchro_Udp_Source
{
public:
    Gnss_Synchro_Udp_Source(const unsigned short port);
    bool read_gnss_synchro(gnss_sdr::Observables& stocks);
    void populate_channels(gnss_sdr::Observables& stocks);
    bool print_table();
    
    // Gnss_Synchro_Udp_Source(const unsigned short& port);
    bool read_monitor_pvt(Monitor_Pvt& monitor_pvt);
    void print_variable_names_column();
    void print_values_column();

private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket;
    boost::system::error_code error;
    boost::asio::ip::udp::endpoint endpoint;
    gnss_sdr::Observables stocks;
    std::map<int, gnss_sdr::GnssSynchro> channels;

    Monitor_Pvt monitor_pvt;
};

#endif /* GNSS_SYNCHRO_UDP_SOURCE_H_ */
