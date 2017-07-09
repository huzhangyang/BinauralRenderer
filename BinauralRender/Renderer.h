#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-06-19
* Description: Renderer for production of binaural audio.
*******************************************************************/

#include <vector>
#include <map>
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

	vec3() {}
};
typedef vec3<float> vec3f;

class Renderer
{
public:
	static Renderer* Instance();
	/* Life Cyle.
	The class is a singleton. No explicit init is needed.
	Call release upon quit.
	*/
	void Release();

	/*Set the HRTF to render.*/
	void SetHRTF(HRTFData* data);

	/*Main render function.*/
	void Render(vector<double>& left, vector<double>& right, vec3f pos);

	/*Set position & orientation for listener. See "export.cpp" for details. */
	void SetListener(vec3f pos, vec3f ori);
private:
	/*Singleton modules.*/
	Renderer() {};
	static Renderer* instance;

	/*Private functions.*/
	void GetAzimuthAndElevation(vec3f sourcePos);
	vector<double> Convolve(vector<double> _signal, vector<double> _filter);//direct frequency multiplication segmentLength. fftSize = 2048
	vector<double> Convolve2(vector<double> _signal, vector<double> _filter);//overlap add. fftSize = 256
	vector<double> Convolve3(vector<double> _signal, vector<double> _filter);//overlap save. fftSize = 512

	/*Used for HRTF.*/
	vec3f listenerPos;
	vec3f listenerOri;
	map<const char*, vec3f> sourcePos;
	float azimuth, elevation;
	HRTFData* hrtf;

	/*Used in convolution.*/
	int fftSize;
	double* buffer;
	double* signal;
	double* filter;
	double* result;
	vector<double> output;
	fftw_plan plan_f;
	fftw_plan plan_i;
};