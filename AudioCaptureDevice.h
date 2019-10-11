//
// Created by Zikomo Fields on 10/10/19.
//

#ifndef BERRYCAM_AUDIOCAPTUREDEVICE_H
#define BERRYCAM_AUDIOCAPTUREDEVICE_H

#include <boost/property_tree/ptree.hpp>

namespace BerryCam {
    class AudioCaptureDevice {
    public:
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void setAudioCaptureParameters(boost::property_tree::ptree &) = 0;
        virtual boost::property_tree::ptree getAudioCaptureParameters() = 0;
    };
}


#endif //BERRYCAM_AUDIOCAPTUREDEVICE_H
