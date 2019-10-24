//
// Created by Zikomo Fields on 2019-03-27.
//

#ifndef BERRYCAM_UDPBROADCASTER_H
#define BERRYCAM_UDPBROADCASTER_H




#include "Broadcaster.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace BerryCam {
class UdpBroadcaster : public Broadcaster {
    public:
        explicit UdpBroadcaster(boost::property_tree::ptree &settings,
                                boost::asio::io_service &io_service);

        void SendPacket(unsigned char *data, unsigned int size) override;

    private:
        boost::property_tree::ptree _settings;
        boost::asio::io_service::strand _strand;
        boost::asio::ip::udp::endpoint _endpoint;
        boost::asio::ip::udp::socket _socket;

        unsigned int _maxMtuSize;
    };

}


#endif //BERRYCAM_UDPBROADCASTER_H
