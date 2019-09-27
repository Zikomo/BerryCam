//
// Created by Zikomo Fields on 2019-09-26.
//

#ifndef BERRYCAM_CAMERA_H
#define BERRYCAM_CAMERA_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::property_tree;

class Camera {
public:
    virtual ~Camera() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setCameraParameters(ptree &) = 0;
    virtual ptree getCameraParameters() = 0;
    virtual void onFrameReceived(unsigned char* frameData) = 0;
};


#endif //BERRYCAM_CAMERA_H
