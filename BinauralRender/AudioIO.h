#pragma once

#include <cstdio>
#include <cstdlib>

#include <fmod.hpp>
#include <fmod_errors.h>

using namespace FMOD;

class AudioIO
{
public:
	void Init();
	void Update();
	void Release();
	void Open(const char* filename);
	void Play();
	void TogglePause();
	bool IsPlaying();
	void Stop();
	unsigned int GetLength();
	unsigned int GetPosition();
	void SetPosition(unsigned int position);
private:
	System *system;
	Channel* channel;
	Sound* sound;
	FMOD_RESULT result;
	void ErrorHandle();

};