//
// Created by Zikomo Fields on 2019-03-27.
//

#ifndef BERRYCAM_UTILITIES_H
#define BERRYCAM_UTILITIES_H

#include <boost/property_tree/ptree.hpp>

#define CAMERA_STILLS_WIDTH "camera.stills.width"
#define CAMERA_STILLS_HEIGHT "camera.stills.height"
#define CAMERA_STILLS_YUV422 "camera.stills.yuv422"
#define CAMERA_STILLS_ONE_SHOT "camera.stills.one_shot"
#define CAMERA_PREVIEW_WIDTH "camera.preview.width"
#define CAMERA_PREVIEW_HEIGHT "camera.preview.height"
#define CAMERA_PREVIEW_FRAMES "camera.preview.frames"
#define CAMERA_STILLS_CAPTURE_CIRCULAR_BUFFER_HEIGHT "camera.stills.capture_circular_buffer_height"
#define CAMERA_PREVIEW_FAST_RESUME "camera.preview.fast_resume"

#define ENCODER_BIT_RATE "encoder.h264.bit_rate"
#define ENCODER_FRAME_RATE "encoder.h264.frame_rate"
#define ENCODER_GOP_SIZE "encoder.h264.gop_size"
#define MAX_B_FRAMES "encoder.h264.max_b_frames"

class Utilities {
public:
    template<class T>
    static T SafeGet(boost::property_tree::ptree& settings, std::string key, T default_value)
    {
        auto iterator = settings.find(key);
        boost::optional<T> value = settings.get_optional<T>(key);
        if (!value) {
            settings.put<T>(key, default_value);
            return default_value;
        }
        return settings.get<T>(key);
    }

};


#endif //BERRYCAM_UTILITIES_H
