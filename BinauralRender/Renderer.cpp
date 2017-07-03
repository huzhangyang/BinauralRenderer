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
	segmentLength = hrtf->length;

	free(buffer);
	free(signal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);

	signal = (double*)malloc(sizeof(double) * segmentLength);
	buffer = (double*)malloc(sizeof(double) * segmentLength / 2);
	result = (double*)malloc(sizeof(double) * segmentLength);
	plan_f = fftw_plan_r2r_1d(segmentLength, signal, signal, FFTW_R2HC, FFTW_MEASURE);
	plan_i = fftw_plan_r2r_1d(segmentLength, result, result, FFTW_HC2R, FFTW_MEASURE);
	fftw_execute(plan_f);
	fftw_execute(plan_i);
}

void Renderer::Render(vector<double>& left, vector<double>& right, vec3f pos, bool useHRTF)
{
	if (hrtf == NULL || useHRTF == false)
	{//leave raw data untouched
		return;
	}

	GetAzimuthAndElevation(pos);
	vector<double> leftHRTF, rightHRTF;
	hrtf->GetHRTF(azimuth, elevation, leftHRTF, rightHRTF);
	left = Convolve(left, leftHRTF);
	right = Convolve(right, rightHRTF);
}

void Renderer::SetListener(vec3f pos, vec3f ori)
{
	listenerPos = pos;
	listenerOri = ori;
}

void Renderer::GetAzimuthAndElevation(vec3f sourcePos)
{
	//azimuth = arctan(y / x)
	float deltaX = sourcePos.x - listenerPos.x;
	float deltaY = sourcePos.y - listenerPos.y;
	float deltaZ = sourcePos.z - listenerPos.z;
	if (deltaX == 0 && deltaY == 0)
		azimuth = 0;
	else
		azimuth = atan2(deltaY, deltaX) * 180 / PI;
	//elevation = 90 - arccos(z / r)
	float r = pow(deltaX, 2.0f) + pow(deltaY, 2.0f) + pow(deltaZ, 2.0f);
	r = sqrt(r);
	if (r == 0)
		elevation = 0;
	else
		elevation = 90 - acos(deltaZ / r) * 180 / PI;
	//cast azimuth to -90~90 and elevation to -90~270
	if (azimuth > 90)
	{
		azimuth = 180 - azimuth;
		elevation = elevation < 0 ? 180 + elevation : 180 - elevation;
	}
	else if (azimuth < -90)
	{
		azimuth = -180 - azimuth;
		elevation = elevation < 0 ? 180 + elevation : 180 - elevation;
	}
	//printf("azimuth: %f, elevation: %f\n", azimuth, elevation);
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

void Renderer::Release()
{
	free(buffer);
	free(signal);
	free(filter);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
	output.clear();
	sourcePos.clear();
}
