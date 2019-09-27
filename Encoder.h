//
// Created by Zikomo Fields on 2019-09-26.
//

#ifndef BERRYCAM_ENCODER_H
#define BERRYCAM_ENCODER_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::property_tree;

class Encoder {
public:
    virtual ~Encoder() = 0;
    virtual void setEncoderParameters(shared_ptr<ptree> encoderParameters) = 0;
    virtual shared_ptr<ptree> getEncoderParameters() = 0;
    virtual void encode(const void*) = 0;
};


#endif //BERRYCAM_ENCODER_H
