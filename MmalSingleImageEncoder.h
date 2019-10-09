//
// Created by Zikomo Fields on 10/2/19.
//

#ifndef BERRYCAM_MMALSINGLEIMAGEENCODER_H
#define BERRYCAM_MMALSINGLEIMAGEENCODER_H



#include <interface/mmal/mmal.h>
#include <interface/mmal/mmal_format.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_component_wrapper.h>
#include <interface/mmal/util/mmal_util_params.h>

#include "Encoder.h"

namespace BerryCam {
    class MmalSingleImageEncoder : public Encoder {
    public:
        MmalSingleImageEncoder();
        ~MmalSingleImageEncoder();
        void setEncoderParameters(boost::property_tree::ptree &encoderParameters) override;

        boost::property_tree::ptree getEncoderParameters() override;

        void encode(const void *image) override;

    private:
        MMAL_WRAPPER_T* _encoder;
        MMAL_FOURCC_T _encoding;
        int _frameCount;
        MMAL_PORT_T* _portIn;
        MMAL_PORT_T* _portOut;
        MMAL_BUFFER_HEADER_T* _input;
        MMAL_BUFFER_HEADER_T* _output;
        MMAL_STATUS_T _status;
        std::string _filename;
        unsigned int _width;
        unsigned int _height;
        int _targetFramesPerSecond;

    };
}


#endif //BERRYCAM_MMALSINGLEIMAGEENCODER_H
