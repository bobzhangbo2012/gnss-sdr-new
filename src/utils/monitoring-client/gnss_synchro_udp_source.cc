#include "gnss_synchro_udp_source.h"
#include <boost/archive/binary_iarchive.hpp>
#include "gnss_synchro.pb.h"
#include <sstream>
#include <ncurses.h>

Gnss_Synchro_Udp_Source::Gnss_Synchro_Udp_Source(const unsigned short port) :
    socket{io_service},
    endpoint{boost::asio::ip::udp::v4(), port}
{
    socket.open(endpoint.protocol(), error);  // Open socket.
    socket.bind(endpoint, error);             // Bind the socket to the given local endpoint.
}

bool Gnss_Synchro_Udp_Source::read_gnss_synchro(gnss_sdr::Observables& stocks)
{
    char buff[1500];  // Buffer for storing the received data.

    // This call will block until one or more bytes of data has been received.
    int bytes = socket.receive(boost::asio::buffer(buff));

    std::string data(&buff[0], bytes);
    // Deserialize a stock of Gnss_Synchro objects from the binary string.
    return stocks.ParseFromString(data);
}

void Gnss_Synchro_Udp_Source::populate_channels(gnss_sdr::Observables& stocks)
{
    for (std::size_t i = 0; i < stocks.observable_size(); i++)
        {
            gnss_sdr::GnssSynchro ch = stocks.observable(i);
            if (ch.fs() != 0)  // Channel is valid.
                {
                    channels[ch.channel_id()] = ch;
                }
        }
}

bool Gnss_Synchro_Udp_Source::print_table()
{
    if (read_gnss_synchro(stocks))
        {
            populate_channels(stocks);

            clear();  // Clear the screen.

            // Print table header.
            attron(A_REVERSE);
            printw("%3s%6s%14s%17s\n", "CH", "PRN", "CN0 [dB-Hz]", "Doppler [Hz]");
            attroff(A_REVERSE);

            // Print table contents.
            for (auto const& ch : channels)
                {
                    int channel_id = ch.first;      // Key
                    gnss_sdr::GnssSynchro data = ch.second;  // Value

                    printw("%3d%6d%14f%17f\n", channel_id, data.prn(), data.cn0_db_hz(), data.carrier_doppler_hz());
                }
            refresh();  // Update the screen.
        }
    else
        {
            return false;
        }

    return true;
}

// #########################################
bool Gnss_Synchro_Udp_Source::read_monitor_pvt(Monitor_Pvt& monitor_pvt)
{
    char buff[1500];  // Buffer for storing the received data.

    // This call will block until one or more bytes of data has been received.
    int bytes = socket.receive(boost::asio::buffer(buff));

    try
        {
            // Deserialize Monitor_Pvt objects from the binary archive.
            std::string archive_data(&buff[0], bytes);
            std::istringstream archive_stream(archive_data);
            boost::archive::binary_iarchive archive(archive_stream);
            archive >> monitor_pvt;
        }
    catch (std::exception& e)
        {
            return false;
        }

    return true;
}

void Gnss_Synchro_Udp_Source::print_variable_names_column()
{
    clear();  // Clear the screen.

    int col = 0;
    int row = 0;
    char fspec[] = "%25s";

    // TOW
    mvprintw(row, col, fspec, "TOW_at_current_symbol_ms:");
    // WEEK
    mvprintw(row + 1, col, fspec, "week:");
    // PVT GPS time
    mvprintw(row + 2, col, fspec, "RX_time:");
    // User clock offset [s]
    mvprintw(row + 3, col, fspec, "user_clk_offset:");

    // ECEF POS X,Y,X [m] + ECEF VEL X,Y,X [m/s] (6 x double)
    mvprintw(row + 4, col, fspec, "pos_x:");
    mvprintw(row + 5, col, fspec, "pos_y:");
    mvprintw(row + 6, col, fspec, "pos_z:");
    mvprintw(row + 7, col, fspec, "vel_x:");
    mvprintw(row + 8, col, fspec, "vel_y:");
    mvprintw(row + 9, col, fspec, "vel_z:");

    // position variance/covariance (m^2) {c_xx,c_yy,c_zz,c_xy,c_yz,c_zx} (6 x double)
    mvprintw(row + 10, col, fspec, "cov_xx:");
    mvprintw(row + 11, col, fspec, "cov_yy:");
    mvprintw(row + 12, col, fspec, "cov_zz:");
    mvprintw(row + 13, col, fspec, "cov_xy:");
    mvprintw(row + 14, col, fspec, "cov_yz:");
    mvprintw(row + 15, col, fspec, "cov_zx:");

    // GEO user position Latitude [deg]
    mvprintw(row + 16, col, fspec, "latitude:");
    // GEO user position Longitude [deg]
    mvprintw(row + 17, col, fspec, "longitude:");
    // GEO user position Height [m]
    mvprintw(row + 18, col, fspec, "height:");

    // NUMBER OF VALID SATS
    mvprintw(row + 19, col, fspec, "valid_sats:");
    // RTKLIB solution status
    mvprintw(row + 20, col, fspec, "solution_status:");
    // RTKLIB solution type (0:xyz-ecef,1:enu-baseline)
    mvprintw(row + 21, col, fspec, "solution_type:");
    // AR ratio factor for validation
    mvprintw(row + 22, col, fspec, "AR_ratio_factor:");
    // AR ratio threshold for validation
    mvprintw(row + 23, col, fspec, "AR_ratio_threshold:");

    // GDOP / PDOP/ HDOP/ VDOP
    mvprintw(row + 24, col, fspec, "gdop:");
    mvprintw(row + 25, col, fspec, "pdop:");
    mvprintw(row + 26, col, fspec, "hdop:");
    mvprintw(row + 27, col, fspec, "vdop:");

    refresh();  // Update the screen.
}

