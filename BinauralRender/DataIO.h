#pragma once

/*******************************************************************
* Author: Zhangyang Hu
* Date: 2017-05-31
* Description: Load CIPIC HRIR data using MATLAB library.
*******************************************************************/

#define MATLAB 0

#include <cassert>
#include <cmath>
#include <fstream>
#include <vector>
#include "fftw3.h"

#if MATLAB
#include "mat.h"
#endif


const int AZIMUTH_COUNT = 25;
const int ELEVATION_COUNT = 50;
const int HRIR_LENGTH = 200;

using namespace std;

class HRIRData
{
public:
	/* Raw MATLAB data.*/
	char* name;
	double OnL[AZIMUTH_COUNT][ELEVATION_COUNT];
	double OnR[AZIMUTH_COUNT][ELEVATION_COUNT];
	double ITD[AZIMUTH_COUNT][ELEVATION_COUNT];
	double hrir_l[AZIMUTH_COUNT][ELEVATION_COUNT][HRIR_LENGTH];
	double hrir_r[AZIMUTH_COUNT][ELEVATION_COUNT][HRIR_LENGTH];
};

class HRTFData
{
public:
	vector<double> hrtf_l[AZIMUTH_COUNT][ELEVATION_COUNT];
	vector<double> hrtf_r[AZIMUTH_COUNT][ELEVATION_COUNT];
	vector<double> dirac;

	HRTFData();

	/* Get HRTF data.
	@param azimuth: azimuth angle in degree. [-90, 90]
	@param elevation: elevation angle in degree. [-90, 270]
	@param distance: distance between audio source and listener. used for dirac interpolation on near field.
	@param minDistance: distance starting interpolation. used for dirac interpolation on near field.
	@param interpolation: if yes interpolate hrir using triangulation method, otherwise use hrtf of nearest location.
	@return HRTF data in pointer left & right.
	*/
	void GetHRTF(float azimuth, float elevation, float distance, float minDistance, vector<double>& left, vector<double>& right ,bool interpolation = true);
private:
	float azimuths[AZIMUTH_COUNT];
	float elevations[ELEVATION_COUNT];

	/* Get corresponding index to be used in hrtf_l and hrtf_r.
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
#if MATLAB
	/* Open a MATLAB .mat file and extract HRIR data.
	@param filename: the path of .mat file.
	@return extracted HRIR data. NULL if failed.
	*/
	static HRIRData* OpenMat(const char* filename);
#endif
	/* Open a binary .bin file and extract HRIR data.
	@param filename: the path of .bin file.
	@return extracted HRIR data. NULL if failed.
	*/
	static HRIRData* OpenBin(const char* filename);

	/* Open a wave .wav file and extract HRIR data.
	@param filename: the path of .wav file.
	@return extracted HRIR data. NULL if failed.
	*/
	static HRIRData* OpenWav(const char* filename);

	/* Convert HRIRs to HRTFs using fourier transform.
	@param hrir: the path of .mat file.
	@return converted HRTF data. NULL if failed.
	*/
	static HRTFData* ConvertToHRTF(HRIRData* hrir, int fftSize);
};
