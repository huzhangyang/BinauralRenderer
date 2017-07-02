#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <windows.h>

int main()
{	
	AudioIO::Instance()->AddAudioSource("c://test2.mp3", "test");

	HRIRData *hrir = DataIO::OpenMat("c://test.mat");
	HRTFData *hrtf = DataIO::ConvertToHRTF(hrir);
	Renderer::Instance()->SetHRTF(hrtf);

	AudioIO::Instance()->PlayAudioSource("test");
	Renderer::Instance()->SetAudioSource("test", vec3f(0, 0, 0));
	Renderer::Instance()->SetListener(vec3f(0, 0, 0), vec3f(0, 0, 0));

	while (true)
	{
		AudioIO::Instance()->Update();
		Sleep(1000 / 60);
	}

	AudioIO::Instance()->Release();
	Renderer::Instance()->Release();

	return 0;
}

