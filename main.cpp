#include "Broadcaster.h"
#include "UdpBroadcaster.h"
#include "Encoder.h"
#include "H264Encoder.h"
#include "Camera.h"
#include "RaspberryPiCamera.h"


#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>



using namespace boost::property_tree;
using namespace BerryCam;

int main(int argc, char **argv) {
    ptree settings = ptree();
    try {
        json_parser::read_json("settings.json", settings);
    }
    catch (json_parser_error& error) {

    }

    boost::asio::io_service io_service;

    std::shared_ptr<Broadcaster> udpBroadcaster = std::make_shared<UdpBroadcaster>(settings, io_service);
    std::shared_ptr<Encoder> h264Encoder = std::make_shared<H264Encoder>(udpBroadcaster);
    h264Encoder->setEncoderParameters(settings);
    std::shared_ptr<Camera> raspberryPiCamera = std::make_shared<RaspberryPiCamera>(h264Encoder);

    raspberryPiCamera->setCameraParameters(settings);

    raspberryPiCamera->start();

    char c;
    std::cin>>c;

    raspberryPiCamera->stop();


    try {
        json_parser::write_json("settings.json", settings);
    }
    catch (json_parser_error& error) {

    }

    return 0;
}