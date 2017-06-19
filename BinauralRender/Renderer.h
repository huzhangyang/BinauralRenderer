#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-06-19
* Description: Renderer for production of binaural audio.
*******************************************************************/

#include <vector>
#include "fftw3.h"

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
	void SetSourcePos(float x, float y, float z);//set the position of sound source
	void SetTargetPos(float x, float y, float z);//set the position of listener
	void SetTargetOri(float angle); //set the orientation of listener (x,y,z)?
	vector<double> Render(vector<double> input, vector<double> hrir);
private:
};