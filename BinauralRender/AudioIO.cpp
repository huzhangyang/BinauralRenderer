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
	{//release every sound and its source
		void* source;
		result = kvp.second->getUserData(&source);
		ErrorHandle();
		((Sound*)source)->release();
		kvp.second->release();
	}
	system->close();
	system->release();

	audioSources.clear();
	channels.clear();
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

	result = soundPCM->setUserData(sound);//pass audio source to userData so as to get from callback
	ErrorHandle();

	if (audioSources.count(sourceID) == 1)
	{
		void* source;
		result = audioSources[sourceID]->getUserData(&source);
		ErrorHandle();
		((Sound*)source)->release();
		audioSources[sourceID]->release();
		audioSources.erase(sourceID);
	}
	audioSources.insert(pair<const char*, Sound*>(sourceID, soundPCM));
}

void AudioIO::RemoveAudioSource(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		void* source;
		result = audioSources[sourceID]->getUserData(&source);
		ErrorHandle();
		((Sound*)source)->release();
		audioSources[sourceID]->release();
		audioSources.erase(sourceID);
	}
}

void AudioIO::PlayAudioSource(const char * sourceID)
{
	if (audioSources.count(sourceID) == 1)
	{
		Channel* channel;
		channels.insert(pair<const char*, Channel*>(sourceID, channel));
		result = system->playSound(audioSources[sourceID], 0, false, &channels[sourceID]);
		ErrorHandle();
	}
}

void AudioIO::StopAudioSource(const char * sourceID)
{
	if (channels.count(sourceID) == 1)
	{
		channels[sourceID]->stop();
	}
}

void AudioIO::ToggleAudioSourcePlaying(const char * sourceID)
{
	if (channels.count(sourceID) == 1)
	{
		bool x;
		result = channels[sourceID]->isPlaying(&x);
		channels[sourceID]->setPaused(!x);
	}
}

bool AudioIO::IsAudioSourcePlaying(const char * sourceID)
{
	bool ret = false;
	if (channels.count(sourceID) == 1)
	{
		channels[sourceID]->isPlaying(&ret);
	}
	return ret;
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
	Sound* source;
	FMOD_RESULT result = ((Sound*)_sound)->getUserData(&userData);
	source = (Sound*) userData;
	if (result != FMOD_OK || source == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}
	//read data from source
	unsigned int actualReadSize;
	short* currentDataBlock = (short*)malloc(DECODE_BUFFER_SIZE * 2 * sizeof(short));
	result = source->readData(&currentDataBlock[0], datalen, &actualReadSize);
	if (result != FMOD_OK)
	{
		//channel->stop();
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
	Renderer::Instance()->Render(leftChannelData, rightChannelData);

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