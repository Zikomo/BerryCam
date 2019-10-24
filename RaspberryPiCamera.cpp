//
// Created by Zikomo Fields on 2019-09-26.
//

#include <iostream>
#include <utility>
#include "RaspberryPiCamera.h"
#include "Utilities.h"

using namespace boost::property_tree;

BerryCam::RaspberryPiCamera::RaspberryPiCamera(std::vector<std::shared_ptr<Encoder>>  encoders) :
    _camera(nullptr),
    _format(nullptr),
    _cameraVideoPort(nullptr),
    _cameraVideoPortPool(nullptr),
    _encoders(std::move(encoders)) {
    bcm_host_init();
    checkStatus(mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &_camera), "mmal_component_create");
    _cameraVideoPort = _camera->output[MMAL_CAMERA_VIDEO_PORT];
    ptree defaultParameters;
    setCameraParameters(defaultParameters);
}

BerryCam::RaspberryPiCamera::~RaspberryPiCamera() = default;

void BerryCam::RaspberryPiCamera::start() {
    _cameraVideoPortPool = mmal_port_pool_create(_cameraVideoPort,
            _cameraVideoPort->buffer_num,
            _cameraVideoPort->buffer_size);
    checkStatus(mmal_port_enable(_cameraVideoPort, onFrameReceivedStaticCallback), "mmal_port_enable");
    checkStatus(mmal_component_enable(_camera), "mmal_component_enable");
    unsigned int queueLength = mmal_queue_length(_cameraVideoPortPool->queue);
    for (unsigned int i = 0; i < queueLength; i++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(_cameraVideoPortPool->queue);
        if (!buffer) {
            std::cout << "Unable to get a required buffer " << i << " from pool queue" << std::endl;
        }
        if (mmal_port_send_buffer(_cameraVideoPort, buffer) != MMAL_SUCCESS) {
            std::cout << "Unable to send a buffer to encoder output port " << i << std::endl;
        }
    }
    checkStatus(mmal_port_parameter_set_boolean(_cameraVideoPort, MMAL_PARAMETER_CAPTURE, 1),
            "mmal_port_parameter_set_boolean");
}

void BerryCam::RaspberryPiCamera::stop() {
    mmal_port_disable(_cameraVideoPort);
}

void BerryCam::RaspberryPiCamera::setCameraParameters(ptree &cameraParameters) {
    MMAL_PARAMETER_CAMERA_CONFIG_T camConfig = {
            { MMAL_PARAMETER_CAMERA_CONFIG, sizeof (camConfig)},
            Utilities::SafeGet(cameraParameters, CAMERA_STILLS_WIDTH, DEFAULT_CAMERA_RESOLUTION_WIDTH),
            Utilities::SafeGet(cameraParameters, CAMERA_STILLS_HEIGHT, DEFAULT_CAMERA_RESOLUTION_HEIGHT),
            Utilities::SafeGet(cameraParameters, CAMERA_STILLS_YUV422, DEFAULT_FALSE),
            Utilities::SafeGet(cameraParameters, CAMERA_STILLS_ONE_SHOT,DEFAULT_FALSE),
            Utilities::SafeGet(cameraParameters, CAMERA_PREVIEW_WIDTH, DEFAULT_CAMERA_RESOLUTION_WIDTH),
            Utilities::SafeGet(cameraParameters, CAMERA_PREVIEW_HEIGHT, DEFAULT_CAMERA_RESOLUTION_HEIGHT),
            Utilities::SafeGet(cameraParameters, CAMERA_PREVIEW_FRAMES, DEFAULT_CAMERA_PREVIEW_FRAMES),
            Utilities::SafeGet(cameraParameters, CAMERA_STILLS_CAPTURE_CIRCULAR_BUFFER_HEIGHT, DEFAULT_FALSE),
            Utilities::SafeGet(cameraParameters, CAMERA_PREVIEW_FAST_RESUME, DEFAULT_TRUE),
            MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };
    checkStatus(mmal_port_parameter_set(_camera->control, &camConfig.hdr), "mmal_port_parameter_set");

    setFlip(cameraParameters);
    setRotation(cameraParameters);

    auto width = cameraParameters.get<unsigned int>(CAMERA_PREVIEW_WIDTH);
    auto height = cameraParameters.get<unsigned int>(CAMERA_PREVIEW_HEIGHT);

    setVideoFormat(width, height);
    setCameraVideoPort(width, height);
    checkStatus(mmal_port_format_commit(_cameraVideoPort), "mmal_port_format_commit");
}

void BerryCam::RaspberryPiCamera::setCameraVideoPort(unsigned int width, unsigned int height)  {
    _cameraVideoPort->buffer_size = width * height * 3 / 2;
    _cameraVideoPort->buffer_num = 10;
    _cameraVideoPort->userdata = (MMAL_PORT_USERDATA_T *)this;
}

