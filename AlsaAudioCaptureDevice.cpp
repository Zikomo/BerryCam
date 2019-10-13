//
// Created by Zikomo Fields on 10/10/19.
//

#include <future>
#include "AlsaAudioCaptureDevice.h"

using namespace std::chrono_literals;

void BerryCam::AlsaAudioCaptureDevice::start() {

    captureThread = std::thread(&AlsaAudioCaptureDevice::captureThreadWorker, this);
//    while (loops > 0) {
//        loops--;
//        rc = snd_pcm_readi(handle, buffer, frames);
//        if (rc == -EPIPE) {
//            /* EPIPE means overrun */
//            fprintf(stderr, "overrun occurred\n");
//            snd_pcm_prepare(handle);
//        } else if (rc < 0) {
//            fprintf(stderr,
//                    "error from read: %s\n",
//                    snd_strerror(rc));
//        } else if (rc != (int)frames) {
//            fprintf(stderr, "short read, read %d frames\n", rc);
//        }
//        rc = write(1, buffer, size);
//        if (rc != size)
//            fprintf(stderr,
//                    "short write: wrote %d bytes\n", rc);
//    }

}

void BerryCam::AlsaAudioCaptureDevice::stop() {
    shutDownPromise.set_value(true);
    captureThread.join();

}

void BerryCam::AlsaAudioCaptureDevice::setAudioCaptureParameters(boost::property_tree::ptree &ptree) {

}

boost::property_tree::ptree BerryCam::AlsaAudioCaptureDevice::getAudioCaptureParameters() {
    return boost::property_tree::ptree();
}

BerryCam::AlsaAudioCaptureDevice::AlsaAudioCaptureDevice() {
    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, "default",
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params,
                                 SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 1);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params,
                                    &val, &dir);

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle,
                                           params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr,
                "unable to set hw parameters: %s\n",
                snd_strerror(rc));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
    size = frames * 2; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);
    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params,
                                      &val, &dir);
}

BerryCam::AlsaAudioCaptureDevice::~AlsaAudioCaptureDevice() {
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
}

void BerryCam::AlsaAudioCaptureDevice::captureThreadWorker() {
    std::future<bool> shutDownStatus = shutDownPromise.get_future();
    while (shutDownStatus.wait_for(0ms) != std::future_status::ready) {

        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr,
                    "error from read: %s\n",
                    snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        //rc = write(1, buffer, size);
        if (rc != size)
            fprintf(stderr,
                    "short write: wrote %d bytes\n", rc);
    }
}