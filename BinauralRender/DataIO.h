#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-05-31
* Description: Load CIPIC HRIR data using MATLAB library.
*******************************************************************/

#include<cmath>
#include "mat.h"

class HRIRData
{
public:
	/* Raw MATLAB data.*/
	char* name;
	double OnL[25][50];
	double OnR[25][50];
	double ITD[25][50];
	double hrir_l[25][50][200];
	double hrir_r[25][50][200];

	HRIRData();

	/* Get HRIR data.
	@param azimuth: azimuth angle in degree. [-90, 90]
	@param elevation: elevation angle in degree. [-90, 270]
	@param nearest: if no specific hrir data found given azimuth and elevation, use hrir of nearest location.
	@return HRIR data of length 200.
	*/
	double* GetLeftHRIR(float azimuth, float elevation, bool nearest = true);
	double* GetRightHRIR(float azimuth, float elevation, bool nearest = true);

private:
	float azimuths[25];
	float elevations[50];

	/* Get corresponding index to be used in hrir_l and hrir_r.
	@param azimuth: azimuth angle in degree. [-90, 90]
	@param elevation: elevation angle in degree. [-90, 270]
	@param nearest: if no specific index found given azimuth or elevation, use index of nearest location.
	@return azimuth or elevation index. [0, 24] or [0, 49]
	*/
	int GetAzimuthIndex(float azimuth, bool nearest);
	int GetElevationIndex(float elevation, bool nearest);
};

class DataIO
{
public:

	/* Open a .mat file and extract HRIR data.
	@param filename: the path of .mat file.
	@return extracted HRIR data. NULL if failed.
	*/
	HRIRData* OpenMat(const char* filename);

private:
};
