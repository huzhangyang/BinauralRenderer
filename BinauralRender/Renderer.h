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

/* Template for a simple vec3 container.
The engine uses a left handed coordinate system by default(+X = right, +Y = up, +Z = forwards).
*/
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

	T dis(vec3 t2)
	{
		T deltaX = this->x - t2.x;
		T deltaY = this->y - t2.y;
		T deltaZ = this->z - t2.z;
		T rr = pow(deltaX, 2.0f) + pow(deltaY, 2.0f) + pow(deltaZ, 2.0f);
		return sqrt(rr);
	}
};
typedef vec3<float> vec3f;

enum ConvolutionMethod{Direct = 1024, OverlapAdd = 256, OverlapSave = 512};

class Renderer
{
public:
	static Renderer* Instance();
	/* Life Cyle.
	The class is a singleton. No explicit init is needed.
	Call release upon quit.
	*/
	void Release();

	/*Set the HRIR to render.*/
	void SetHRIR(HRIRData* data);

	/*Main render function.*/
	void Render(vector<double>& left, vector<double>& right, vec3f pos);

	/*Set position & orientation for listener. See "export.cpp" for details. */
	void SetListener(vec3f pos, vec3f ori);
private:
	/*Singleton modules.*/
	Renderer() { method = OverlapAdd; };
	static Renderer* instance;

	/*Private functions.*/
	void GetAzimuthAndElevation(vec3f sourcePos);
	float GetDistanceAttenuation(float distance, float minDistance = 1, float maxDistance = 50);//unit: meter
	vector<double> Convolve(vector<double> _signal, vector<double> _filter);//direct frequency multiplication. fftSize = 2048
	vector<double> Convolve2(vector<double> _signal, vector<double> _filter);//overlap add. fftSize = 256
	vector<double> Convolve3(vector<double> _signal, vector<double> _filter);//overlap save. fftSize = 512

	/*Used for HRTF.*/
	vec3f listenerPos;
	vec3f listenerOri;
	map<const char*, vec3f> sourcePos;
	float azimuth, elevation, distance;
	HRTFData* hrtf;

	/*Used in convolution.*/
	ConvolutionMethod method;
	int fftSize;
	double* buffer;
	double* signal;
	double* filter;
	double* result;
	vector<double> output;
	fftw_plan plan_f;
	fftw_plan plan_i;
	//vector<double> lastLeftHRTF, lastRightHRTF;
};