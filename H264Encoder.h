//
// Created by Zikomo Fields on 2019-09-28.
//

#ifndef BERRYCAM_H264ENCODER_H
#define BERRYCAM_H264ENCODER_H

#include "Encoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

namespace BerryCam {
    class H264Encoder : public Encoder {
    public:
        H264Encoder();
        ~H264Encoder() override;
        void setEncoderParameters(std::shared_ptr<boost::property_tree::ptree> encoderParameters) override;
        std::shared_ptr<boost::property_tree::ptree> getEncoderParameters() override;
        void encode(const void *pVoid) override;
    private:
        const char  *codec_name;
        const AVCodec *codec;
        AVFrame *frame;
        AVCodecContext *codec_context= nullptr;
        AVPacket *pkt;

    };
}


#endif //BERRYCAM_H264ENCODER_H
