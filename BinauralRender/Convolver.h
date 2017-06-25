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

	vector<double> Convolve();
	void SetSegmentLength(int segmentLength);
	void SetCurrentSignal(vector<double> signal);
	void SetCurrentFilter(vector<double> filter);
private:
	Convolver() {};
	static Convolver* instance;

	int segmentLength;
	double* buffer;
	double* output;
	vector<double> signal;
	vector<double> filter;

};
