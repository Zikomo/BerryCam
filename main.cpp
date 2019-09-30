#include "Broadcaster.h"
#include "UdpBroadcaster.h"
#include "Encoder.h"
#include "H264Encoder.h"
#include "Camera.h"
#include "RaspberryPiCamera.h"


#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>


using namespace BerryCam;
using namespace boost::property_tree;
using namespace boost::program_options;

std::string parseCommandLine(int argc, char *const *argv);

ptree getJsonSettings(std::string &jsonFileName);

void waitUntilQuit();

void saveJsonSettings(std::string &jsonFileName, const ptree &settings);

int main(int argc, char **argv) {
    //First parse the command line options
    std::string jsonFileName = parseCommandLine(argc, argv);

    //Loading the JSON settings
    ptree settings = getJsonSettings(jsonFileName);

    //Setting up broadcaster, encoder, and camera
    boost::asio::io_service io_service;
    std::shared_ptr<Broadcaster> udpBroadcaster = std::make_shared<UdpBroadcaster>(settings, io_service);
    std::shared_ptr<Encoder> h264Encoder = std::make_shared<H264Encoder>(udpBroadcaster);
    h264Encoder->setEncoderParameters(settings);
    std::shared_ptr<Camera> raspberryPiCamera = std::make_shared<RaspberryPiCamera>(h264Encoder);
    raspberryPiCamera->setCameraParameters(settings);

    //Start camera
    raspberryPiCamera->start();

    //Waiting for operator to enter quit
    waitUntilQuit();


    //Stop camera and begin shutdown.
    raspberryPiCamera->stop();
    
    //Write out updated settings
    saveJsonSettings(jsonFileName, settings);

    return 0;
}

void saveJsonSettings(std::string &jsonFileName, const ptree &settings) {
    try {
        write_json(jsonFileName, settings);
    }
    catch (json_parser_error& error) {
        std::cerr << error.what() << std::endl;
    }
}

void waitUntilQuit() {
    std::string quit;
    std::cout << "To quit type quit and press enter." << std::endl;
    do {
        std::cin >> quit;
        transform(quit.begin(), quit.end(), quit.begin(),
                       [](unsigned char c){ return tolower(c); });
    } while (quit != "quit");
}

ptree getJsonSettings(std::string &jsonFileName) {
    ptree settings = ptree();
    try {
        read_json(jsonFileName, settings);
    }
    catch (json_parser_error& error) {
        std::cerr << error.what() << std::endl;
    }
    return settings;
}

std::string parseCommandLine(int argc, char *const *argv) {
    options_description optionsDescription("BerryCam options");
    optionsDescription.add_options()
            ("help,h", "This Message")
            ("settings_file,s", value<std::string>(),
                    "Path to the JSON settings, if it does not exist one will be created with default "
                    "values at the location specified. ");
    variables_map commandLineOptions;
    bool commandLineParsingError = false;
    try {
        store(parse_command_line(argc, argv, optionsDescription), commandLineOptions);
        notify(commandLineOptions);
    }
    catch (error& commandLineError) {
        commandLineParsingError = true;
    }
    //If the user wants help or had or set invalid commandline options display help message and exit.
    if (commandLineOptions.count("help") || commandLineParsingError) {
        std::cout << optionsDescription << std::endl;
        exit(0);
    }

    std::string jsonFileName = "settings.json";
    if (commandLineOptions.count("settings_file")) {
        jsonFileName = commandLineOptions["settings_file"].as<std::string>();
        std::cout << jsonFileName << std::endl;
    }

    return jsonFileName;
}



