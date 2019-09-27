//
// Created by Zikomo Fields on 2019-03-27.
//

#ifndef BERRYCAM_BROADCASTER_H
#define BERRYCAM_BROADCASTER_H

#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

class UdpBroadcaster {
public:
    explicit UdpBroadcaster(std::shared_ptr<ptree> settings, boost::asio::io_service &io_service);
    void SendPacket(uint8_t *data, int size);

private:
    void OnSent(const boost::system::error_code& error);
    std::shared_ptr<ptree> _settings;
    boost::asio::io_service::strand _strand;
    boost::asio::ip::udp::endpoint _endpoint;
    boost::asio::ip::udp::socket _socket;
};


#endif //BERRYCAM_BROADCASTER_H
