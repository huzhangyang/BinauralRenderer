#include "DataIO.h"

HRIRData* DataIO::OpenMat(const char* filename)
{
	MATFile *mat = matOpen(filename, "r");

	if (mat == NULL)
	{
		printf("Error opening file %s\n", filename);
		return NULL;
	}

	mxArray *matArray;
	HRIRData *data = new HRIRData();
	size_t M, N;

	matArray = matGetVariable(mat, "name");
	if (matArray != NULL)
	{
		data->name = (char *)mxCalloc(mxGetN(matArray) + 1, sizeof(char));
		mxGetString(matArray, data->name, mxGetN(matArray) + 1);
		printf("Load data of %s\n", data->name);
	}

	matArray = matGetVariable(mat, "OnL");
	if (matArray != NULL)
	{
		auto matData = (double*)mxGetData(matArray);
		M = mxGetM(matArray);//25
		N = mxGetN(matArray);//50
		for (int i = 0; i < M; i++)
			for (int j = 0; j < N; j++)
				data->OnL[i][j] = matData[M*j + i];
	}
	matArray = matGetVariable(mat, "OnR");
	if (matArray != NULL)
	{
		auto matData = (double*)mxGetData(matArray);
		M = mxGetM(matArray);//25
		N = mxGetN(matArray);//50
		for (int i = 0; i < M; i++)
			for (int j = 0; j < N; j++)
				data->OnR[i][j] = matData[M*j + i];
	}
	matArray = matGetVariable(mat, "ITD");
	if (matArray != NULL)
	{
		auto matData = (double*)mxGetData(matArray);
		M = mxGetM(matArray);//25
		N = mxGetN(matArray);//50
		for (int i = 0; i < M; i++)
			for (int j = 0; j < N; j++)
				data->ITD[i][j] = matData[M*j + i];
	}
	matArray = matGetVariable(mat, "hrir_l");
	if (matArray != NULL)
	{
		auto matData = (double*)mxGetData(matArray);
		const mwSize *dimension = mxGetDimensions(matArray);//25*50*200
		for (int i = 0; i < dimension[0]; i++)
			for (int j = 0; j < dimension[1]; j++)
				for (int k = 0; k < dimension[2]; k++)
					data->hrir_l[i][j][k] = matData[dimension[0] * (j + dimension[1] * k) + i];
	}
	matArray = matGetVariable(mat, "hrir_r");
	if (matArray != NULL)
	{
		auto matData = (double*)mxGetData(matArray);
		const mwSize *dimension = mxGetDimensions(matArray);//25*50*200
		for (int i = 0; i < dimension[0]; i++)
			for (int j = 0; j < dimension[1]; j++)
				for (int k = 0; k < dimension[2]; k++)
					data->hrir_r[i][j][k] = matData[dimension[0] * (j + dimension[1] * k) + i];
	}

	mxDestroyArray(matArray);
	matClose(mat);

	return data;
}

HRIRData::HRIRData()
{// assemble azimuth and elevation values according to CIPIC document.
	azimuths[0] = -80;
	azimuths[1] = -65;
	azimuths[2] = -55;
	for (int i = 3; i <= 21; i++)
		azimuths[i] = -60 + 5.0f * i;
	azimuths[22] = 55;
	azimuths[23] = 65;
	azimuths[24] = 80;

	for (int i = 0; i < 50; i++)
		elevations[i] = -45 + 5.625f * i;
}

double* HRIRData::GetLeftHRIR(float azimuth, float elevation, bool nearest)
{
	int azimuthIndex = GetAzimuthIndex(azimuth, nearest);
	int elevationIndex = GetElevationIndex(elevation, nearest);
	if (azimuthIndex >= 0 && elevationIndex >= 0)
	{
		return hrir_l[azimuthIndex][elevationIndex];
	}
	else
	{
		printf("GetLeftHRIR Failed at %f %f.\n", azimuth, elevation);
		return NULL;
	}
}

double* HRIRData::GetRightHRIR(float azimuth, float elevation, bool nearest)
{
	int azimuthIndex = GetAzimuthIndex(azimuth, nearest);
	int elevationIndex = GetElevationIndex(elevation, nearest);
	if (azimuthIndex >= 0 && elevationIndex >= 0)
	{
		return hrir_r[azimuthIndex][elevationIndex];
	}
	else
	{
		printf("GetRightHRIR Failed at %f %f.\n", azimuth, elevation);
		return NULL;
	}
}

int HRIRData::GetAzimuthIndex(float azimuth, bool nearest)
{
	assert(azimuth >= -90 && azimuth <= 90);

	int azimuthIndex = -1;
	float minAzimuthError = 180;

	for (int i = 0; i < 25; i++)
	{
		float azimuthError = abs(azimuth - azimuths[i]);
		if (azimuthError < minAzimuthError)
		{
			minAzimuthError = azimuthError;
			azimuthIndex = i;
		}
	}

	if (!nearest && minAzimuthError > 1e-6)
		return -1;

	return azimuthIndex;
}

int HRIRData::GetElevationIndex(float elevation, bool nearest)
{
	assert(elevation >= -90 && elevation <= 270);

	int elevationIndex = -1;
	float minElevationIndex = 360;

	for (int i = 0; i < 25; i++)
	{
		float elevationError = abs(elevation - elevations[i]);
		if (elevationError < minElevationIndex)
		{
			minElevationIndex = elevationError;
			elevationIndex = i;
		}
	}

	if (!nearest && minElevationIndex > 1e-6)
		return -1;

	return elevationIndex;
}
