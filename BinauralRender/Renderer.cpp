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

void Renderer::SetSourcePos(float x, float y, float z)
{
}

void Renderer::SetTargetPos(float x, float y, float z)
{
}

void Renderer::SetTargetOri(float angle)
{
}

vector<double> Renderer::Render(vector<double> input, vector<double> hrir)
{

	return vector<double>();
}
