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
	sound->release();
	soundPCM->release();
	system->release();
}

void AudioIO::Open(const char * filename, bool openOnly)
{
	if(openOnly)
		result = system->createStream(filename, FMOD_CREATESTREAM | FMOD_OPENONLY, 0, &sound);
	else
		result = system->createSound(filename, FMOD_INIT_NORMAL, 0, &sound);
	ErrorHandle();

	int numchannels;
	result = sound->getFormat(0, 0, &numchannels, 0);
	if (numchannels != 2)
	{
		printf("Sorry, only 2-channel stereo audio is supported for now !\n");
		sound->release();
	}
}

void AudioIO::Play()
{
	result = system->playSound(sound, 0, false, &channel);
	ErrorHandle();
}

void AudioIO::TogglePause()
{
	bool x;
	result = channel->isPlaying(&x);
	channel->setPaused(!x);
}

bool AudioIO::IsPlaying()
{
	bool x;
	result = channel->isPlaying(&x);
	return x;
}

void AudioIO::Stop()
{
	result = channel->stop();
}

unsigned int AudioIO::GetLength()
{
	unsigned int x;
	result = sound->getLength(&x, FMOD_TIMEUNIT_MS);
	return x;
}

unsigned int AudioIO::GetPosition()
{
	unsigned int x;
	result = channel->getPosition(&x, FMOD_TIMEUNIT_MS);
	return x;
}

void AudioIO::SetPosition(unsigned int position)
{
	result = channel->setPosition(position, FMOD_TIMEUNIT_MS);
	ErrorHandle();
}

void AudioIO::InitPCM()
{
	assert(sound);
	unsigned int pcmLength;
	int numchannels;
	float frequency;
	FMOD_SOUND_FORMAT format;
	result = sound->getDefaults(&frequency, 0);
	result = sound->getFormat(0, &format, &numchannels, 0);
	result = sound->getLength(&pcmLength, FMOD_TIMEUNIT_PCM);

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

	result = soundPCM->setUserData(this);//pass the class to userData so as to get from callback
	ErrorHandle();
}

void AudioIO::PlayPCM()
{
	result = system->playSound(soundPCM, 0, false, &channelPCM);
	ErrorHandle();

	bool playing = true;
	while (playing)
	{
		Update();
		channelPCM->isPlaying(&playing);
	}
}

void AudioIO::ErrorHandle()
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

void AudioIO::ReadData(unsigned int size)
{
	short* dataBlock = (short*)malloc(size);//pcm 16bits data 
	unsigned int actualReadSize;
	result = sound->readData(&dataBlock[0], size, &actualReadSize);
	if (result != FMOD_OK)
	{
		channelPCM->stop();
		return;
	}
	ErrorHandle();

	//ConvertToDoubleVector
	leftChannelData.clear();
	rightChannelData.clear();
	for (unsigned int i = 0; i < actualReadSize / sizeof(short); i++)
	{
		double value = (double)dataBlock[i] / 32768.0;
		if (value > 1) value = 1.0;
		if (value < -1) value = -1.0;
		i % 2 == 0 ? leftChannelData.push_back(value) : rightChannelData.push_back(value);
	}

	free(dataBlock);
}

FMOD_RESULT F_CALLBACK AudioIO::PCMReadCallback(FMOD_SOUND* _sound, void *data, unsigned int datalen)
{
	void *userData;
	AudioIO* context;
	FMOD_RESULT result = ((Sound*)_sound)->getUserData(&userData);
	context = (AudioIO*) userData;
	if (result != FMOD_OK || context == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}
	context->ReadData(datalen);

	Renderer::Instance()->Render(context->leftChannelData, context->rightChannelData);

	//ConvertToPCM
	short* pcm = (short*)data;
	for (unsigned int i = 0; i < context->leftChannelData.size() + context->rightChannelData.size(); i++)
	{
		short value = i % 2 == 0 ? (short)(context->leftChannelData[i / 2] * 32768) : (short)(context->rightChannelData[i / 2] * 32768);
		if (value > 32767) value = 32767;
		if (value < -32768) value = -32768;
		pcm[i] = value;
	}

	return FMOD_OK;
}