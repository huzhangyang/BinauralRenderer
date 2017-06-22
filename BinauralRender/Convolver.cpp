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

vector<double> Convolver::Convolve()
{
	vector<double> output;
	int filterLength = (int)filter.size();
	int signalLength = (int)signal.size();
	int bufferLength = filterLength + signalLength - 1;
	assert(filterLength > 0 && signalLength > 0);

	//fft signal
	fftw_complex* out1 = (fftw_complex *)fftw_malloc(sizeof(double) * signalLength * 2);
	fftw_plan plan1 = fftw_plan_dft_r2c_1d(signalLength, &signal[0], out1, FFTW_ESTIMATE);
	fftw_execute(plan1);
	//padding
	vector<double> signal_f;
	for (int i = 0; i < bufferLength; i++)
	{
		signal_f.push_back(out1[i][0]);
	}
	for (int i = 0; i < filterLength - 1; i++)
	{
		signal_f.push_back(0.0);
	}
	for (int i = 0; i < signalLength - 1; i++)
	{
		filter.push_back(0.0);
	}
	//multiply and ifft
	fftw_complex* in2 = (fftw_complex *)fftw_malloc(sizeof(double) * bufferLength * 2);
	double* out2 = (double*)malloc(sizeof(double) * bufferLength);
	for (int i = 0; i < bufferLength; i++)
	{
		in2[i][0] = signal_f[i] * filter[i];
		in2[i][1] = 0.0;
	}
	fftw_plan plan2 = fftw_plan_dft_c2r_1d(bufferLength, in2, out2, FFTW_ESTIMATE);
	fftw_execute(plan2);

	//result
	for (int i = 0; i < bufferLength; i++)
	{
		output.push_back(out2[i]);
	}

	fftw_destroy_plan(plan1);
	fftw_free(out1);

	fftw_destroy_plan(plan2);
	fftw_free(in2);
	free(out2);

	return output;
}

void Convolver::SetCurrentSignal(vector<double> signal)
{
	this->signal = signal;
}

void Convolver::SetCurrentFilter(vector<double> filter)
{
	this->filter = filter;
}
