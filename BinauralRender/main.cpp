#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"
#include <iostream>

int main()
{	
	AudioIO::Instance()->Open("c://test.mp3", true);
	AudioIO::Instance()->InitPCM();

	HRIRData *hrir = DataIO::OpenMat("c://test.mat");
	HRTFData *hrtf = DataIO::ConvertToHRTF(hrir);

	return 0;
}

