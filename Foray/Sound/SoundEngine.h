#pragma once

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <vector>
#include "../Events/Observer.h"
#include "../DeviceResources.h"

class SoundEngine : public Observer
{
private:
	//locals
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

public:
	SoundEngine();
	virtual const void HandleEvent(const Event* const event);
	void Update();
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
	~SoundEngine();
};