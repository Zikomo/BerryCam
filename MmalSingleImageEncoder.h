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

        void encode(const void *pVoid) override;

    private:
        MMAL_WRAPPER_T* encoder;
        MMAL_FOURCC_T encoding;
        int _frameCount;
        MMAL_PORT_T* portIn;
        MMAL_PORT_T* portOut;
        MMAL_BUFFER_HEADER_T* in;
        MMAL_BUFFER_HEADER_T* out;
        MMAL_STATUS_T status;
        std::string filename;
        unsigned int width;
        unsigned int height;

    };
}


#endif //BERRYCAM_MMALSINGLEIMAGEENCODER_H
