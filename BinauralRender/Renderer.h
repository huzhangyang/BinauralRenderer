#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-06-19
* Description: Renderer for production of binaural audio.
*******************************************************************/

#include <vector>
#include "fftw3.h"

#include "Convolver.h"
#include "DataIO.h"

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
};
typedef vec3<float> vec3f;

class Renderer
{
public:
	static Renderer* Instance();

	void SetHRTF(HRTFData* data);
	void SetSourcePos(float x, float y, float z);//set the position of sound source
	void SetTargetPos(float x, float y, float z);//set the position of listener
	void SetTargetOri(float x, float y, float z); //set the orientation of listener
	vector<double> Render(vector<double> data, int channel);// channel: 0 for left, 1 for right
private:
	Renderer() {};
	static Renderer* instance;
	vec3f sourcePos, targetPos, targetOri;
	HRTFData* hrtf;
};