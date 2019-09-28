//
// Created by Zikomo Fields on 2019-09-26.
//

#ifndef BERRYCAM_RASPBERRYPICAMERA_H
#define BERRYCAM_RASPBERRYPICAMERA_H

#include "Camera.h"
#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/mmal_parameters_camera.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "Encoder.h"

#define MMAL_CAMERA_VIDEO_PORT   1
#define CAMERA_STILLS_WIDTH "camera.stills.width"
#define CAMERA_STILLS_HEIGHT "camera.stills.height"
#define CAMERA_STILLS_YUV422 "camera.stills.yuv422"
#define CAMERA_STILLS_ONE_SHOT "camera.stills.one_shot"
#define CAMERA_PREVIEW_WIDTH "camera.preview.width"
#define CAMERA_PREVIEW_HEIGHT "camera.preview.height"
#define CAMERA_PREVIEW_FRAMES "camera.preview.frames"
#define CAMERA_STILLS_CAPTURE_CIRCULAR_BUFFER_HEIGHT "camera.stills.capture_circular_buffer_height"
#define CAMERA_PREVIEW_FAST_RESUME "camera.preview.fast_resume"

namespace BerryCam {

    class RaspberryPiCamera : public Camera {

    public:
        explicit RaspberryPiCamera(std::shared_ptr<Encoder> encoder);
        ~RaspberryPiCamera() override;
        void start() override;
        void stop() override;
        void setCameraParameters(boost::property_tree::ptree &ptr) override;
        boost::property_tree::ptree getCameraParameters() override;
        static void onFrameReceivedStaticCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        void onFrameReceived(unsigned char *frameData) override;

    private:
        void checkStatus(MMAL_STATUS_T status, std::string description);
        void setVideoFormat(unsigned int width, unsigned int height);
        void setCameraVideoPort(unsigned int width, unsigned int height);

        MMAL_COMPONENT_T *_camera;
        MMAL_ES_FORMAT_T *_format;
        MMAL_PORT_T *_cameraVideoPort;
        MMAL_POOL_T *_cameraVideoPortPool;

        std::shared_ptr<Encoder> _encoder;

    };
}


#endif //BERRYCAM_RASPBERRYPICAMERA_H
