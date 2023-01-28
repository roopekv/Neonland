#include "pch.h"
#include "AudioPlayer.hpp"

using namespace winrt::Windows::ApplicationModel;

AudioPlayer::AudioPlayer() : _path(Package::Current().InstalledLocation().Path() + L"\\") {
    ZeroMemory(&_sfxFormat, sizeof(_sfxFormat));
    ZeroMemory(&_musicFormat, sizeof(_musicFormat));

	winrt::check_hresult(XAudio2Create(_engine.put(), 0, XAUDIO2_DEFAULT_PROCESSOR));
	winrt::check_hresult(_engine->CreateMasteringVoice(&_masterVoice));

	winrt::check_hresult(
		MFStartup(MF_VERSION)
	);

    for (size_t i = 0; i < _audioData.size(); i++) {
        auto type = static_cast<AudioType>(i);
        _audioData[i] = LoadAudioData(type);
    }
};

AudioPlayer& AudioPlayer::Instance()
{
    static AudioPlayer instance;
    return instance;
}

void AudioPlayer::SetVoice(AudioType type, IXAudio2SourceVoice* voice) {
    XAUDIO2_BUFFER buffer = { 0 };

    buffer.AudioBytes = (uint32_t)_audioData[type].size();
    buffer.pAudioData = _audioData[type].data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    if (Neon_IsMusic(type)) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        winrt::check_hresult(
            voice->SetVolume(Neon_MusicVolume())
        );
    }
    else 
    {
        winrt::check_hresult(
            voice->SetVolume(Neon_SFXVolume())
        );
    }

    winrt::check_hresult(
        voice->SubmitSourceBuffer(&buffer)
    );
}

void AudioPlayer::Play(AudioType type) {
    IXAudio2SourceVoice* voice;

    if (Neon_IsMusic(type)) {
        winrt::check_hresult(_engine->CreateSourceVoice(&voice, &_musicFormat));
    }
    else {
        winrt::check_hresult(_engine->CreateSourceVoice(&voice, &_sfxFormat));
    }

    SetVoice(type, voice);
    winrt::check_hresult(
        voice->Start()
    );
}

std::vector<byte> AudioPlayer::LoadAudioData(AudioType type) {
	static const std::unordered_map<AudioType, std::wstring> audioIdToName = {
        {LASER1_AUDIO,L"laser1",               },
        {LASER2_AUDIO, L"laser2",              },
        {LASER3_AUDIO, L"laser3",              },
        {EXPLOSION_AUDIO, L"explosion",        },
        {CLICK_AUDIO, L"click",                },
        {MUSIC_AUDIO, L"neon_beat",            },
        {LOSE_HP_AUDIO, L"lose_hp",            },
        {GAME_OVER_AUDIO, L"game_over",        },
        {LEVEL_CLEARED_AUDIO, L"level_cleared" },
        {COLLECT_HP_AUDIO, L"collect_hp",      },
        {POWER_UP_AUDIO, L"power_up"           }
	};

    std::wstring filename = audioIdToName.at(type);
    if (Neon_IsMusic(type)) {
        filename += L".mp3";
    }
    else {
        filename += L".wav";
    }

    winrt::com_ptr<IMFSourceReader> reader;
    winrt::check_hresult(
        MFCreateSourceReaderFromURL(
            (_path + filename).c_str(),
            nullptr,
            reader.put()
        )
    );

    winrt::com_ptr<IMFMediaType> nativeMediaType;
    winrt::check_hresult(
        reader->GetNativeMediaType(static_cast<uint32_t>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nativeMediaType.put())
    );

    GUID subType = {};
    winrt::check_hresult(
        nativeMediaType->GetGUID(MF_MT_SUBTYPE, &subType)
    );

    if (subType != MFAudioFormat_Float && subType != MFAudioFormat_PCM) 
    {
        winrt::com_ptr<IMFMediaType> newType;
        winrt::check_hresult(
            MFCreateMediaType(newType.put())
        );

        winrt::check_hresult(
            newType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)
        );

        winrt::check_hresult(
            newType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)
        );

        winrt::check_hresult(
            reader->SetCurrentMediaType(static_cast<uint32_t>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, newType.get())
        );
    }
 
    winrt::com_ptr<IMFMediaType> outputMediaType;
    winrt::check_hresult(
        reader->GetCurrentMediaType(static_cast<uint32_t>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), outputMediaType.put())
    );

    UINT32 size = 0;
    WAVEFORMATEX* waveFormat;
    winrt::check_hresult(
        MFCreateWaveFormatExFromMFMediaType(outputMediaType.get(), &waveFormat, &size)
    );

    if (Neon_IsMusic(type)) {
        CopyMemory(&_musicFormat, waveFormat, sizeof(_musicFormat));
    }
    else {
        CopyMemory(&_sfxFormat, waveFormat, sizeof(_sfxFormat));
    }

    CoTaskMemFree(waveFormat);
    std::vector<byte> fileData;
    winrt::com_ptr<IMFSample> sample;
    DWORD flags{};

    while (true) {
        winrt::check_hresult(
            reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, sample.put())
        );

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }

        winrt::com_ptr<IMFMediaBuffer> buffer;
        winrt::check_hresult(sample->ConvertToContiguousBuffer(buffer.put()));

        byte* audioData = nullptr;
        DWORD audioDataLength{};
        winrt::check_hresult(buffer->Lock(&audioData, nullptr, &audioDataLength));

        for (size_t i = 0; i < audioDataLength; i++)
        {
            fileData.push_back(*(audioData + i));
        }

        winrt::check_hresult(buffer->Unlock());
    }

    return fileData;
}
