//
// Created by Zikomo Fields on 10/2/19.
//

#include "MmalSingleImageEncoder.h"
#include "Utilities.h"

void BerryCam::MmalSingleImageEncoder::setEncoderParameters(boost::property_tree::ptree &encoderParameters) {
    width = encoderParameters.get<unsigned int>(CAMERA_PREVIEW_WIDTH);//Utilities::SafeGet(encoderParameters, CAMERA_PREVIEW_WIDTH, 320u);
    height = encoderParameters.get<unsigned int>(CAMERA_PREVIEW_HEIGHT);

    filename = Utilities::SafeGet<std::string>(encoderParameters, WEBSERVER_IMAGE_PATH, "latest_image.jpg");
    
    portIn->format->encoding = MMAL_ENCODING_RGBA;
    portIn->format->es->video.width = VCOS_ALIGN_UP(width, 32);
    portIn->format->es->video.height = VCOS_ALIGN_UP(height, 16);
    portIn->format->es->video.crop.x = 0;
    portIn->format->es->video.crop.y = 0;
    portIn->format->es->video.crop.width = width;
    portIn->format->es->video.crop.height = height;
    if (mmal_port_format_commit(portIn) != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to commit input port format\n");
        exit(1);
    }

    portIn->buffer_size = portIn->buffer_size_recommended;
    portIn->buffer_num = portIn->buffer_num_recommended;

    if (mmal_wrapper_port_enable(portIn, MMAL_WRAPPER_FLAG_PAYLOAD_ALLOCATE)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to enable input port\n");
        exit(1);
    }

    printf("- input %4.4s %ux%u\n",
           (char*)&portIn->format->encoding,
           portIn->format->es->video.width, portIn->format->es->video.height);

    // Configure output

    portOut = encoder->output[0];

    if (portOut->is_enabled) {
        if (mmal_wrapper_port_disable(portOut) != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to disable output port\n");
            exit(1);
        }
    }

    portOut->format->encoding = encoding;
    if (mmal_port_format_commit(portOut) != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to commit output port format\n");
        exit(1);
    }

    mmal_port_parameter_set_uint32(portOut, MMAL_PARAMETER_JPEG_Q_FACTOR, 100);

    portOut->buffer_size = portOut->buffer_size_recommended;
    portOut->buffer_num = portOut->buffer_num_recommended;

    if (mmal_wrapper_port_enable(portOut, MMAL_WRAPPER_FLAG_PAYLOAD_ALLOCATE)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to enable output port\n");
        exit(1);
    }

    printf("- output %4.4s\n", (char*)&encoding);


}

boost::property_tree::ptree BerryCam::MmalSingleImageEncoder::getEncoderParameters() {
    return boost::property_tree::ptree();
}

void BerryCam::MmalSingleImageEncoder::encode(const void *pVoid) {
    if (_frameCount % 30 != 0) {
        _frameCount++;
        return;
    }
    _frameCount++;


    int eos = 0;
    int sent = 0;
    int outputWritten = 0;
    FILE* outFile;
    int nw;


    outFile = fopen(filename.c_str(), "w");
    if (!outFile) {
        fprintf(stderr, "Failed to open file %s (%s)\n", filename.c_str(), strerror(errno));
        exit(1);
    }

    while (!eos) {

        // Send output buffers to be filled with encoded image.
        while (mmal_wrapper_buffer_get_empty(portOut, &out, 0) == MMAL_SUCCESS) {
            if (mmal_port_send_buffer(portOut, out) != MMAL_SUCCESS) {
                fprintf(stderr, "Failed to send buffer\n");
                break;
            }
        }

        // Send image to be encoded.
        if (!sent && mmal_wrapper_buffer_get_empty(portIn, &in, 0) == MMAL_SUCCESS) {
            printf("- sending %u bytes to encoder\n", in->alloc_size);
            //create_rgba_test_image(in->data, in->alloc_size, portIn->format->es->video.width);
            //memcpy(in->data, rgba, in->alloc_size);
            unsigned char* rgba = in->data;
            auto* yuv = reinterpret_cast<const unsigned char*>(pVoid);
            int total = width * height, y = 0, u = 0 , v = 0;
            for (int column = 0; column < height; column++) {
                for (int x = 0; x < width; x++) {
                    y = yuv[column * width + x];
                    u = yuv[(column / 2) * (width / 2) + (x / 2) + total];
                    v = yuv[(column / 2) * (width / 2) + (x / 2) + total + (total / 4)];
                    //UV420toRGB888(y, u, v);
                    *(rgba++) = Utilities::Clamp(y +  (1.370705 * (v-128)), 0, 0xFF);
                    *(rgba++) = Utilities::Clamp(y - (0.698001 * (v-128)) - (0.337633 * (u-128)), 0, 0xFF);
                    *(rgba++) =  Utilities::Clamp(y + (1.732446 * (u-128)), 0, 0xFF);
                    *(rgba++) = 0xFF;
                }
            }
            in->length = in->alloc_size;
            in->flags = MMAL_BUFFER_HEADER_FLAG_EOS;
            if (mmal_port_send_buffer(portIn, in) != MMAL_SUCCESS) {
                fprintf(stderr, "Failed to send buffer\n");
                break;
            }
            sent = 1;
        }

        // Get filled output buffers.
        status = mmal_wrapper_buffer_get_full(portOut, &out, 0);
        if (status == MMAL_EAGAIN) {
            // No buffer available, wait for callback and loop.
            //vcos_semaphore_wait(&sem);
            continue;
        } else if (status != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to get full buffer\n");
            exit(1);
        }

        printf("- received %i bytes\n", out->length);
        eos = out->flags & MMAL_BUFFER_HEADER_FLAG_EOS;

        nw = fwrite(out->data, 1, out->length, outFile);
        if (nw != out->length) {
            fprintf(stderr, "Failed to write complete buffer\n");
            exit(1);
        }
        outputWritten += nw;
        mmal_buffer_header_release(out);

    }


    mmal_port_flush(portOut);

    fclose(outFile);
    printf("- written %u bytes to %s\n\n", outputWritten, filename.c_str());

}

BerryCam::MmalSingleImageEncoder::MmalSingleImageEncoder() :
        _frameCount(0) {
    if (mmal_wrapper_create(&encoder, MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER)
        != MMAL_SUCCESS) {
        fprintf(stderr, "Failed to create mmal component\n");
        exit(1);
    }
    encoding = MMAL_ENCODING_JPEG;

    portIn = encoder->input[0];
    encoder->status = MMAL_SUCCESS;

    if (portIn->is_enabled) {
        if (mmal_wrapper_port_disable(portIn) != MMAL_SUCCESS) {
            fprintf(stderr, "Failed to disable input port\n");
            exit(1);
        }
    }
}

BerryCam::MmalSingleImageEncoder::~MmalSingleImageEncoder() {

}
