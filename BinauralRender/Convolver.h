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
	vector<double> Convolve();
	void SetCurrentSignal(vector<double> signal);
	void SetCurrentFilter(vector<double> filter);
private:
	vector<double> buffer;
	vector<double> signal;
	vector<double> filter;

};
