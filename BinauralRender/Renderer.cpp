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

/*
zero pad filter to 256 taps, do an FFT
Initialize and "overlap" buffer of length 128 taps to all zeros
Initialize an input pointer to 0
Loop starts here
Take 128 samples from your input starting at input pointer. Zero pad to 256, FFT
Multiply the FFTs
Inverse FFT, add overlap buffer to the first 128 samples, output these samples. Put the second 128 samples into the overlap buffer
Advance your input pointer by 128 samples and go back to step 5. Repeat until the input is all done
*/

vector<double> Renderer::Convolve(vector<double> signal, vector<double> filter)
{
	int signalSize = (int)signal.size();
	int segmentNum = signalSize * 2 / segmentLength;
	int signalLength = segmentLength / 2;// TODO not always it (end)
	output.resize(signalSize);

	int index = 0;
	for (int i = 0; i < signalLength; i++)
		buffer[i] = 0;

	for (int i = 0; i < segmentNum; i++)
	{
		//zero-tap signal
		for (int j = 0; j < signalLength; j++)
			tappedSignal[j] = signal[index++];
		for (int j = signalLength; j < segmentLength; j++)
			tappedSignal[j] = 0;
		//fft signal
		fftw_execute(plan_f);
		//Multiply the FFTs		
		result[0] = tappedSignal[0] * filter[0];
		result[signalLength] = tappedSignal[signalLength] * filter[signalLength];
		for (int j = 1; j < segmentLength / 2; j++)
		{
			int k = segmentLength - j;
			result[j] = tappedSignal[j] * filter[j] - tappedSignal[k] * filter[k];
			result[k] = tappedSignal[j] * filter[k] + tappedSignal[k] * filter[j];
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
	tappedSignal = (double*)malloc(sizeof(double) * segmentLength);
	buffer = (double*)malloc(sizeof(double) * segmentLength / 2);
	result = (double*)malloc(sizeof(double) * segmentLength);
	plan_f = fftw_plan_r2r_1d(segmentLength, tappedSignal, tappedSignal, FFTW_R2HC, FFTW_MEASURE);
	plan_i = fftw_plan_r2r_1d(segmentLength, result, result, FFTW_HC2R, FFTW_MEASURE);
	fftw_execute(plan_f);
	fftw_execute(plan_i);
}

void Renderer::Release()
{
	free(buffer);
	free(tappedSignal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
	output.clear();
}
