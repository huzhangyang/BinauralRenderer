#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <windows.h>

auto id = "test";

int main()
{	
	HRIRData *hrir = DataIO::OpenMat("c://test.mat");
	Renderer::Instance()->SetHRIR(hrir);

	//AudioIO::Instance()->OutputToWAV("c://test.mp3", "test.wav");
	
	AudioIO::Instance()->AddAudioSource("c://test.mp3", id);
	AudioIO::Instance()->PlayAudioSource(id);	

	while(AudioIO::Instance()->IsAudioSourcePlaying(id))
	{
		Renderer::Instance()->SetListener(vec3f(0, 0, 0), vec3f(0, 0, 0));
		AudioIO::Instance()->SetAudioSourcePos(id, vec3f(3, 3, 3));
		AudioIO::Instance()->SetAudioSourceHRTF(id, true);

		AudioIO::Instance()->Update();
		Sleep(1000 / 60);
	}

	AudioIO::Instance()->Release();
	Renderer::Instance()->Release();

	return 0;
}