//FIXME Ignoring warning
//  The following boilerplate code causes warning: Use of a signed integer operand with a binary bitwise operator
//      _format->encoding = MMAL_ENCODING_YV12;
//      _format->encoding_variant = MMAL_ENCODING_YV12;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
void BerryCam::RaspberryPiCamera::setVideoFormat(unsigned int width, unsigned int height) {
    _format = _cameraVideoPort->format;
    _format->encoding = MMAL_ENCODING_YV12;
    _format->encoding_variant = MMAL_ENCODING_YV12;
    _format->es->video.width = width;
    _format->es->video.height = height;
    _format->es->video.crop.x = 0;
    _format->es->video.crop.y = 0;
    _format->es->video.crop.width = width;
    _format->es->video.crop.height = height;
    _format->es->video.frame_rate.num = DEFAULT_ENCODER_TARGET_FRAMES_PER_SECOND;
    _format->es->video.frame_rate.den = 1;
}
#pragma clang diagnostic pop

ptree BerryCam::RaspberryPiCamera::getCameraParameters() {
    ptree parameters;
    MMAL_PARAMETER_CAMERA_CONFIG_T camConfig;
    checkStatus(mmal_port_parameter_get(_camera->control, &camConfig.hdr),"mmal_port_parameter_get");
    parameters.put<uint32_t>(CAMERA_STILLS_WIDTH, camConfig.max_preview_video_w);
    parameters.put<uint32_t>(CAMERA_STILLS_HEIGHT, camConfig.max_preview_video_h);
    parameters.put<uint32_t>(CAMERA_STILLS_YUV422, camConfig.stills_yuv422);
    parameters.put<uint32_t>(CAMERA_STILLS_ONE_SHOT, camConfig.one_shot_stills);
    parameters.put<uint32_t>(CAMERA_PREVIEW_WIDTH, camConfig.max_preview_video_w);
    parameters.put<uint32_t>(CAMERA_PREVIEW_HEIGHT, camConfig.max_preview_video_h);
    parameters.put<uint32_t>(CAMERA_PREVIEW_FRAMES, camConfig.num_preview_video_frames);
    parameters.put<uint32_t>(CAMERA_STILLS_CAPTURE_CIRCULAR_BUFFER_HEIGHT, camConfig.num_preview_video_frames);
    parameters.put<uint32_t>(CAMERA_PREVIEW_FAST_RESUME, camConfig.fast_preview_resume);
    return parameters;
}

void BerryCam::RaspberryPiCamera::checkStatus(MMAL_STATUS_T status, const std::string& description = "") {
    if (status != MMAL_SUCCESS) {
        throw std::runtime_error(description);
    }
}

void BerryCam::RaspberryPiCamera::onFrameReceivedStaticCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    //lock the buffer for use.
    mmal_buffer_header_mem_lock(buffer);
    auto * raspberryPiCamera = reinterpret_cast<RaspberryPiCamera*>(port->userdata);
    //Call our local callback
    raspberryPiCamera->onFrameReceived(buffer->data);
    //Release and return the buffer back to the pool
    MMAL_BUFFER_HEADER_T *new_buffer;
    mmal_buffer_header_release(buffer);
    if (port->is_enabled) {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        new_buffer = mmal_queue_get(raspberryPiCamera->_cameraVideoPortPool->queue);
        if (new_buffer)
            status = mmal_port_send_buffer(port, new_buffer);
        if (!new_buffer || status != MMAL_SUCCESS)
            std::cerr << "Unable to return a buffer to the video port" <<std::endl;
    }
}

void BerryCam::RaspberryPiCamera::onFrameReceived(unsigned char *frameData) {
    if ((_encoders.empty()) || (frameData == nullptr))
        return;
    for (const auto& encoder : _encoders) {
        encoder->encode(frameData);
    }
}

void BerryCam::RaspberryPiCamera::setFlip(ptree &cameraParameters) {
    MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof(MMAL_PARAMETER_MIRROR_T)}, MMAL_PARAM_MIRROR_NONE};

    int horizontalFlip = Utilities::SafeGet<int>(cameraParameters, CAMERA_HORIZONTAL_FLIP, DEFAULT_CAMERA_HORIZONTAL_FLIP);
    int verticalFlip = Utilities::SafeGet<int>(cameraParameters, CAMERA_VERTICAL_FLIP, DEFAULT_CAMERA_VERTICAL_FLIP);
    
    if (horizontalFlip && verticalFlip)
        mirror.value = MMAL_PARAM_MIRROR_BOTH;
    else if (horizontalFlip)
        mirror.value = MMAL_PARAM_MIRROR_HORIZONTAL;
    else if (verticalFlip)
        mirror.value = MMAL_PARAM_MIRROR_VERTICAL;

    checkStatus(mmal_port_parameter_set(_camera->output[0], &mirror.hdr));
    checkStatus(mmal_port_parameter_set(_camera->output[1], &mirror.hdr));
    checkStatus(mmal_port_parameter_set(_camera->output[2], &mirror.hdr));
}

void BerryCam::RaspberryPiCamera::setRotation(boost::property_tree::ptree &cameraParameters) {
    int rotation = Utilities::SafeGet<int>(cameraParameters, CAMERA_ROTATION, DEFAULT_CAMERA_ROTATION);

    int my_rotation = ((rotation % 360 ) / 90) * 90;

    checkStatus(mmal_port_parameter_set_int32(_camera->output[0], MMAL_PARAMETER_ROTATION, my_rotation));
    checkStatus(mmal_port_parameter_set_int32(_camera->output[1], MMAL_PARAMETER_ROTATION, my_rotation));
    checkStatus(mmal_port_parameter_set_int32(_camera->output[2], MMAL_PARAMETER_ROTATION, my_rotation));
}


