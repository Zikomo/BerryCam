#include <utility>
#include <iostream>

//
// Created by Zikomo Fields on 2019-03-27.
//

#include "UdpBroadcaster.h"
#include "Utilities.h"
#include "BerryCamHeader.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::property_tree;

BerryCam::UdpBroadcaster::UdpBroadcaster(ptree &settings, boost::asio::io_service &io_service)
    : _strand(io_service),
      _settings(settings),
      _endpoint(address_v4::from_string(Utilities::SafeGet(_settings, "socket.broadcast.address", std::string("239.255.0.1"))),
              Utilities::SafeGet(_settings, "socket.broadcast.port", (unsigned short)9999)),
      _socket(_strand.get_io_service(), _endpoint.protocol()),
      _maxMtuSize(Utilities::SafeGet(_settings, "socket.broadcast.max_mtu_size", 1450u))
{
    cout<<"Broadcasting on: "<<_endpoint.address().to_string()<< " port: " << _endpoint.port()<<endl;
}



void BerryCam::UdpBroadcaster::SendPacket(uint8_t *data, unsigned int size) {
    try {
        BerryCamHeader berryCamHeader {BERRYCAM_PREAMBLE, size};
        _socket.send_to(boost::asio::buffer(&berryCamHeader, static_cast<size_t>(sizeof(BerryCamHeader))), _endpoint);
        if (size <= _maxMtuSize) {
            _socket.send_to(boost::asio::buffer(data , static_cast<size_t>(size)), _endpoint);
            return;
        }
        unsigned int lastPosition = 0;
        for (unsigned int position = 0; position < size; position += _maxMtuSize) {
            _socket.send_to(boost::asio::buffer(data + position, static_cast<size_t>(_maxMtuSize)), _endpoint);
            lastPosition = position;
        }
        if (lastPosition < size) {
            _socket.send_to(boost::asio::buffer(data + lastPosition, static_cast<size_t>(size - lastPosition)), _endpoint);
        }

    }
    catch (std::runtime_error& error) {
        cerr<<error.what()<<endl;
    }
}


