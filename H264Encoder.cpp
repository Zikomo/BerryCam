//
// Created by Zikomo Fields on 2019-09-28.
//

#include <iostream>
#include "H264Encoder.h"
#include "Utilities.h"

using namespace boost::property_tree;

BerryCam::H264Encoder::H264Encoder(std::shared_ptr<Broadcaster> broadcaster) :
        _frameCount(0),
        _codec(nullptr),
        _frame(nullptr),
        _codecContext(nullptr),
        _tempPacket(nullptr),
        _broadcaster(std::move(broadcaster)) {
    avcodec_register_all();
    std::stringstream errorString;
    _codec = avcodec_find_encoder_by_name("h264_omx");//avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!_codec) {
        errorString<<"Codec "<<AV_CODEC_ID_H264<<" not found. "<<std::endl;
        throw std::runtime_error(errorString.str());
    }

    _codecContext = avcodec_alloc_context3(_codec);
    if (!_codecContext) {
        errorString<< "Could not allocate video codec context"<<std::endl;
        throw std::runtime_error(errorString.str());
    }

    _tempPacket = av_packet_alloc();
    if (!_tempPacket) {
        errorString<<"av_packet_alloc failed exiting"<<std::endl;
        throw std::runtime_error(errorString.str());
    }
}

BerryCam::H264Encoder::~H264Encoder() {
    avcodec_free_context(&_codecContext);
    av_frame_free(&_frame);
    av_packet_free(&_tempPacket);
}

void BerryCam::H264Encoder::setEncoderParameters(ptree &encoderParameters) {
    /* put sample parameters */
    _codecContext->bit_rate = Utilities::SafeGet(encoderParameters, ENCODER_BIT_RATE, 4000000);
    /* resolution must be a multiple of two */
    _codecContext->width = Utilities::SafeGet(encoderParameters, CAMERA_PREVIEW_WIDTH, 1024u);
    _codecContext->height = Utilities::SafeGet(encoderParameters, CAMERA_PREVIEW_HEIGHT, 768u);;
    /* frames per second */
    _codecContext->time_base = (AVRational){1, Utilities::SafeGet(encoderParameters, ENCODER_FRAME_RATE, 25)};
    _codecContext->framerate = (AVRational){Utilities::SafeGet(encoderParameters, ENCODER_FRAME_RATE, 25), 1};
    _codecContext->level = 32;
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    _codecContext->gop_size = Utilities::SafeGet(encoderParameters, ENCODER_GOP_SIZE, 10);
    _codecContext->max_b_frames =  Utilities::SafeGet(encoderParameters, MAX_B_FRAMES, 1);;
    _codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    av_opt_set(_codecContext->priv_data, "preset", "slow", 0);
    /* open it */
    int ret = avcodec_open2(_codecContext, _codec, nullptr);
    std::stringstream errorStream;
    if (ret < 0) {
        errorStream<<"Could not open codec"<<std::endl;
        throw std::runtime_error(errorStream.str());
    }

    _frame = av_frame_alloc();
    if (!_frame) {
        errorStream<<"Could not allocate video frame"<<std::endl;
        throw std::runtime_error(errorStream.str());
    }
    _frame->format = _codecContext->pix_fmt;
    _frame->width  = _codecContext->width;
    _frame->height = _codecContext->height;

    ret = av_frame_get_buffer(_frame, 32);
    if (ret < 0) {
        errorStream<<"Could not allocate the video frame data"<<std::endl;
        throw std::runtime_error(errorStream.str());
    }
}

boost::property_tree::ptree BerryCam::H264Encoder::getEncoderParameters() {
    return ptree();
}

void BerryCam::H264Encoder::encode(const void *buffer) {
    std::stringstream errorStream;
    int ret = av_frame_make_writable(_frame);
    if (ret < 0) {
        errorStream<<"Unable to make frame writable"<<std::endl;
        throw std::runtime_error(errorStream.str());
    }
    
    if (buffer != nullptr)
        copyBufferToFrame(buffer);

    _frame->pts = _frameCount;
    _frameCount++;


    ret = avcodec_send_frame(_codecContext, buffer != nullptr ? _frame : nullptr);
    if (ret < 0) {
        errorStream<<"Error sending a frame #"<<ret<<std::endl;
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(_codecContext, _tempPacket);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        if (_frameCount == 1) {
            std::cout<<"Check em: "<<_tempPacket->size<<std::endl;
            _ppsFrameStore = std::vector<unsigned char>(_tempPacket->size);
            memcpy(&_ppsFrameStore[0], _tempPacket->data, _tempPacket->size);
        }

        if (_broadcaster != nullptr)
            _broadcaster->SendPacket(_tempPacket->data, _tempPacket->size);
        if ((_frameCount % 30) == 0)
            _broadcaster->SendPacket(&_ppsFrameStore[0], _ppsFrameStore.size());
        av_packet_unref(_tempPacket);
    }
}

void BerryCam::H264Encoder::copyBufferToFrame(const void *buffer) const {
    auto y_size = static_cast<unsigned int>(_frame->linesize[0] * _codecContext->height);
    auto u_size = static_cast<unsigned int>(_frame->linesize[1] * _codecContext->height / 2);
    auto v_size = static_cast<unsigned int>(_frame->linesize[2] * _codecContext->height / 2);

    memcpy(_frame->data[0], (unsigned char*)buffer, y_size);
    memcpy(_frame->data[1], (unsigned char*)buffer + y_size, u_size);
    memcpy(_frame->data[2], (unsigned char*)buffer + y_size + u_size, v_size);
}

