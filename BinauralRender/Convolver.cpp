#include "Convolver.h"

Convolver* Convolver::instance = nullptr;

Convolver* Convolver::Instance()
{
	if (!instance)
	{
		instance = new Convolver();
	}
	return instance;
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

vector<double> Convolver::Convolve(vector<double> signal, vector<double> filter)
{
	vector<double> output;
	int segmentNum = (int)signal.size() * 2 / segmentLength;
	int signalLength = segmentLength / 2;// TODO not always it (end)

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
			output.push_back(buffer[j] + result[j] / segmentLength);
		}
		for (int j = signalLength; j < segmentLength; j++)
		{
			buffer[j - signalLength] = result[j] / segmentLength;
		}
	}

	return output;
}

void Convolver::SetSegmentLength(int segmentLength)
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

void Convolver::Release()
{
	free(buffer);
	free(output);
	free(tappedSignal);
	free(result);
	fftw_destroy_plan(plan_f);
	fftw_destroy_plan(plan_i);
}
