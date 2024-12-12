#include "AudioManager.h"
#include <stdexcept>

#include "Utils.h"
#include <iostream>

namespace mc
{
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

    static HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) {
        HRESULT hr = S_OK;
        if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
            return HRESULT_FROM_WIN32(GetLastError());

        DWORD dwChunkType;
        DWORD dwChunkDataSize;
        DWORD dwRIFFDataSize = 0;
        DWORD dwFileType;
        DWORD bytesRead = 0;
        DWORD dwOffset = 0;

        while (hr == S_OK) {
            DWORD dwRead;
            if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());

            if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());

            switch (dwChunkType) {
            case fourccRIFF:
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());
                break;

            default:
                if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                    return HRESULT_FROM_WIN32(GetLastError());
            }

            dwOffset += sizeof(DWORD) * 2;

            if (dwChunkType == fourcc) {
                dwChunkSize = dwChunkDataSize;
                dwChunkDataPosition = dwOffset;
                return S_OK;
            }

            dwOffset += dwChunkDataSize;

            if (bytesRead >= dwRIFFDataSize) return S_FALSE;

        }

        return S_OK;

    }

    static HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
        HRESULT hr = S_OK;
        if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
            return HRESULT_FROM_WIN32(GetLastError());
        DWORD dwRead;
        if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    static HRESULT LoadAudioFile(const char* path, Wav* soundData) {

        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (INVALID_HANDLE_VALUE == hFile)
            return HRESULT_FROM_WIN32(GetLastError());

        if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
            return HRESULT_FROM_WIN32(GetLastError());

        DWORD dwChunkSize;
        DWORD dwChunkPosition;

        // NOTE: Locate the RIFF chuck
        FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
        DWORD filetype;
        ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
        if (filetype != fourccWAVE)
            return S_FALSE;

        // NOTE: Locate the FMT chuck
        FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
        ReadChunkData(hFile, &soundData->format, dwChunkSize, dwChunkPosition);

        // NOTE: Locate the DATA chuck
        FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
        BYTE* pDataBuffer = (BYTE*)malloc(dwChunkSize);
        ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

        // NOTE: Populate XAudio2 buffer
        soundData->size = dwChunkSize;
        soundData->data = (void*)pDataBuffer;

        CloseHandle(hFile);

        return S_OK;
    }




    AudioManager::AudioManager()
    {
        if (FAILED(XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        {
            throw std::runtime_error("Error: creating XAudio2");
        }
        if (FAILED(xAudio2_->CreateMasteringVoice(&masterVoice_)))
        {
            throw std::runtime_error("Error: creating mastering voice");
        }

        // Read the file
        LoadAudioFile("assets/audio/ship.wav", &shipWav);

        // Create the ship source voice
        if (FAILED(xAudio2_->CreateSourceVoice(&shipVoice_, (WAVEFORMATEX*)&shipWav.format, 0,
            XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr)))
        {
            throw std::runtime_error("Error: ship source voice");
        }
        // fill the source voice
        XAUDIO2_BUFFER buffer{};
        buffer.AudioBytes = shipWav.size;
        buffer.pAudioData = (BYTE *)shipWav.data;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        if (FAILED(shipVoice_->SubmitSourceBuffer(&buffer)))
        {
            throw std::runtime_error("Error: filling ship voice source");

        }

        // Read the file
        LoadAudioFile("assets/audio/song.wav", &musicWav);

        // Create the ship source voice
        if (FAILED(xAudio2_->CreateSourceVoice(&musicVoice_, (WAVEFORMATEX*)&musicWav.format, 0,
            XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr)))
        {
            throw std::runtime_error("Error: ship source voice");
        }
        // fill the source voice
        buffer = {};
        buffer.AudioBytes = musicWav.size;
        buffer.pAudioData = (BYTE*)musicWav.data;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        if (FAILED(musicVoice_->SubmitSourceBuffer(&buffer)))
        {
            throw std::runtime_error("Error: filling music voice source");

        }

        // Set volumes
        static float shipVolumes[2] = { 2.0f, 2.0f };
        shipVoice_->SetChannelVolumes(2, shipVolumes);

        static float musicVolumes[2] = { 0.1f, 0.1f };
        musicVoice_->SetChannelVolumes(2, musicVolumes);


        Pause();
    }

    AudioManager::~AudioManager()
    {
        if (musicVoice_)
        {
            musicVoice_->DestroyVoice();
        }
        if (shipVoice_)
        {
            shipVoice_->DestroyVoice();
        }
        if (masterVoice_)
        {
            masterVoice_->DestroyVoice();
        }
        if (xAudio2_)
        {
            xAudio2_->Release();
        }

        free(shipWav.data);
        free(musicWav.data);

    }

    void AudioManager::Start()
    {
        musicVoice_->Start();
        shipVoice_->Start();
    }

    void AudioManager::Pause()
    {
        musicVoice_->Stop();
        shipVoice_->Stop();
    }

    void AudioManager::Update(float thrust)
    {
        float minPitch = 1.0f;
        float maxPitch = 2.0f;
        shipVoice_->SetFrequencyRatio(Utils::Lerp(minPitch, maxPitch, thrust));
    }
}