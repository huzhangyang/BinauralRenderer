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



	signal = fftw_alloc_real(fftSize);
	buffer = fftw_alloc_real(fftSize / 2);
	result = fftw_alloc_real(fftSize);
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
	hrtf->GetHRTF(azimuth, elevation, distance, (float)MIN_DISTANCE, leftHRTF, rightHRTF);

	vector<double> retLeft, retRight;
	if (method == Direct)
	{
		retLeft = Convolve(left, leftHRTF);
		retRight = Convolve(right, rightHRTF);
	}
	else if (method == OverlapAdd)
	{
		retLeft = Convolve2(left, leftHRTF);
		retRight = Convolve2(right, rightHRTF);
	}
	else if (method == OverlapSave)
	{
		retLeft = Convolve3(left, leftHRTF);
		retRight = Convolve3(right, rightHRTF);
	}
	/*
	if (lastLeftHRTF.size() > 0 && lastRightHRTF.size() > 0)
	{
		vector<double> lastLeft, lastRight;
		if (method == Direct)
		{
			lastLeft = Convolve(left, lastLeftHRTF);
			lastRight = Convolve(right, lastRightHRTF);
		}
		else if (method == OverlapAdd)
		{
			lastLeft = Convolve2(left, lastLeftHRTF);
			lastRight = Convolve2(right, lastRightHRTF);
		}
		else if (method == OverlapSave)
		{
			lastLeft = Convolve3(left, lastLeftHRTF);
			lastRight = Convolve3(right, lastRightHRTF);
		}
		//cosine shaped crossfade
		size_t size = left.size();
		for (int i = 0; i < size; i++)
		{
			double crossfadeRatio = cos(i / (double)size * PI / 2);
			retLeft[i] = crossfadeRatio * lastLeft[i] + (1 - crossfadeRatio) * retLeft[i];
			retRight[i] = crossfadeRatio * lastRight[i] + (1 - crossfadeRatio) * retRight[i];
		}
	}
	lastLeftHRTF = leftHRTF;
	lastRightHRTF = rightHRTF;*/
	float attenuation = GetDistanceAttenuation(distance);
	size_t size = left.size();
	for (int i = 0; i < size; i++)
	{
		retLeft[i] *= attenuation;
		retRight[i] *= attenuation;
	}
	left = retLeft;
	right = retRight;
}

void Renderer::SetListener(vec3f pos, vec3f ori)
{
	listenerPos = pos;
	listenerOri = ori;
}

void Renderer::GetAzimuthAndElevation(vec3f sourcePos)
{
	//azimuth = arctan(x / z)
	float deltaX = sourcePos.x - listenerPos.x;
	float deltaY = sourcePos.y - listenerPos.y;
	float deltaZ = sourcePos.z - listenerPos.z;
	if (deltaX == 0 && deltaZ == 0)
		azimuth = 0;
	else
		azimuth = atan2(deltaX, deltaZ) * 180 / PI;
	//elevation = 90 - arccos(y / r)
	distance = sourcePos.dis(listenerPos);
	if (distance == 0)
		elevation = 0;
	else
		elevation = 90 - acos(deltaY / distance) * 180 / PI;
	//calculate rotation
	if (listenerOri.x > 180)
		listenerOri.x = listenerOri.x - 360;
	if (listenerOri.y > 180)
		listenerOri.y = listenerOri.y - 360;
	azimuth -= listenerOri.y;
	elevation += listenerOri.x;
	//cast azimuth to -90~90 and elevation to -90~270
	if (azimuth > 90)
	{
		azimuth = azimuth < 270 ? 180 - azimuth : azimuth - 360;
		elevation = elevation < 0 ? 180 + elevation : 180 - elevation;
	}
	else if (azimuth < -90)
	{
		azimuth = azimuth > -270 ? -180 - azimuth : azimuth + 360;
		elevation = elevation < 0 ? 180 + elevation : 180 - elevation;
	}
	//printf("azimuth: %f, elevation: %f\n", azimuth, elevation);
}

float Renderer::GetDistanceAttenuation(float distance)
{
	if (distance <= MIN_DISTANCE)
		return 1.0f;

	if (distance > MAX_DISTANCE)
		return 0.0f;

	float attenuation = pow(10.0f, -(distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE));
	return attenuation;
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
		output[i] = result[i] / fftSize;
	}
	return output;
}

vector<double> Renderer::Convolve2(vector<double> _signal, vector<double> _filter)
{
	int signalSize = (int)_signal.size();
	int segmentNum = signalSize * 2 / fftSize;
	int fftSizeHalf = fftSize / 2;
	output.resize(signalSize);
	filter = &_filter[0];

	int index = 0;
	for (int i = 0; i < fftSizeHalf; i++)
		buffer[i] = 0;

	for (int i = 0; i < segmentNum; i++)
	{
		//zero-tap signal
		for (int j = 0; j < fftSizeHalf; j++)
			signal[j] = _signal[index++];
		for (int j = fftSizeHalf; j < fftSize; j++)
			signal[j] = 0;
		//fft signal
		fftw_execute(plan_f);
		//Multiply the FFTs		
		result[0] = signal[0] * filter[0];
		result[fftSizeHalf] = signal[fftSizeHalf] * filter[fftSizeHalf];
		for (int j = 1; j < fftSizeHalf; j++)
		{
			int k = fftSize - j;
			result[j] = signal[j] * filter[j] - signal[k] * filter[k];
			result[k] = signal[j] * filter[k] + signal[k] * filter[j];
		}
		//ifft
		fftw_execute(plan_i);
		//overlap-add
		for (int j = 0; j < fftSizeHalf; j++)
		{
			output[i * fftSizeHalf + j] = buffer[j] + result[j] / fftSize;
		}
		for (int j = fftSizeHalf; j < fftSize; j++)
		{
			buffer[j - fftSizeHalf] = result[j] / fftSize;
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

	while (index <= signalLength)
	{
		//take signal
		for (int i = 0; i < fftSize; i++)
		{
			if(index + i < HRIR_LENGTH - 1)
				signal[i] = 0;
			else if (index + i < signalLength + HRIR_LENGTH - 1)
				signal[i] = _signal[index + i - (HRIR_LENGTH - 1)];
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
	fftw_free(buffer);
	fftw_free(signal);
	fftw_free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
	output.clear();
	sourcePos.clear();

	delete(hrtf);
	instance = NULL;
}