void Gnss_Synchro_Udp_Source::print_values_column()
{
    if (read_monitor_pvt(monitor_pvt))
        {
            int col = 26;
            int row = 0;

            // TOW
            mvprintw(row, col, "%u", unsigned(monitor_pvt.TOW_at_current_symbol_ms));
            // WEEK
            mvprintw(row + 1, col, "%u", unsigned(monitor_pvt.week));
            // PVT GPS time
            mvprintw(row + 2, col, "%lf", monitor_pvt.RX_time);
            // User clock offset [s]
            mvprintw(row + 3, col, "%lf", monitor_pvt.user_clk_offset);

            // ECEF POS X,Y,X [m] + ECEF VEL X,Y,X [m/s] (6 x double)
            mvprintw(row + 4, col, "%lf\n", monitor_pvt.pos_x);
            mvprintw(row + 5, col, "%lf\n", monitor_pvt.pos_y);
            mvprintw(row + 6, col, "%lf\n", monitor_pvt.pos_z);
            mvprintw(row + 7, col, "%lf\n", monitor_pvt.vel_x);
            mvprintw(row + 8, col, "%lf\n", monitor_pvt.vel_y);
            mvprintw(row + 9, col, "%lf\n", monitor_pvt.vel_z);

            // position variance/covariance (m^2) {c_xx,c_yy,c_zz,c_xy,c_yz,c_zx} (6 x double)
            mvprintw(row + 10, col, "%lf\n", monitor_pvt.cov_xx);
            mvprintw(row + 11, col, "%lf\n", monitor_pvt.cov_yy);
            mvprintw(row + 12, col, "%lf\n", monitor_pvt.cov_zz);
            mvprintw(row + 13, col, "%lf\n", monitor_pvt.cov_xy);
            mvprintw(row + 14, col, "%lf\n", monitor_pvt.cov_yz);
            mvprintw(row + 15, col, "%lf\n", monitor_pvt.cov_zx);

            // GEO user position Latitude [deg]
            mvprintw(row + 16, col, "%lf\n", monitor_pvt.latitude);
            // GEO user position Longitude [deg]
            mvprintw(row + 17, col, "%lf\n", monitor_pvt.longitude);
            // GEO user position Height [m]
            mvprintw(row + 18, col, "%lf\n", monitor_pvt.height);

            // NUMBER OF VALID SATS
            mvprintw(row + 19, col, "%u\n", unsigned(monitor_pvt.valid_sats));
            // RTKLIB solution status
            mvprintw(row + 20, col, "%u\n", unsigned(monitor_pvt.solution_status));
            // RTKLIB solution type (0:xyz-ecef,1:enu-baseline)
            mvprintw(row + 21, col, "%u\n", unsigned(monitor_pvt.solution_type));
            // AR ratio factor for validation
            mvprintw(row + 22, col, "%f\n", monitor_pvt.AR_ratio_factor);
            // AR ratio threshold for validation
            mvprintw(row + 23, col, "%f\n", monitor_pvt.AR_ratio_threshold);

            // GDOP / PDOP/ HDOP/ VDOP
            mvprintw(row + 24, col, "%lf\n", monitor_pvt.gdop);
            mvprintw(row + 25, col, "%lf\n", monitor_pvt.pdop);
            mvprintw(row + 26, col, "%lf\n", monitor_pvt.hdop);
            mvprintw(row + 27, col, "%lf\n", monitor_pvt.vdop);

            refresh();  // Update the screen.
        }
}
