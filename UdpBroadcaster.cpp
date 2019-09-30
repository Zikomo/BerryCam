#include <utility>
#include <iostream>

//
// Created by Zikomo Fields on 2019-03-27.
//

#include "UdpBroadcaster.h"
#include "Utilities.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::property_tree;

BerryCam::UdpBroadcaster::UdpBroadcaster(ptree &settings, boost::asio::io_service &io_service)
    : _strand(io_service),
      _settings(settings),
      _endpoint(address_v4::from_string(Utilities::SafeGet(_settings, "socket.broadcast.address", std::string("239.255.0.1"))),
              Utilities::SafeGet(_settings, "socket.broadcast.port", (unsigned short)9999)),
      _socket(_strand.get_io_service(), _endpoint.protocol())
{
    cout<<"Broadcasting on: "<<_endpoint.address().to_string()<< " port: " << _endpoint.port()<<endl;
}



void BerryCam::UdpBroadcaster::SendPacket(uint8_t *data, int size) {
    _socket.send_to(boost::asio::buffer(data, static_cast<size_t>(size)), _endpoint);
}


