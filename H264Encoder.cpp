//
// Created by Zikomo Fields on 2019-09-28.
//

#include <iostream>
#include "H264Encoder.h"

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
    _codec = avcodec_find_encoder(AV_CODEC_ID_H264);
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

    uint32_t width = 320;
    uint32_t height = 240;

    /* put sample parameters */
    _codecContext->bit_rate = 400000;
    /* resolution must be a multiple of two */
    _codecContext->width = width;
    _codecContext->height = height;
    /* frames per second */
    _codecContext->time_base = (AVRational){1, 25};
    _codecContext->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    _codecContext->gop_size = 10;
    _codecContext->max_b_frames = 1;
    _codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (_codec->id == AV_CODEC_ID_H264)
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

    std::cout<<"Frame count:"<<_frameCount<<std::endl;

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

        if (_broadcaster != nullptr)
            _broadcaster->SendPacket(_tempPacket->data, _tempPacket->size);
        av_packet_unref(_tempPacket);
    }


}

void BerryCam::H264Encoder::copyBufferToFrame(const void *buffer) const {
    int y_size = this->_frame->linesize[0] * this->_codecContext->height;
    int u_size = this->_frame->linesize[1] * this->_codecContext->height / 2;
    int v_size = this->_frame->linesize[2] * this->_codecContext->height / 2;

    memcpy(this->_frame->data[0], (unsigned char*)buffer, y_size);
    memcpy(this->_frame->data[1], (unsigned char*)buffer + y_size, u_size);
    memcpy(this->_frame->data[2], (unsigned char*)buffer + y_size + u_size, v_size);


}

