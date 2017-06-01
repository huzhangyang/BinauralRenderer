#include "AudioIO.h"
#include <iostream>

int main()
{
	AudioIO *audio = new AudioIO();
	audio->Init();
	audio->Open("c://test.mp3");
	audio->Play();
	unsigned int length = audio->GetLength();
	audio->SetPosition(length / 3);
	std::cin >> length;
	return 0;
}