//
// Created by Zikomo Fields on 2019-09-26.
//

#include <iostream>
#include "RaspberryPiCamera.h"
#include "Utilities.h"

RaspberryPiCamera::RaspberryPiCamera() {
    bcm_host_init();
    checkStatus(mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera), "mmal_component_create");
    camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    ptree defaultParameters;
    setCameraParameters(defaultParameters);

}

RaspberryPiCamera::~RaspberryPiCamera() = default;

void RaspberryPiCamera::start() {
    camera_video_port_pool = mmal_port_pool_create(camera_video_port,
                                                   camera_video_port->buffer_num,
                                                   camera_video_port->buffer_size);
    checkStatus(mmal_port_enable(camera_video_port, onFrameReceivedStaticCallback), "mmal_port_enable");
    checkStatus(mmal_component_enable(camera), "mmal_component_enable");
    int queueLength = mmal_queue_length(camera_video_port_pool->queue);
    for (int i = 0; i < queueLength; i++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(camera_video_port_pool->queue);
        if (!buffer) {
            cout << "Unable to get a required buffer " << i << " from pool queue" << endl;
        }
        if (mmal_port_send_buffer(camera_video_port, buffer) != MMAL_SUCCESS) {
            cout << "Unable to send a buffer to encoder output port " << i << endl;
        }
    }
    checkStatus(mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1),
            "mmal_port_parameter_set_boolean");
}

void RaspberryPiCamera::stop() {
    mmal_port_disable(camera_video_port);
}

void RaspberryPiCamera::setCameraParameters(ptree &cameraParameters) {
    MMAL_PARAMETER_CAMERA_CONFIG_T camConfig = {
            { MMAL_PARAMETER_CAMERA_CONFIG, sizeof (camConfig)},
            Utilities::SafeGet(cameraParameters, "camera.stills.width", 320u),
            Utilities::SafeGet(cameraParameters, "camera.stills.height", 240u),
            Utilities::SafeGet(cameraParameters, "camera.stills.yuv422", 0u),
            Utilities::SafeGet(cameraParameters, "camera.stills.one_shot",0u),
            Utilities::SafeGet(cameraParameters, "camera.preview.width", 320u),
            Utilities::SafeGet(cameraParameters, "camera.preview.height", 240u),
            Utilities::SafeGet(cameraParameters, "camera.preview.frames", 3u),
            Utilities::SafeGet(cameraParameters, "camera.stills.capture_circular_buffer_height", 0u),
            Utilities::SafeGet(cameraParameters, "camera.preview.fast_resume", 1u),
            MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };
    checkStatus(mmal_port_parameter_set(camera->control, &camConfig.hdr), "mmal_port_parameter_set");

    auto width = cameraParameters.get<unsigned int>("camera.preview.width");
    auto height = cameraParameters.get<unsigned int>("camera.preview.height");

    format = camera_video_port->format;
    format->encoding = MMAL_ENCODING_YV12;
    format->encoding_variant = MMAL_ENCODING_YV12;
    format->es->video.width = width;
    format->es->video.height = height;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = 30;
    format->es->video.frame_rate.den = 1;
    camera_video_port->buffer_size = width * height * 3 / 2;
    camera_video_port->buffer_num = 10;
    camera_video_port->userdata = (MMAL_PORT_USERDATA_T*)this;
    checkStatus(mmal_port_format_commit(camera_video_port), "mmal_port_format_commit");

}

ptree RaspberryPiCamera::getCameraParameters() {
    ptree parameters;
    MMAL_PARAMETER_CAMERA_CONFIG_T camConfig;
    checkStatus(mmal_port_parameter_get(camera->control, &camConfig.hdr),"mmal_port_parameter_get");
    parameters.put<uint32_t>("camera.stills.width", camConfig.max_preview_video_w);
    parameters.put<uint32_t>("camera.stills.height", camConfig.max_preview_video_h);
    parameters.put<uint32_t>("camera.stills.yuv422", camConfig.stills_yuv422);
    parameters.put<uint32_t>("camera.stills.one_shot", camConfig.one_shot_stills);
    parameters.put<uint32_t>("camera.preview.width", camConfig.max_preview_video_w);
    parameters.put<uint32_t>("camera.preview.height", camConfig.max_preview_video_h);
    parameters.put<uint32_t>("camera.preview.frames", camConfig.num_preview_video_frames);
    parameters.put<uint32_t>("camera.stills.capture_circular_buffer_height", camConfig.num_preview_video_frames);
    parameters.put<uint32_t>("camera.preview.fast_resume", camConfig.fast_preview_resume);
    return parameters;
}

void RaspberryPiCamera::checkStatus(MMAL_STATUS_T status, string description = "") {
    if (status != MMAL_SUCCESS) {
        throw runtime_error(description);
    }
}

void RaspberryPiCamera::onFrameReceivedStaticCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    reinterpret_cast<RaspberryPiCamera*>(port->userdata)->onFrameReceived(buffer->data);
}

void RaspberryPiCamera::onFrameReceived(unsigned char *frameData) {

}


