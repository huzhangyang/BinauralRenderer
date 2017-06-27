#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-05-31
* Description: A finite-impulse-resonse convolution engine.
*******************************************************************/

#include <cassert>
#include <vector>
#include "fftw3.h"

using namespace std;

class Convolver
{
public:
	static Convolver* Instance();

	vector<double> Convolve(vector<double> signal, vector<double> filter);
	void SetSegmentLength(int segmentLength);
	void Release();
private:
	Convolver() {};
	static Convolver* instance;

	int segmentLength;
	double* buffer;
	double* output;
	double* tappedSignal;
	double* result;
	fftw_plan plan_f;
	fftw_plan plan_i;
};
