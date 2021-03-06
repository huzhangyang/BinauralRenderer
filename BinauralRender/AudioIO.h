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
#include <map>

#include <fmod.hpp>
#include <fmod_errors.h>

#include "Renderer.h"

using namespace std;
using namespace FMOD;

/*num of samples per channel to read for each update.*/
const int DECODE_BUFFER_SIZE = 1024;

struct AudioSource
{
	Sound* sound;
	Channel* channel;
	DSP* dsp;
	vec3f pos = vec3f(0, 0, 0);
};

class AudioIO
{
public:

	/* Life Cyle.
	The class is a singleton. No explicit init is needed.
	Call update every frame in game engine.
	Call release upon quit.
	*/
	static AudioIO* Instance();
	void Update();
	void Release();

	/* Playback functions. See "export.cpp" for details.
	*/
	void AddAudioSource(const char* filename, const char* sourceID);
	void RemoveAudioSource(const char* sourceID);
	void PlayAudioSource(const char* sourceID);
	void StopAudioSource(const char* sourceID);
	void SetAudioSourcePaused(const char* sourceID, bool paused);
	bool IsAudioSourcePlaying(const char* sourceID);
	void SetAudioSourceHRTF(const char* sourceID, bool enable);
	void SetAudioSourcePos(const char* sourceID, vec3f pos);

	/* Output filtered audio to a .wav file. Experimental.
	*/
	void OutputToWAV(const char* input, const char* output, vec3f pos);
	int GetLatency();
private:
	/*Singleton modules.*/
	AudioIO() {};
	static AudioIO* instance;
	void Init();

	/*FMOD modules.*/
	System *system;
	map<string, AudioSource*> audioSources;
	FMOD_RESULT result;
	void ErrorHandle();

	/*DSPRead callback for applying HRTF.*/
	static FMOD_RESULT F_CALLBACK DSPReadCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
};