//
// Created by Zikomo Fields on 2019-03-27.
//

#ifndef BERRYCAM_UTILITIES_H
#define BERRYCAM_UTILITIES_H

#include <boost/property_tree/ptree.hpp>
#include <cmath>

//Camera Keys
#define CAMERA_STILLS_WIDTH "camera.stills.width"
#define CAMERA_STILLS_HEIGHT "camera.stills.height"
#define CAMERA_STILLS_YUV422 "camera.stills.yuv422"
#define CAMERA_STILLS_ONE_SHOT "camera.stills.one_shot"
#define CAMERA_PREVIEW_WIDTH "camera.preview.width"
#define CAMERA_PREVIEW_HEIGHT "camera.preview.height"
#define CAMERA_PREVIEW_FRAMES "camera.preview.frames"
#define CAMERA_STILLS_CAPTURE_CIRCULAR_BUFFER_HEIGHT "camera.stills.capture_circular_buffer_height"
#define CAMERA_PREVIEW_FAST_RESUME "camera.preview.fast_resume"
#define CAMERA_ROTATION "camera.rotation"
#define CAMERA_HORIZONTAL_FLIP "camera.preview.horizontal_flip"
#define CAMERA_VERTICAL_FLIP "camera.preview.vertical_flip"

//h264 Encoder Keys
#define ENCODER_BIT_RATE "encoder.h264.bit_rate"
#define ENCODER_FRAME_RATE "encoder.h264.frame_rate"
#define ENCODER_GOP_SIZE "encoder.h264.gop_size"
#define ENCODER_MAX_B_FRAMES "encoder.h264.max_b_frames"

//Single Image Encoder Keys
#define SINGLE_IMAGE_ENCODER_IMAGE_PATH "single_image_encoder.image_path"
#define SINGLE_IMAGE_ENCODER_TARGET_FRAMES_PER_SECOND "single_image_encoder.target_fps"

//Broadcaste Encoder Keys
#define UDP_BROADCAST_ADDRESS "socket.broadcast.address"
#define UDP_BROADCAST_PORT "socket.broadcast.port"
#define UDP_BROADCAST_MTU "socket.broadcast.max_mtu_size"

//Camera default values
#define DEFAULT_CAMERA_RESOLUTION_WIDTH 320u
#define DEFAULT_CAMERA_RESOLUTION_HEIGHT 240u
#define DEFAULT_CAMERA_PREVIEW_FRAMES 3u
#define DEFAULT_CAMERA_HORIZONTAL_FLIP 0
#define DEFAULT_CAMERA_VERTICAL_FLIP 0
#define DEFAULT_CAMERA_ROTATION 0

//Encoder default values
#define DEFAULT_ENCODER_TARGET_FRAMES_PER_SECOND 25
#define DEFAULT_ENCODER_BITRATE 40000
#define DEFAULT_ENCODER_GOP_SIZE 10
#define DEFAULT_ENCODER_MAX_B_FRAMES 1

//UDP Broadcaster defaut values
#define DEFAULT_BROADCAST_ADDRESS "239.255.0.1"
#define DEFAULT_BROADCAST_PORT 9999
#define DEFAULT_BROADCAST_MTU 1450u

//Default Binary Values
#define DEFAULT_FALSE 0u
#define DEFAULT_TRUE 1u


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
    template<class T>
    static T Clamp(double d, T min, T max) {
        if (d < min)
            return min;
        if (d > max)
            return max;
        return std::round(d);
    }
};


#endif //BERRYCAM_UTILITIES_H
