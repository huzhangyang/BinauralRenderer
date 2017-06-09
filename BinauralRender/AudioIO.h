#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-05-31
* Description: Audio input / output class. Basically a wrapper of FMOD.
*******************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>

#include <fmod.hpp>
#include <fmod_errors.h>

using namespace std;
using namespace FMOD;

class AudioIO
{
public:
	/* Life Cyle.
	Call init to initialize the class.
	Call update every frame in game engine.
	Call release upon quit.
	*/
	void Init();
	void Update();
	void Release();

	/*
	Basic Playback funtions. Pretty much self-explainatory.
	OpenOnly: Don't buffer the audio so that readData is to be used.
	The length and position unit is millisecond.
	*/
	void Open(const char* filename, bool openOnly = false);
	void Play();
	void TogglePause();
	bool IsPlaying();
	void Stop();
	unsigned int GetLength();
	unsigned int GetPosition();
	void SetPosition(unsigned int position);

	/*
	PCM related.
	*/
	void InitPCM();
	void PlayPCM();
private:
	System *system;
	Channel* channel;
	Sound* sound;
	FMOD_RESULT result;
	void ErrorHandle();

	/*
	PCM related.
	*/
	Channel* channelPCM;
	Sound* soundPCM;

	vector<double> ReadData(unsigned int length);
	static FMOD_RESULT F_CALLBACK PCMReadCallback(FMOD_SOUND* _sound, void *data, unsigned int datalen);
};