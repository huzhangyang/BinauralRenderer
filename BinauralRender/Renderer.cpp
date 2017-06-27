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
	SetSegmentLength(hrtf->length);
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

void Renderer::Render(vector<double>& left, vector<double>& right)
{
	vector<double> leftHRTF, rightHRTF;
	hrtf->GetHRTF(0, 0, leftHRTF, rightHRTF);
	left = Convolve(left, leftHRTF);
	right = Convolve(right, rightHRTF);
}

vector<double> Renderer::Convolve(vector<double> _signal, vector<double> _filter)
{
	int signalSize = (int)_signal.size();
	int segmentNum = signalSize * 2 / segmentLength;
	int signalLength = segmentLength / 2;
	output.resize(signalSize);
	filter = &_filter[0];

	int index = 0;
	for (int i = 0; i < signalLength; i++)
		buffer[i] = 0;

	for (int i = 0; i < segmentNum; i++)
	{
		//zero-tap signal
		for (int j = 0; j < signalLength; j++)
			signal[j] = _signal[index++];
		for (int j = signalLength; j < segmentLength; j++)
			signal[j] = 0;
		//fft signal
		fftw_execute(plan_f);
		//Multiply the FFTs		
		result[0] = signal[0] * filter[0];
		result[signalLength] = signal[signalLength] * filter[signalLength];
		for (int j = 1; j < segmentLength / 2; j++)
		{
			int k = segmentLength - j;
			result[j] = signal[j] * filter[j] - signal[k] * filter[k];
			result[k] = signal[j] * filter[k] + signal[k] * filter[j];
		}
		//IFFT
		fftw_execute(plan_i);
		//overlap-add
		for (int j = 0; j < signalLength; j++)
		{
			output[i * signalLength + j] = buffer[j] + result[j] / segmentLength;
		}
		for (int j = signalLength; j < segmentLength; j++)
		{
			buffer[j - signalLength] = result[j] / segmentLength;
		}
	}

	return output;
}

void Renderer::SetSegmentLength(int segmentLength)
{
	this->segmentLength = segmentLength;
	Release();
	signal = (double*)malloc(sizeof(double) * segmentLength);
	buffer = (double*)malloc(sizeof(double) * segmentLength / 2);
	result = (double*)malloc(sizeof(double) * segmentLength);
	plan_f = fftw_plan_r2r_1d(segmentLength, signal, signal, FFTW_R2HC, FFTW_MEASURE);
	plan_i = fftw_plan_r2r_1d(segmentLength, result, result, FFTW_HC2R, FFTW_MEASURE);
	fftw_execute(plan_f);
	fftw_execute(plan_i);
}

void Renderer::Release()
{
	free(buffer);
	free(signal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
	output.clear();
}
