#include "../stdafx.h"
#include "AudioSource.h"

AudioSource::AudioSource(WAVEFORMATEXTENSIBLE wfx, XAUDIO2_BUFFER buffer)
	: wfx{ wfx },
	  buffer{ buffer }
{
}

WAVEFORMATEXTENSIBLE& AudioSource::GetWaveFormat()
{
	return wfx;
}

XAUDIO2_BUFFER& AudioSource::GetBuffer()
{
	return buffer;
}
