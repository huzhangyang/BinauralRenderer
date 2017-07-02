#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <windows.h>

int main()
{	
	auto id = "test";
	AudioIO::Instance()->AddAudioSource("c://test.mp3", id);

	HRIRData *hrir = DataIO::OpenMat("c://test.mat");
	HRTFData *hrtf = DataIO::ConvertToHRTF(hrir);
	Renderer::Instance()->SetHRTF(hrtf);
	Renderer::Instance()->SetAudioSource(id, vec3f(0, 0, 0));
	Renderer::Instance()->SetListener(vec3f(0, 0, 0), vec3f(0, 0, 0));

	AudioIO::Instance()->OutputToWAV(id, "test.wav");
	AudioIO::Instance()->PlayAudioSource(id);

	while (true)
	{
		AudioIO::Instance()->Update();
		Sleep(1000 / 60);
	}

	AudioIO::Instance()->Release();
	Renderer::Instance()->Release();

	return 0;
}

