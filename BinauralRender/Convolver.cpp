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

vector<double> Convolver::Convolve()
{
	vector<double> output;
	int signalLength = (int)signal.size();
	int segmentNum = signalLength / segmentLength * 2;

	int index = 0;
	double* result = (double*)malloc(sizeof(double) * segmentLength);
	fftw_plan plan = fftw_plan_r2r_1d(segmentLength, buffer, buffer, FFTW_R2HC, FFTW_MEASURE);//TODO FFTW_PATIENT ?
	fftw_plan plan_i = fftw_plan_r2r_1d(segmentLength, result, result, FFTW_HC2R, FFTW_MEASURE);//TODO FFTW_PATIENT ?
	fftw_execute(plan);
	fftw_execute(plan_i);

	for (int i = 0; i < segmentNum; i++)
	{
		for (int j = 0; j < segmentLength / 2; j++)
			buffer[j] = signal[index++];
		for (int j = segmentLength / 2; j < segmentLength; j++)
			buffer[j] = 0;
		//fft signal
		fftw_execute(plan);
		//Multiply the FFTs
		
		result[0] = buffer[0] * filter[0];
		result[segmentLength - 1] = buffer[segmentLength - 1] * filter[segmentLength - 1];
		for (int j = 1; j < segmentLength / 2; j++)
		{
			int k = segmentLength - j;
			result[j] = buffer[j] * filter[j] - buffer[k] * filter[k];
			result[k] = buffer[j] * filter[k] + buffer[k] * filter[j];
		}
		//IFFT
		fftw_execute(plan_i);
		//overlap-add
		for (int j = 0; j < segmentLength / 2; j++)
		{
			output.push_back(signal[i * segmentLength / 2 + j] + result[j] / segmentLength);
		}
	}

	fftw_destroy_plan(plan);
	fftw_destroy_plan(plan_i);
	free(result);

	return output;
}

void Convolver::SetSegmentLength(int segmentLength)
{
	this->segmentLength = segmentLength;
	buffer = (double*)malloc(sizeof(double) * segmentLength);
}

void Convolver::SetCurrentSignal(vector<double> signal)
{
	this->signal = signal;
}

void Convolver::SetCurrentFilter(vector<double> filter)
{
	this->filter = filter;
}
