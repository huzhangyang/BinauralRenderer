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
	system->release();
}

void AudioIO::Open(const char * filename)
{
	result = system->createStream(filename, FMOD_DEFAULT | FMOD_ACCURATETIME, 0, &sound);
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

void AudioIO::ErrorHandle()
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}
