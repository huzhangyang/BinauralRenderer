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
		as->source->release();
		as->pcm->release();
	}
	system->close();
	system->release();

	audioSources.clear();
}

void AudioIO::AddAudioSource(const char * filename, const char* sourceID)
{
	Sound *sound, *soundPCM;
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

	FMOD_CREATESOUNDEXINFO exinfo = {};
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels = numchannels;
	exinfo.defaultfrequency = (int)frequency;
	exinfo.decodebuffersize = DECODE_BUFFER_SIZE;
	exinfo.length = exinfo.defaultfrequency * exinfo.numchannels * sizeof(short) * pcmLength / 1000;
	exinfo.format = format;
	exinfo.pcmreadcallback = this->PCMReadCallback;

	result = system->createStream(0, FMOD_OPENUSER | FMOD_OPENONLY, &exinfo, &soundPCM);
	ErrorHandle();

	AudioSource* as = new AudioSource();
	as->source = sound;
	as->pcm = soundPCM;
	as->sourceID = sourceID;

	result = soundPCM->setUserData(as);//pass audio source to userData so as to get from callback
	ErrorHandle();

	if (audioSources.count(sourceID) == 1)
	{
		AudioSource* _as = audioSources[sourceID];
		_as->source->release();
		_as->pcm->release();
		audioSources.erase(sourceID);
	}
	audioSources.insert(pair<const char*, AudioSource*>(sourceID, as));
}

void AudioIO::RemoveAudioSource(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		AudioSource* _as = audioSources[sourceID];
		_as->source->release();
		_as->pcm->release();
		audioSources.erase(sourceID);
	}
}

void AudioIO::PlayAudioSource(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		result = system->playSound(audioSources[sourceID]->pcm, 0, false, &audioSources[sourceID]->channel);
		ErrorHandle();
	}
}

void AudioIO::StopAudioSource(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		audioSources[sourceID]->channel->stop();
	}
}

void AudioIO::ToggleAudioSourcePlaying(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		bool x;
		result = audioSources[sourceID]->channel->isPlaying(&x);
		audioSources[sourceID]->channel->setPaused(!x);
	}
}

bool AudioIO::IsAudioSourcePlaying(const char * sourceID)
{
	bool ret = false;
	if (audioSources.count(sourceID) == 1)
	{
		audioSources[sourceID]->channel->isPlaying(&ret);
	}
	return ret;
}

void AudioIO::SetAudioSourceHRTF(const char * sourceID, bool enable)
{
	if (audioSources.count(sourceID) == 1)
	{
		audioSources[sourceID]->hrtfEnabled = enable;
	}
}

void AudioIO::OutputToWAV(const char * sourceID, const char * output)
{
	System *systemNRT;
	result = FMOD::System_Create(&systemNRT);
	ErrorHandle();
	result = systemNRT->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);
	ErrorHandle();
	result = systemNRT->init(1, FMOD_INIT_STREAM_FROM_UPDATE, (void*)output);
	ErrorHandle();

	printf("Outputing to %s, it might take a few minutes. Please wait.\n", output);
	systemNRT->playSound(audioSources[sourceID]->pcm, 0, false, &audioSources[sourceID]->channel);
	while (IsAudioSourcePlaying(sourceID))
	{
		systemNRT->update();
	}

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

FMOD_RESULT F_CALLBACK AudioIO::PCMReadCallback(FMOD_SOUND* _sound, void *data, unsigned int datalen)
{
	void *userData;
	AudioSource* as;
	FMOD_RESULT result = ((Sound*)_sound)->getUserData(&userData);
	as = (AudioSource*) userData;
	if (result != FMOD_OK || as == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}
	//read data from source
	unsigned int actualReadSize;
	short* currentDataBlock = (short*)malloc(DECODE_BUFFER_SIZE * 2 * sizeof(short));
	result = as->source->readData(&currentDataBlock[0], datalen, &actualReadSize);
	if (result != FMOD_OK)
	{
		as->channel->stop();
		return result;
	}

	//ConvertToDoubleVector
	unsigned int actualNumSamples = actualReadSize / sizeof(short) / 2;
	auto leftChannelData = vector<double>(actualNumSamples);
	auto rightChannelData = vector<double>(actualNumSamples);
	unsigned int totalSize = actualNumSamples * 2;

	for (size_t i = 0; i < totalSize; i++)
	{
		double value = (double)currentDataBlock[i] / 32768.0;
		if (value > 1) value = 1.0;
		if (value < -1) value = -1.0;
		i % 2 == 0 ? leftChannelData[i / 2] = value : rightChannelData[i / 2] = value;
	}

	//Rendering
	Renderer::Instance()->Render(leftChannelData, rightChannelData, as->sourceID, as->hrtfEnabled);

	//ConvertToPCM
	short* pcm = (short*)data;
	for (size_t i = 0; i < totalSize; i++)
	{
		short value = i % 2 == 0 ? (short)(leftChannelData[i / 2] * 32768) : (short)(rightChannelData[i / 2] * 32768);
		if (value > 32767) value = 32767;
		if (value < -32768) value = -32768;
		pcm[i] = value;
	}

	leftChannelData.clear();
	rightChannelData.clear();
	free(currentDataBlock);

	return FMOD_OK;
}