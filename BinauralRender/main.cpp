#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <iostream>

int main()
{
	AudioIO *audio = new AudioIO();
	audio->Init();
	audio->Open("c://test.mp3", true);
	audio->InitPCM();
	//audio->PlayPCM();

	DataIO *data = new DataIO();
	auto mat = data->OpenMat("c://test.mat");

	Renderer *renderer = new Renderer();
	//renderer->Render(vector<double>(), vector<double>());

	return 0;
}

