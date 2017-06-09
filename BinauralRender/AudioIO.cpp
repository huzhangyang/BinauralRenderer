#include "AudioIO.h"

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
	exinfo.decodebuffersize = (int)frequency;
	exinfo.length = exinfo.defaultfrequency * exinfo.numchannels * sizeof(short) * pcmLength / 1000;
	exinfo.format = format;
	exinfo.pcmreadcallback = this->PCMReadCallback;

	result = system->createStream(0, FMOD_OPENUSER | FMOD_OPENONLY, &exinfo, &soundPCM);
	ErrorHandle();

	result = soundPCM->setUserData(this);//pass the class to userData so as to get from callback
	ErrorHandle();

	result = system->playSound(soundPCM, 0, false, &channelPCM);
	ErrorHandle();
}

void AudioIO::PlayPCM()
{
	while (true)
	{
		Update();
	}
}

void AudioIO::ErrorHandle()
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

vector<double> AudioIO::ReadData(unsigned int size)
{
	short* dataBlock = (short*)malloc(size);//pcm 16bits data 
	unsigned int actualReadSize;
	result = sound->readData(&dataBlock[0], size, &actualReadSize);
	ErrorHandle();

	//ConvertToDoubleVector
	vector<double> ret;
	for (unsigned int i = 0; i < actualReadSize / sizeof(short); i++)
	{
		double value = (double)dataBlock[i] / 32768.0;
		if (value > 1) value = 1.0;
		if (value < -1) value = -1.0;
		ret.push_back(value);
	}
	return ret;
}

FMOD_RESULT F_CALLBACK AudioIO::PCMReadCallback(FMOD_SOUND* _sound, void *data, unsigned int datalen)
{
	void *userData;
	FMOD_RESULT result = ((Sound*)_sound)->getUserData(&userData);

	if (result != FMOD_OK || userData == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}
	vector<double> blockData = ((AudioIO*)userData)->ReadData(datalen);

	short* pcm = (short*)data;
	for (unsigned int i = 0; i < datalen / sizeof(short); i++)
	{
		double value = blockData[i] * 32768;
		if (value > 32767) value = 32767;
		if (value < -32768) value = -32768;
		pcm[i] = (short)value;
	}

	for (unsigned int count = 0; count < (datalen >> 2); count++)     // >>2 = 16bit stereo (4 bytes per sample)
	{
		//*stereo16bitbuffer++ = (signed short)(Common_Sin(t1) * 32767.0f);    // left channel
		//*stereo16bitbuffer++ = (signed short)(Common_Sin(t2) * 32767.0f);    // right channel
	}

	return FMOD_OK;
}