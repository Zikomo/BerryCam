//
// Created by Zikomo Fields on 2019-09-28.
//

#ifndef BERRYCAM_H264ENCODER_H
#define BERRYCAM_H264ENCODER_H

#include "Encoder.h"
#include "Broadcaster.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

namespace BerryCam {
    class H264Encoder : public Encoder {
    public:
        explicit H264Encoder(std::shared_ptr<Broadcaster> broadcaster);
        ~H264Encoder();
        void setEncoderParameters(boost::property_tree::ptree &encoderParameters) override;
        boost::property_tree::ptree getEncoderParameters() override;
        void encode(const void *pVoid) override;
    private:
        void copyBufferToFrame(const void *buffer) const;
        long _frameCount;
        const AVCodec *_codec;
        AVFrame *_frame;
        AVCodecContext *_codecContext;
        AVPacket *_tempPacket;
        std::vector<unsigned char> _ppsFrameStore;
        std::shared_ptr<Broadcaster> _broadcaster;


    };
}


#endif //BERRYCAM_H264ENCODER_H
