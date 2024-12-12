#pragma once

#include <windows.h>
#include <xaudio2.h>
#include <wrl.h>

namespace mc
{
    struct Wav
    {
        WAVEFORMATEXTENSIBLE format;
        void* data;
        size_t size;
    };

    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager();

        void Start();
        void Pause();
        void Update(float thrust);
    private:
        IXAudio2 *xAudio2_;
        IXAudio2MasteringVoice *masterVoice_;
        IXAudio2SourceVoice *shipVoice_;
        IXAudio2SourceVoice *musicVoice_;

        Wav shipWav;
        Wav musicWav;
    };
}

