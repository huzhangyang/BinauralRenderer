#pragma once

#include<cmath>
#include "mat.h"

class HRIRData
{
public:
	char* name;
	double OnL[25][50];
	double OnR[25][50];
	double ITD[25][50];
	double hrir_l[25][50][200];
	double hrir_r[25][50][200];

	HRIRData();
	double* GetLeftHRIR(float azimuth, float elevation, bool nearest = true);
	double* GetRightHRIR(float azimuth, float elevation, bool nearest = true);

private:
	float azimuths[25];
	float elevations[50];

	int GetAzimuthIndex(float azimuth, bool nearest);
	int GetElevationIndex(float elevation, bool nearest);
};

class DataIO
{
public:
	HRIRData* OpenMat(const char* filename);
private:
};
