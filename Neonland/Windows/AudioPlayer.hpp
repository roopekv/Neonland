#pragma once
#include "../Neonland.h"

class AudioPlayer final {
public:
	static AudioPlayer& Instance();
	void Play(AudioType type);
private:
	AudioPlayer();
	winrt::com_ptr<IXAudio2> _engine;
	IXAudio2MasteringVoice* _masterVoice;
	WAVEFORMATEX _sfxFormat;
	WAVEFORMATEX _musicFormat;
	winrt::hstring _path;

	std::array<std::vector<byte>, AudioTypeCount> _audioData;

	std::vector<byte> LoadAudioData(AudioType type);

	void SetVoice(AudioType type, IXAudio2SourceVoice* voice);
};
