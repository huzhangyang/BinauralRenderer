#include "Renderer.h"

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::Instance()
{
	if (!instance)
	{
		instance = new Renderer();
	}
	return instance;
}

void Renderer::SetHRTF(HRTFData * data)
{
	hrtf = data;
	Convolver::Instance()->SetSegmentLength(SEGMENT_LENGTH);
}

void Renderer::SetSourcePos(float x, float y, float z)
{
	sourcePos.x = x;
	sourcePos.y = y;
	sourcePos.z = z;
}

void Renderer::SetTargetPos(float x, float y, float z)
{
	targetPos.x = x;
	targetPos.y = y;
	targetPos.z = z;
}

void Renderer::SetTargetOri(float x, float y, float z)
{
	targetOri.x = x;
	targetOri.y = y;
	targetOri.z = z;
}

vector<double> Renderer::Render(vector<double> data, int channel)
{
	auto filter = channel == 0 ? hrtf->GetLeftHRTF(90, 0) : hrtf->GetRightHRTF(90, 0);
	return Convolver::Instance()->Convolve(data, filter);
}
