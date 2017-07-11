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

void Renderer::SetHRIR(HRIRData* data)
{
	fftSize = (int)method;
	hrtf = DataIO::ConvertToHRTF(data, fftSize);

	free(buffer);
	free(signal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);

	signal = (double*)malloc(sizeof(double) * fftSize);
	buffer = (double*)malloc(sizeof(double) * fftSize / 2);
	result = (double*)malloc(sizeof(double) * fftSize);
	plan_f = fftw_plan_r2r_1d(fftSize, signal, signal, FFTW_R2HC, FFTW_MEASURE);
	plan_i = fftw_plan_r2r_1d(fftSize, result, result, FFTW_HC2R, FFTW_MEASURE);
	fftw_execute(plan_f);
	fftw_execute(plan_i);
}

void Renderer::Render(vector<double>& left, vector<double>& right, vec3f pos)
{
	if (hrtf == NULL)
	{//leave raw data untouched
		return;
	}

	GetAzimuthAndElevation(pos);
	vector<double> leftHRTF, rightHRTF;
	hrtf->GetHRTF(azimuth, elevation, leftHRTF, rightHRTF);
	if (method == Frequency)
	{
		left = Convolve(left, leftHRTF);
		right = Convolve(right, rightHRTF);
	}
	else if (method == OverlapAdd)
	{
		left = Convolve2(left, leftHRTF);
		right = Convolve2(right, rightHRTF);
	}
	else if (method == OverlapSave)
	{
		left = Convolve3(left, leftHRTF);
		right = Convolve3(right, rightHRTF);
	}
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
	//zero-pad signal
	int signalSize = (int)_signal.size();
	for (size_t i = 0; i < signalSize; i++)
	{
		signal[i] = _signal[i];
	}
	for (size_t i = signalSize; i < fftSize; i++)
	{
		signal[i] = 0;
	}
	filter = &_filter[0];

	//fft signal
	fftw_execute(plan_f);
	//Multiply the FFTs		
	result[0] = signal[0] * filter[0];
	result[fftSize / 2] = signal[fftSize / 2] * filter[fftSize / 2];
	for (int j = 1; j < fftSize / 2; j++)
	{
		int k = fftSize - j;
		result[j] = signal[j] * filter[j] - signal[k] * filter[k];
		result[k] = signal[j] * filter[k] + signal[k] * filter[j];
	}
	//IFFT
	fftw_execute(plan_i);
	//OUTPUT
	output.resize(signalSize);
	for (int i = 0; i < output.size(); i++)
	{
		output[i] = result[HRIR_LENGTH / 2 + i] / fftSize;
	}

	return output;
}

vector<double> Renderer::Convolve2(vector<double> _signal, vector<double> _filter)
{
	int signalSize = (int)_signal.size();
	int segmentNum = signalSize * 2 / fftSize;
	int signalLength = fftSize / 2;
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
		for (int j = signalLength; j < fftSize; j++)
			signal[j] = 0;
		//fft signal
		fftw_execute(plan_f);
		//Multiply the FFTs		
		result[0] = signal[0] * filter[0];
		result[signalLength] = signal[signalLength] * filter[signalLength];
		for (int j = 1; j < signalLength; j++)
		{
			int k = fftSize - j;
			result[j] = signal[j] * filter[j] - signal[k] * filter[k];
			result[k] = signal[j] * filter[k] + signal[k] * filter[j];
		}
		//ifft
		fftw_execute(plan_i);
		//overlap-add
		for (int j = 0; j < signalLength; j++)
		{
			output[i * signalLength + j] = buffer[j] + result[j] / fftSize;
		}
		for (int j = signalLength; j < fftSize; j++)
		{
			buffer[j - signalLength] = result[j] / fftSize;
		}
	}

	return output;
}

vector<double> Renderer::Convolve3(vector<double> _signal, vector<double> _filter)
{
	int signalLength = (int)_signal.size();
	int filterLength = (int)_filter.size();
	int stepSize = fftSize + 1 - HRIR_LENGTH;
	output.resize(signalLength);
	filter = &_filter[0];

	int index = 0;

	while (index + stepSize <= signalLength)
	{
		//take signal
		for (int i = 0; i < fftSize; i++)
		{
			if (index + i < signalLength)
				signal[i] = _signal[index + i];
			else
				signal[i] = 0;
		}
		//fft signal
		fftw_execute(plan_f);
		//Multiply the FFTs		
		result[0] = signal[0] * filter[0];
		result[fftSize / 2] = signal[fftSize / 2] * filter[fftSize / 2];
		for (int j = 1; j < fftSize / 2; j++)
		{
			int k = fftSize - j;
			result[j] = signal[j] * filter[j] - signal[k] * filter[k];
			result[k] = signal[j] * filter[k] + signal[k] * filter[j];
		}
		//ifft
		fftw_execute(plan_i);
		//overlap-save
		for (int i = 0; i < stepSize; i++)
		{
			if (index + i < signalLength)
				output[index + i] = result[i + HRIR_LENGTH - 1] / fftSize;
		}			

		index += stepSize;
	}

	return output;
}

void Renderer::Release()
{
	free(buffer);
	free(signal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
	output.clear();
	sourcePos.clear();

	delete(hrtf);
	instance = NULL;
}
