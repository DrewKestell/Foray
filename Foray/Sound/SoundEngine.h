#pragma once

#include "AudioSource.h"
#include "../Events/Observer.h"

class SoundEngine : public Observer
{
private:
	//locals
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice; // for some reason IXAudio2MasteringVoice does not work with ComPtr
	std::unordered_map<unsigned int, AudioSource> audioSources;

public:
	SoundEngine();
	void Initialize();
	virtual const void HandleEvent(const Event* const event);
	void Update();
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
	HRESULT CreateAudioSource(const unsigned int audioId, const wchar_t* fileName);
	void PlayAudio(const unsigned int audioId, const float volume);
	~SoundEngine();
};