
/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-07-01
* Description: Used to export needed functions to be called in c# (e.g. Unity) or other c++ module (e.g. Unreal Engine).
*******************************************************************/

#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"

extern "C"
{
	/*
	Add an audio source object. Corresponding to an sound in FMOD. Max of 512 channels are supported in theory.
	filename: The path of audio file. Can be any mainstream format.
	sourceID: An custom ID used to uniquely identify the source object.
	If the sourceID already exists, the sound will be replaced by this new file.
	*/
	__declspec(dllexport) void AddAudioSource(const char* filename, const char* sourceID)
	{
		AudioIO::Instance()->AddAudioSource(filename, sourceID);
	}

	/*
	Remove an audio source.
	sourceID: An custom ID used to uniquely identify the source object.
	*/
	__declspec(dllexport) void RemoveAudioSource(const char* sourceID)
	{
		AudioIO::Instance()->RemoveAudioSource(sourceID);
	}

	/*
	Play an audio source.
	sourceID: An custom ID used to uniquely identify the source object.
	*/
	__declspec(dllexport) void PlayAudioSource(const char* sourceID)
	{
		AudioIO::Instance()->PlayAudioSource(sourceID);
	}

	/*
	Stop an audio source. Once stopped, it can only be played from start.
	sourceID: An custom ID used to uniquely identify the source object.
	*/
	__declspec(dllexport) void StopAudioSource(const char* sourceID)
	{
		AudioIO::Instance()->StopAudioSource(sourceID);
	}

	/*
	Set the pause or resume of an audio source.
	sourceID: An custom ID used to uniquely identify the source object.
	paused: if the audio source is to be paused.
	*/
	__declspec(dllexport) void SetAudioSourcePaused(const char* sourceID, bool paused)
	{
		AudioIO::Instance()->SetAudioSourcePaused(sourceID, paused);
	}

	/*
	Get if the audio source is currently playing.
	sourceID: An custom ID used to uniquely identify the source object.
	*/
	__declspec(dllexport) bool IsAudioSourcePlaying(const char* sourceID)
	{
		return AudioIO::Instance()->IsAudioSourcePlaying(sourceID);
	}

	/*
	Set hrtf effect enabled or not for an audio source.
	sourceID: An custom ID used to uniquely identify the source object.
	*/
	__declspec(dllexport) void SetAudioSourceHRTF(const char* sourceID, bool enable)
	{
		return AudioIO::Instance()->SetAudioSourceHRTF(sourceID, enable);
	}

	/*
	Set the position of an audio source so that it's properly used in the renderer.
	sourceID: An custom ID used to uniquely identify the source object.
	posx, posy, posz: the position in x,y,z axis of the audio source.
	*/
	__declspec(dllexport) void SetAudioSourcePos(const char* sourceID, float posx, float posy, float posz)
	{
		AudioIO::Instance()->SetAudioSourcePos(sourceID, vec3f(posx, posy, posz));
	}

	/*
	Update the audio engine. The audio playback is driven by the update function.
	Should be called with the update function in the game engine.
	*/
	__declspec(dllexport) void UpdateAudioEngine()
	{
		AudioIO::Instance()->Update();
	}

	/*
	Release the audio engine. Close everything and release allocated resources.
	Should be called upon exit of the application.
	*/
	__declspec(dllexport) void ReleaseAudioEngine()
	{
		AudioIO::Instance()->Release();
		Renderer::Instance()->Release();
	}

	/*
	Set the hrir file to be used in the renderer.
	filename: The path of original hrir file. CIPIC hrir format is supported.
	*/
	__declspec(dllexport) void SetHRIR(const char * filename)
	{
		HRIRData *hrir = DataIO::OpenMat(filename);
		Renderer::Instance()->SetHRIR(hrir);
	}

	/*
	Set the position of listener so that it's properly used in the renderer.
	posx, posy, posz: the position in x,y,z axis of the listener.
	orix, oriy, oriz: the orientation in x,y,z axis of the listener.
	*/
	__declspec(dllexport) void SetListener(float posx, float posy, float posz, float orix, float oriy, float oriz)
	{
		Renderer::Instance()->SetListener( vec3f(posx, posy, posz), vec3f(orix, oriy, oriz));
	}
}