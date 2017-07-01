
/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-07-01
* Description: Used to export needed functions to be called in c#(e.g. Unity) or other c++ module (e.g. Unreal Engine).
*******************************************************************/

#include "AudioIO.h"
#include "DataIO.h"
#include "Renderer.h"

extern "C"
{
	__declspec(dllexport) void OpenAudio(const char * filename, bool openOnly)
	{
		AudioIO::Instance()->Open(filename, openOnly);
		AudioIO::Instance()->InitPCM();
	}

	__declspec(dllexport) void OpenHRIR(const char * filename)
	{
		HRIRData *hrir = DataIO::OpenMat(filename);
		HRTFData *hrtf = DataIO::ConvertToHRTF(hrir);
		Renderer::Instance()->SetHRTF(hrtf);
	}

	__declspec(dllexport) void StartPlay()
	{
		AudioIO::Instance()->PlayPCM();
	}

	__declspec(dllexport) void UpdateAudio()
	{
		AudioIO::Instance()->Update();
	}

	__declspec(dllexport) void SetListener(float posx, float posy, float posz, float orix, float oriy, float oriz)
	{
		Renderer::Instance()->Update(vec3f(0, 0, 0), vec3f(posx, posy, posz), vec3f(orix, oriy, oriz));
	}
}