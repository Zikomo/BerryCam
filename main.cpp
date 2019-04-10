#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/mmal_parameters_camera.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "Broadcaster.h"


#define MMAL_CAMERA_VIDEO_PORT   1


using namespace std;
using namespace std::chrono_literals;
using namespace boost::property_tree;

int frame_count = 0;
MMAL_POOL_T *camera_video_port_pool = nullptr;
ofstream mpeg_file;
AVFrame *frame;
AVCodecContext *codec_context= nullptr;
AVPacket *pkt;

uint8_t * output_buffer = nullptr;
std::mutex callback_mutex;

static void encode(AVFrame *encode_frame, Broadcaster* broadcaster)
{
    int ret;

    /* send the frame to the encoder */
    if (encode_frame)
        cout<<"Send frame"<<encode_frame->pts<<endl;
        //printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(codec_context, encode_frame);

    if (ret < 0) {
        cerr<<"Error sending a frame #"<<ret<<endl;
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_context, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        cout<<"Write packet "<<pkt->pts<< "(size = " << pkt->size << ")"<<endl;

        if (broadcaster != nullptr)
            broadcaster->SendPacket(pkt->data, pkt->size);
        av_packet_unref(pkt);
    }

}


void color_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    lock_guard<std::mutex> callback_lock_guard(callback_mutex);
    MMAL_BUFFER_HEADER_T *new_buffer;
    mmal_buffer_header_mem_lock(buffer);
    if (output_buffer == nullptr) {
        output_buffer = static_cast<uint8_t *>(malloc(buffer->alloc_size));
    }

    int ret = av_frame_make_writable(frame);
    if (ret < 0)
        exit(1);


    unsigned char* pointer = buffer->data;

    cout<<(buffer->length)<<endl;

    int y_size = frame->linesize[0] * codec_context->height;
    int u_size = frame->linesize[1] * codec_context->height / 2;
    int v_size = frame->linesize[2] * codec_context->height / 2;

    cout<< y_size << "x" << u_size << "x" << v_size << endl;

    memcpy(frame->data[0], pointer, y_size);
    //memset(frame->data[0], 0, y_size);
    memcpy(frame->data[1], pointer + y_size, u_size);
    //memset(frame->data[1], 0, u_size);
    //memset(frame->data[2], 0, v_size);
    memcpy(frame->data[2], pointer + y_size + u_size, v_size);

    cout<<"Total size: "<<y_size + u_size + v_size<<endl;

    frame->pts = frame_count;


    //avcodec_encode_video(codec_context, output_buffer, buffer->length, frame);
    //mpeg_file.write(reinterpret_cast<const char *>(pointer), buffer->length);

    cout<<"Data read from camera! Frame # "<<frame_count<<endl;
    mmal_buffer_header_release(buffer);
    if (port->is_enabled) {
        MMAL_STATUS_T status;
        new_buffer = mmal_queue_get(camera_video_port_pool->queue);
        if (new_buffer)
            status = mmal_port_send_buffer(port, new_buffer);
        if (!new_buffer || status != MMAL_SUCCESS)
            cerr << "Unable to return a buffer to the video port" <<endl;
    }
    encode(frame, (Broadcaster*)port->userdata);
    frame_count++;
}

int main(int argc, char **argv) {
    shared_ptr<ptree> settings = make_shared<ptree>();


    try {
        json_parser::read_json("settings.json", *settings);
    }
    catch (json_parser_error error) {

    }

    boost::asio::io_service io_service;
    std::cout<<"Hello world!"<<std::endl;
    Broadcaster broadcaster(settings, io_service);


    const char  *codec_name;
    const AVCodec *codec;

    int ret;

    uint8_t endcode[] = { 0, 0, 1, 0xb7 };


    avcodec_register_all();

    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);

    if (!codec) {
        cerr<<"Codec "<<codec_name<<" not found"<<endl;
        exit(1);
    }

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        cerr<< "Could not allocate video codec context"<<endl;
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        cerr<<"av_packet_alloc failed exiting"<<endl;
        exit(1);
    }

    uint32_t width = 320;
    uint32_t height = 240;

    /* put sample parameters */
    codec_context->bit_rate = 400000;
    /* resolution must be a multiple of two */
    codec_context->width = width;
    codec_context->height = height;
    /* frames per second */
    codec_context->time_base = (AVRational){1, 25};
    codec_context->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    codec_context->gop_size = 10;
    codec_context->max_b_frames = 1;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(codec_context->priv_data, "preset", "slow", 0);
    /* open it */
    ret = avcodec_open2(codec_context, codec, nullptr);
    if (ret < 0) {
        //fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        cerr<<"Could not open codec"<<endl;
        exit(1);
    }


    frame = av_frame_alloc();
    if (!frame) {
        cerr<<"Could not allocate video frame"<<endl;
        exit(1);
    }
    frame->format = codec_context->pix_fmt;
    frame->width  = codec_context->width;
    frame->height = codec_context->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        cerr<<"Could not allocate the video frame data"<<endl;
        exit(1);
    }

    MMAL_COMPONENT_T *camera;

    MMAL_ES_FORMAT_T *format;
    MMAL_STATUS_T status;
    MMAL_PORT_T *camera_video_port;

    bcm_host_init();

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    if (status != MMAL_SUCCESS) {
        cerr<<"Creating Camera Failed"<<endl;
        return -1;
    }

    camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];

    {
        MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {{ MMAL_PARAMETER_CAMERA_CONFIG, sizeof (cam_config)}, width, height, 0, 0,width, height, 3, 0, 1, MMAL_PARAM_TIMESTAMP_MODE_RESET_STC };
        mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }


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
    status = mmal_port_format_commit(camera_video_port);
    camera_video_port->userdata = (MMAL_PORT_USERDATA_T*)&broadcaster;
    if (status != MMAL_SUCCESS) {
        cerr<<"Video Port Commit Failed"<<endl;
        return -1;
    }
    camera_video_port_pool = mmal_port_pool_create(camera_video_port,
                                                   camera_video_port->buffer_num, camera_video_port->buffer_size);

    mpeg_file.open("test.mp4", ios_base::out | ios_base::binary | ios_base::trunc);


    status = mmal_port_enable(camera_video_port, color_callback);
    if (status != MMAL_SUCCESS)
        cerr<< "Error: unable to enable camera video port" <<endl;
    else
        cout << "Attached color callback" << endl;


    status = mmal_component_enable(camera);
    // Send all the buffers to the camera video port
    int num = mmal_queue_length(camera_video_port_pool->queue);
    int q;
    for (q = 0; q < num; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(camera_video_port_pool->queue);
        if (!buffer) {
            cout << "Unable to get a required buffer " << q << " from pool queue" << endl;
        }
        if (mmal_port_send_buffer(camera_video_port, buffer) != MMAL_SUCCESS) {
            cout << "Unable to send a buffer to encoder output port " << q << endl;
        }
    }
    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
        //printf("%s: Failed to start capture\n", __func__);
        cerr<< "Failed to start capture" << endl;
    }
    cout << "Capture started" << endl;

    char c;
    cin>>c;

    //this_thread::sleep_for(10s);
    mmal_port_disable(camera_video_port);
    encode(nullptr, nullptr);



    avcodec_free_context(&codec_context);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    try {
        json_parser::write_json("settings.json", *settings);
    }
    catch (json_parser_error error) {

    }

    return 0;
}