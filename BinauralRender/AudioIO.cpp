#include "AudioIO.h"

AudioIO* AudioIO::instance = nullptr;

AudioIO* AudioIO::Instance()
{
	if (!instance)
	{
		instance = new AudioIO();
		instance->Init();
	}
	return instance;
}

void AudioIO::Init()
{
	result = System_Create(&system);
	ErrorHandle();

	result = system->init(512, FMOD_INIT_NORMAL, 0);
	ErrorHandle();	
}

void AudioIO::Update()
{
	system->update();
}

void AudioIO::Release()
{
	for (const auto& kvp : audioSources)
	{//release every AudioSource
		AudioSource* as = kvp.second;
		as->sound->release();
		as->dsp->release();
	}
	system->close();
	system->release();

	audioSources.clear();
	instance = NULL;
}

void AudioIO::AddAudioSource(const char * filename, const char* sourceID)
{
	Sound *sound;
	result = system->createStream(filename, FMOD_CREATESTREAM | FMOD_OPENONLY, 0, &sound);
	ErrorHandle();

	//create PCM sound
	unsigned int pcmLength;
	int numchannels;
	float frequency;
	FMOD_SOUND_FORMAT format;
	result = sound->getDefaults(&frequency, 0);
	result = sound->getFormat(0, &format, &numchannels, 0);
	result = sound->getLength(&pcmLength, FMOD_TIMEUNIT_PCM);

	if (numchannels != 2)
	{
		printf("Sorry, only 2-channel stereo audio is supported for now !\n");
		sound->release();
		return;
	}

	if ((int)frequency != 44100)
	{
		printf("Sorry, only 44100Hz sample rate is supported for now !\n");
		sound->release();
		return;
	}

	FMOD_DSP_DESCRIPTION dspdesc = {};
	strcpy_s(dspdesc.name, "HRTF unit");
	dspdesc.version = 1;
	dspdesc.numinputbuffers = 1;
	dspdesc.numoutputbuffers = 1;
	dspdesc.read = DSPReadCallback;

	DSP *dsp;
	result = system->createDSP(&dspdesc, &dsp);
	ErrorHandle();	

	AudioSource* as = new AudioSource();
	as->sound = sound;
	as->dsp = dsp;
	dsp->setUserData(as);//pass audiosource to userdata so as to be accessed from the callback

	RemoveAudioSource(sourceID);//if duplicate
	audioSources.insert(pair<const char*, AudioSource*>(sourceID, as));
}

void AudioIO::RemoveAudioSource(const char * sourceID)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->sound->release();
		audioSources[sourceID]->dsp->release();
		audioSources.erase(sourceID);
	}
}

void AudioIO::PlayAudioSource(const char * sourceID)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		result = system->playSound(audioSources[sourceID]->sound, 0, false, &audioSources[sourceID]->channel);
		ErrorHandle();
		result = audioSources[sourceID]->channel->addDSP(0, audioSources[sourceID]->dsp);
		ErrorHandle();
	}
}

void AudioIO::StopAudioSource(const char * sourceID)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->channel->stop();
	}
}

void AudioIO::SetAudioSourcePaused(const char* sourceID, bool paused)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->channel->setPaused(paused);
	}
}

bool AudioIO::IsAudioSourcePlaying(const char * sourceID)
{
	bool ret = false;
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->channel->isPlaying(&ret);
	}
	return ret;
}

void AudioIO::SetAudioSourceHRTF(const char * sourceID, bool enable)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->dsp->setBypass(!enable);
	}
}

void AudioIO::SetAudioSourcePos(const char * sourceID, vec3f pos)
{
	if (audioSources.find(sourceID) != audioSources.end())
	{
		audioSources[sourceID]->pos = pos;
	}
}

void AudioIO::OutputToWAV(const char * input, const char * output, vec3f pos)
{
	System *systemNRT;
	result = FMOD::System_Create(&systemNRT);
	ErrorHandle();
	result = systemNRT->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);
	ErrorHandle();
	result = systemNRT->init(1, FMOD_INIT_STREAM_FROM_UPDATE, (void*)output);
	ErrorHandle();

	Sound *sound;
	result = systemNRT->createStream(input, FMOD_CREATESTREAM | FMOD_OPENONLY, 0, &sound);
	ErrorHandle();

	FMOD_DSP_DESCRIPTION dspdesc = {};
	strcpy_s(dspdesc.name, "HRTF unit");
	dspdesc.version = 1;
	dspdesc.numinputbuffers = 1;
	dspdesc.numoutputbuffers = 1;
	dspdesc.read = DSPReadCallback;

	DSP *dsp;
	result = systemNRT->createDSP(&dspdesc, &dsp);
	ErrorHandle();

	AudioSource* as = new AudioSource();
	as->sound = sound;
	as->dsp = dsp;
	as->pos = pos;
	dsp->setUserData(as);//pass audiosource to userdata so as to be accessed from the callback

	printf("Outputing to %s, it might take a few minutes. Please wait.\n", output);

	result = systemNRT->playSound(as->sound, 0, false, &as->channel);
	result = as->channel->addDSP(0, as->dsp);

	bool playing = true;
	unsigned int process;
	while (playing)
	{
		as->channel->isPlaying(&playing);
		as->channel->getPosition(&process, FMOD_TIMEUNIT_PCM);
		printf("%d\n", process);
		systemNRT->update();
	}

	sound->release();
	dsp->release();
	systemNRT->close();
	systemNRT->release();
	printf("Outputing Complete.");
}

void AudioIO::ErrorHandle()
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

FMOD_RESULT F_CALLBACK AudioIO::DSPReadCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
	assert(inchannels == 2 && *outchannels == 2);

	void * data;
	((DSP*)dsp_state->instance)->getUserData(&data);
	AudioSource *as = (AudioSource*)data;
	assert(as);

	auto leftChannelData = vector<double>(length);
	auto rightChannelData = vector<double>(length);

	for (unsigned int samp = 0; samp < length; samp++)
	{
		leftChannelData[samp] = (double)inbuffer[samp * inchannels + 0];
		rightChannelData[samp] = (double)inbuffer[samp * inchannels + 1];
	}

	Renderer::Instance()->Render(leftChannelData, rightChannelData, as->pos);

	for (unsigned int samp = 0; samp < length; samp++)
	{
		outbuffer[(samp * inchannels) + 0] = (float)leftChannelData[samp];
		outbuffer[(samp * inchannels) + 1] = (float)rightChannelData[samp];
	}

	leftChannelData.clear();
	rightChannelData.clear();

	return FMOD_OK;
}