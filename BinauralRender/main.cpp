#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <windows.h>

int main()
{	
	AudioIO::Instance()->Open("c://test2.mp3", true);
	AudioIO::Instance()->InitPCM();


	HRIRData *hrir = DataIO::OpenMat("c://test.mat");
	HRTFData *hrtf = DataIO::ConvertToHRTF(hrir);

	Renderer::Instance()->SetHRTF(hrtf);

	AudioIO::Instance()->PlayPCM();

	while (true)
	{
		Renderer::Instance()->Update(vec3f(-1, 0, 0), vec3f(0, 0, 0), vec3f(0, 0, 0));
		AudioIO::Instance()->Update();
		Sleep(1000 / 60);
	}

	return 0;
}

