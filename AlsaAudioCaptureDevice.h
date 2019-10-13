//
// Created by Zikomo Fields on 10/10/19.
//

#ifndef BERRYCAM_ALSAAUDIOCAPTUREDEVICE_H
#define BERRYCAM_ALSAAUDIOCAPTUREDEVICE_H

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#include "AudioCaptureDevice.h"

namespace BerryCam {
    class AlsaAudioCaptureDevice : public AudioCaptureDevice {
    public:
        AlsaAudioCaptureDevice();
        ~AlsaAudioCaptureDevice();
        void start() override;

        void stop() override;

        void setAudioCaptureParameters(boost::property_tree::ptree &ptree) override;

        boost::property_tree::ptree getAudioCaptureParameters() override;

    private:
        void captureThreadWorker();
        long loops{};
        int rc{};
        int size{};
        snd_pcm_t *handle{};
        snd_pcm_hw_params_t *params{};
        unsigned int val{};
        int dir{};
        snd_pcm_uframes_t frames{};
        char *buffer{};
        std::promise<bool> shutDownPromise{};
        std::thread captureThread{};

    };
}


#endif //BERRYCAM_ALSAAUDIOCAPTUREDEVICE_H
