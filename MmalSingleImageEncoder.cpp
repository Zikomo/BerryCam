//
// Created by Zikomo Fields on 10/2/19.
//

#include "MmalSingleImageEncoder.h"
#include "Utilities.h"

BerryCam::MmalSingleImageEncoder::MmalSingleImageEncoder() :
        _encoder(nullptr),
        _encoding(MMAL_ENCODING_JPEG),
        _frameCount(0),
        _portIn(nullptr),
        _portOut(nullptr),
        _input(nullptr),
        _output(nullptr),
        _status(MMAL_SUCCESS),
        _filename(""),
        _width(0),
        _height(0),
        _targetFramesPerSecond(0)
{
    if (mmal_wrapper_create(&_encoder, MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to create mmal component\n");
        exit(1);
    }

    _portIn = _encoder->input[0];
    _encoder->status = MMAL_SUCCESS;

    if (_portIn->is_enabled) {
        if (mmal_wrapper_port_disable(_portIn) != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to disable input port\n");
            exit(1);
        }
    }
}

BerryCam::MmalSingleImageEncoder::~MmalSingleImageEncoder() = default;

void BerryCam::MmalSingleImageEncoder::setEncoderParameters(boost::property_tree::ptree &encoderParameters) {
    _width = encoderParameters.get<unsigned int>(CAMERA_PREVIEW_WIDTH);//Utilities::SafeGet(encoderParameters, CAMERA_PREVIEW_WIDTH, 320u);
    _height = encoderParameters.get<unsigned int>(CAMERA_PREVIEW_HEIGHT);
    _targetFramesPerSecond =  Utilities::SafeGet<unsigned int>(encoderParameters, SINGLE_IMAGE_ENCODER_TARGET_FRAMES_PER_SECOND, 1);
    _targetFramesPerSecond = Utilities::Clamp(_targetFramesPerSecond, 1, 30);


    _filename = Utilities::SafeGet<std::string>(encoderParameters, SINGLE_IMAGE_ENCODER_IMAGE_PATH, "latest_image.jpg");

    _portIn->format->encoding = MMAL_ENCODING_RGBA;
    _portIn->format->es->video.width = VCOS_ALIGN_UP(_width, 32);
    _portIn->format->es->video.height = VCOS_ALIGN_UP(_height, 16);
    _portIn->format->es->video.crop.x = 0;
    _portIn->format->es->video.crop.y = 0;
    _portIn->format->es->video.crop.width = _width;
    _portIn->format->es->video.crop.height = _height;
    if (mmal_port_format_commit(_portIn) != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to commit input port format\n");
        exit(1);
    }

    _portIn->buffer_size = _portIn->buffer_size_recommended;
    _portIn->buffer_num = _portIn->buffer_num_recommended;

    if (mmal_wrapper_port_enable(_portIn, MMAL_WRAPPER_FLAG_PAYLOAD_ALLOCATE)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to enable input port\n");
        exit(1);
    }

    printf("- input %4.4s %ux%u\n",
           (char*)&_portIn->format->encoding,
           _portIn->format->es->video.width, _portIn->format->es->video.height);

    // Configure output

    _portOut = _encoder->output[0];

    if (_portOut->is_enabled) {
        if (mmal_wrapper_port_disable(_portOut) != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to disable output port\n");
            exit(1);
        }
    }

    _portOut->format->encoding = _encoding;
    if (mmal_port_format_commit(_portOut) != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to commit output port format\n");
        exit(1);
    }

    mmal_port_parameter_set_uint32(_portOut, MMAL_PARAMETER_JPEG_Q_FACTOR, 100);

    _portOut->buffer_size = _portOut->buffer_size_recommended;
    _portOut->buffer_num = _portOut->buffer_num_recommended;

    if (mmal_wrapper_port_enable(_portOut, MMAL_WRAPPER_FLAG_PAYLOAD_ALLOCATE)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to enable output port\n");
        exit(1);
    }

    printf("- output %4.4s\n", (char*)&_encoding);


}

boost::property_tree::ptree BerryCam::MmalSingleImageEncoder::getEncoderParameters() {
    return boost::property_tree::ptree();
}

void BerryCam::MmalSingleImageEncoder::encode(const void *image) {
    if (_frameCount % (30 / _targetFramesPerSecond ) != 0) {
        _frameCount++;
        return;
    }
    _frameCount++;


    int eos = 0;
    int sent = 0;
    int outputWritten = 0;
    FILE* outFile;
    int nw;


    outFile = fopen(_filename.c_str(), "w");
    if (!outFile) {
        fprintf(stderr, "Failed to open file %s (%s)\n", _filename.c_str(), strerror(errno));
        exit(1);
    }

    while (!eos) {

        // Send output buffers to be filled with encoded image.
        while (mmal_wrapper_buffer_get_empty(_portOut, &_output, 0) == MMAL_SUCCESS) {
            if (mmal_port_send_buffer(_portOut, _output) != MMAL_SUCCESS) {
                fprintf(stderr, "Failed to send buffer\n");
                break;
            }
        }

        // Send image to be encoded.
        if (!sent && mmal_wrapper_buffer_get_empty(_portIn, &_input, 0) == MMAL_SUCCESS) {
            printf("- sending %u bytes to encoder\n", _input->alloc_size);
            //create_rgba_test_image(in->data, in->alloc_size, portIn->format->es->video.width);
            //memcpy(in->data, rgba, in->alloc_size);
            unsigned char* rgba = _input->data;
            auto* yuv = reinterpret_cast<const unsigned char*>(image);
            unsigned int total = _width * _height, y = 0, u = 0 , v = 0;
            for (unsigned int column = 0; column < _height; column++) {
                for (unsigned int x = 0; x < _width; x++) {
                    y = yuv[column * _width + x];
                    u = yuv[(column / 2) * (_width / 2) + (x / 2) + total];
                    v = yuv[(column / 2) * (_width / 2) + (x / 2) + total + (total / 4)];
                    //UV420toRGB888(y, u, v);
                    *(rgba++) = Utilities::Clamp(y +  (1.370705 * (v-128)), 0, 0xFF);
                    *(rgba++) = Utilities::Clamp(y - (0.698001 * (v-128)) - (0.337633 * (u-128)), 0, 0xFF);
                    *(rgba++) =  Utilities::Clamp(y + (1.732446 * (u-128)), 0, 0xFF);
                    *(rgba++) = 0xFF;
                }
            }
            _input->length = _input->alloc_size;
            _input->flags = MMAL_BUFFER_HEADER_FLAG_EOS;
            if (mmal_port_send_buffer(_portIn, _input) != MMAL_SUCCESS) {
                fprintf(stderr, "Failed to send buffer\n");
                break;
            }
            sent = 1;
        }

        // Get filled output buffers.
        _status = mmal_wrapper_buffer_get_full(_portOut, &_output, 0);
        if (_status == MMAL_EAGAIN) {
            // No buffer available, wait for callback and loop.
            //vcos_semaphore_wait(&sem);
            continue;
        } else if (_status != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to get full buffer\n");
            exit(1);
        }

        printf("- received %i bytes\n", _output->length);
        eos = _output->flags & MMAL_BUFFER_HEADER_FLAG_EOS;

        nw = fwrite(_output->data, 1, _output->length, outFile);
        if (nw != _output->length) {
            fprintf(stderr, "Failed to write complete buffer\n");
            exit(1);
        }
        outputWritten += nw;
        mmal_buffer_header_release(_output);

    }


    mmal_port_flush(_portOut);

    fclose(outFile);
    printf("- written %u bytes to %s\n\n", outputWritten, _filename.c_str());

}
