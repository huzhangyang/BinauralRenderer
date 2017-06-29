#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-06-19
* Description: Renderer for production of binaural audio.
*******************************************************************/

#include <vector>
#include <cmath>
#include "fftw3.h"

#include "DataIO.h"

const float PI = 3.14159265358979f;

using namespace std;

// Template for a simple vec3 container
template <typename T>
struct vec3
{
	union
	{
		struct
		{
			T x, y, z;
		};
		T coords[3];
	};

	vec3(T x, T y, T z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};
typedef vec3<float> vec3f;

class Renderer
{
public:
	static Renderer* Instance();

	void SetHRTF(HRTFData* data);
	void Update(vec3f sourcePos, vec3f targetPos, vec3f targetOri);
	void Render(vector<double>& left, vector<double>& right);
private:
	Renderer() {};
	static Renderer* instance;

	float azimuth, elevation;
	HRTFData* hrtf;

	/*Convolution*/
	int segmentLength;
	double* buffer;
	double* signal;
	double* filter;
	double* result;
	vector<double> output;
	fftw_plan plan_f;
	fftw_plan plan_i;
	vector<double> Convolve(vector<double> signal, vector<double> filter);

	void Release();
};