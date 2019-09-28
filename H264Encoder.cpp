//
// Created by Zikomo Fields on 2019-09-28.
//

#include <iostream>
#include "H264Encoder.h"


BerryCam::H264Encoder::H264Encoder() {
    avcodec_register_all();
    std::stringstream errorString;
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        errorString<<"Codec "<<AV_CODEC_ID_H264<<" not found. "<<std::endl;
        throw std::runtime_error(errorString.str());
    }

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        errorString<< "Could not allocate video codec context"<<std::endl;
        throw std::runtime_error(errorString.str());
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        errorString<<"av_packet_alloc failed exiting"<<std::endl;
        throw std::runtime_error(errorString.str());
    }

}

BerryCam::H264Encoder::~H264Encoder() {

}

void BerryCam::H264Encoder::setEncoderParameters(std::shared_ptr<boost::property_tree::ptree> encoderParameters) {

}

std::shared_ptr<boost::property_tree::ptree> BerryCam::H264Encoder::getEncoderParameters() {
    return std::shared_ptr<boost::property_tree::ptree>();
}

void BerryCam::H264Encoder::encode(const void *pVoid) {

}

