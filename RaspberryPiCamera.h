//
// Created by Zikomo Fields on 2019-09-26.
//

#ifndef BERRYCAM_RASPBERRYPICAMERA_H
#define BERRYCAM_RASPBERRYPICAMERA_H

#include "Camera.h"
#include "Encoder.h"

#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/mmal_parameters_camera.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"


#define MMAL_CAMERA_VIDEO_PORT   1



namespace BerryCam {

    class RaspberryPiCamera : public Camera {

    public:
        explicit RaspberryPiCamera(std::vector<std::shared_ptr<Encoder>> encoders);
        ~RaspberryPiCamera();
        void start() override;
        void stop() override;
        void setCameraParameters(boost::property_tree::ptree &ptr) override;
        boost::property_tree::ptree getCameraParameters() override;
        static void onFrameReceivedStaticCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        void onFrameReceived(unsigned char *frameData) override;

    private:
        static void checkStatus(MMAL_STATUS_T status, const std::string& description);
        void setVideoFormat(unsigned int width, unsigned int height);
        void setCameraVideoPort(unsigned int width, unsigned int height);
        void setFlip(boost::property_tree::ptree& cameraParameters);
        void setRotation(boost::property_tree::ptree& cameraParameters);

        MMAL_COMPONENT_T *_camera;
        MMAL_ES_FORMAT_T *_format;
        MMAL_PORT_T *_cameraVideoPort;
        MMAL_POOL_T *_cameraVideoPortPool;

        std::vector<std::shared_ptr<Encoder>> _encoders;

    };
}


#endif //BERRYCAM_RASPBERRYPICAMERA_H
