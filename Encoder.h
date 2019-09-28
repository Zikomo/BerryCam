//
// Created by Zikomo Fields on 2019-09-26.
//

#ifndef BERRYCAM_ENCODER_H
#define BERRYCAM_ENCODER_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>



namespace BerryCam {
    class Encoder {
    public:
        virtual void setEncoderParameters(boost::property_tree::ptree &encoderParameters) = 0;
        virtual boost::property_tree::ptree getEncoderParameters() = 0;
        virtual void encode(const void *) = 0;

    };
}


#endif //BERRYCAM_ENCODER_H
