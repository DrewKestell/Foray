#pragma once

class AudioSource
{
private:
	WAVEFORMATEXTENSIBLE wfx;
	XAUDIO2_BUFFER buffer;

public:
	AudioSource(WAVEFORMATEXTENSIBLE wfx, XAUDIO2_BUFFER buffer);
	WAVEFORMATEXTENSIBLE& GetWaveFormat();
	XAUDIO2_BUFFER& GetBuffer();
};