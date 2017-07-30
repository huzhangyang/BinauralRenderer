#include "DataIO.h"

#if MATLAB
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
	mxDestroyArray(matArray);
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
	mxDestroyArray(matArray);
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
	mxDestroyArray(matArray);
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
	mxDestroyArray(matArray);
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
	mxDestroyArray(matArray);
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
#endif

HRIRData * DataIO::OpenBin(const char * filename)
{
	ifstream bin(filename, ios::in | ios::binary);
	HRIRData *data = new HRIRData();

	float v;

	//dispose -90 azimuth
	for (int j = 0; j < ELEVATION_COUNT + 2; j++)
	{
		for (int k = 0; k < HRIR_LENGTH; k++)
		{
			bin.read((char*)&v, sizeof(float));
			bin.read((char*)&v, sizeof(float));
		}
	}

	for (int i = 0; i < AZIMUTH_COUNT; i++)
	{
		//dispose -90 elevation
		for (int k = 0; k < HRIR_LENGTH; k++)
		{
			bin.read((char*)&v, sizeof(float));
			bin.read((char*)&v, sizeof(float));
		}

		for (int j = 0; j < ELEVATION_COUNT; j++)
		{
			for (int k = 0; k < HRIR_LENGTH; k++)
			{
				bin.read((char*)&v, sizeof(float));
				data->hrir_l[i][j][k] = (double)v;
			}
			for (int k = 0; k < HRIR_LENGTH; k++)
			{
				bin.read((char*)&v, sizeof(float));
				data->hrir_r[i][j][k] = (double)v;
			}
		}
		//dispose 270 elevation
		for (int k = 0; k < HRIR_LENGTH; k++)
		{
			bin.read((char*)&v, sizeof(float));
			bin.read((char*)&v, sizeof(float));
		}

	}

	bin.close();
	return data;
}

HRTFData* DataIO::ConvertToHRTF(HRIRData * hrir, int fftSize)
{
	HRTFData* data = new HRTFData();

	//pre-measure
	double* inout = fftw_alloc_real(fftSize);
	fftw_plan plan = fftw_plan_r2r_1d(fftSize, inout, inout, FFTW_R2HC, FFTW_MEASURE);
	fftw_execute(plan);

	//fft to hrtf
	for (int i = 0; i < AZIMUTH_COUNT; i++)
	{
		for (int j = 0; j < ELEVATION_COUNT; j++)
		{
			//left channel data & zero padding
			for (int k = 0; k < HRIR_LENGTH; k++)
			{
				inout[k] = hrir->hrir_l[i][j][k];
			}
			for (int k = HRIR_LENGTH; k < fftSize; k++)
			{
				inout[k] = 0.0;
			}
			//left channel HRTF output
			fftw_execute(plan);
			data->hrtf_l[i][j].resize(fftSize);
			for (int k = 0; k < fftSize; k++)
			{
				data->hrtf_l[i][j][k] = inout[k];
			}
			//right channel data & zero padding
			for (int k = 0; k < HRIR_LENGTH; k++)
			{
				inout[k] = hrir->hrir_r[i][j][k];
			}
			for (int k = HRIR_LENGTH; k < fftSize; k++)
			{
				inout[k] = 0.0;
			}
			//right channel HRTF output
			fftw_execute(plan);
			data->hrtf_r[i][j].resize(fftSize);
			for (int k = 0; k < fftSize; k++)
			{
				data->hrtf_r[i][j][k] = inout[k];
			}
		}
	}

	//prepare Dirac filter
	for (int k = 0; k < fftSize; k++)
	{
		inout[k] = 0.0;
	}

	double max = 0;
	int maxIndex = 0;
	for (int k = 0; k < HRIR_LENGTH; k++)
	{
		if (abs(hrir->hrir_l[0][0][k]) > max)
		{
			max = abs(hrir->hrir_l[0][0][k]);
			maxIndex = k;
		}
	}
	inout[maxIndex + 1] = 1.0;

	fftw_execute(plan);
	data->dirac.resize(fftSize);
	for (int k = 0; k < fftSize; k++)
	{
		data->dirac[k] = inout[k];
	}

	fftw_destroy_plan(plan);
	fftw_free(inout);
	delete(hrir);

	return data;
}

HRTFData::HRTFData()
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

void HRTFData::GetHRTF(float azimuth, float elevation, float distance, float minDistance, vector<double>& left, vector<double>& right, bool interpolation)
{
	if (interpolation)
	{
		int azimuthIndex1 = 0, azimuthIndex2 = AZIMUTH_COUNT - 1, elevationIndex1 = 0, elevationIndex2 = ELEVATION_COUNT - 1;
		float azimuthRatio1 = 0, azimuthRatio2 = 0, elevationRatio1 = 0, elevationRatio2 = 0;

		if (azimuth <= azimuths[0])
		{
			azimuthIndex1 = 0;
			azimuthIndex2 = 0;
			azimuthRatio1 = 0.5f;
			azimuthRatio2 = 0.5f;
		}
		else if (azimuth >= azimuths[AZIMUTH_COUNT - 1])
		{
			azimuthIndex1 = AZIMUTH_COUNT - 1;
			azimuthIndex2 = AZIMUTH_COUNT - 1;
			azimuthRatio1 = 0.5f;
			azimuthRatio2 = 0.5f;
		}
		else
		{
			for (int i = 0; i < AZIMUTH_COUNT; i++)
			{
				if (azimuths[i] >= azimuth)
				{
					azimuthIndex1 = i == 0 ? AZIMUTH_COUNT - 1 : i - 1;
					azimuthIndex2 = i;
					break;
				}
			}
			azimuthRatio1 = abs(azimuth - azimuths[azimuthIndex1]) / abs(azimuths[azimuthIndex2] - azimuths[azimuthIndex1]);
			azimuthRatio2 = 1 - azimuthRatio1;
		}

		for (int i = 0; i < ELEVATION_COUNT; i++)
		{
			if (elevations[i] >= elevation)
			{
				elevationIndex1 = i == 0 ? ELEVATION_COUNT - 1 : i - 1;
				elevationIndex2 = i;
				break;
			}
		}

		auto e = elevation < 0 ? elevation + 360 : elevation;
		auto e1 = elevations[elevationIndex1] < 0 ? elevations[elevationIndex1] + 360 : elevations[elevationIndex1];
		auto e2 = elevations[elevationIndex2] < 0 ? elevations[elevationIndex2] + 360 : elevations[elevationIndex2];
		elevationRatio1 = abs(e - e1) / abs(e2 - e1);
		elevationRatio2 = 1 - elevationRatio1;

		int size = (int)hrtf_l[0][0].size();
		left = vector<double>(size);
		right = vector<double>(size);
		for (int i = 0; i < size; i++)
		{
			left[i] = hrtf_l[azimuthIndex1][elevationIndex1][i] * azimuthRatio1 * elevationRatio1
				+ hrtf_l[azimuthIndex1][elevationIndex2][i] * azimuthRatio1 * elevationRatio2
				+ hrtf_l[azimuthIndex2][elevationIndex1][i] * azimuthRatio2 * elevationRatio1
				+ hrtf_l[azimuthIndex2][elevationIndex2][i] * azimuthRatio2 * elevationRatio2;
			right[i] = hrtf_r[azimuthIndex1][elevationIndex1][i] * azimuthRatio1 * elevationRatio1
				+ hrtf_r[azimuthIndex1][elevationIndex2][i] * azimuthRatio1 * elevationRatio2
				+ hrtf_r[azimuthIndex2][elevationIndex1][i] * azimuthRatio2 * elevationRatio1
				+ hrtf_r[azimuthIndex2][elevationIndex2][i] * azimuthRatio2 * elevationRatio2;
		}
	}
	else
	{
		int azimuthIndex = GetAzimuthIndex(azimuth, true);
		int elevationIndex = GetElevationIndex(elevation, true);
		if (azimuthIndex >= 0 && elevationIndex >= 0)
		{
			left = hrtf_l[azimuthIndex][elevationIndex];
			right = hrtf_r[azimuthIndex][elevationIndex];
		}
	}

	if (distance < minDistance)
	{
		//interpolate with Dirac Filter
		int size = (int)left.size();
		for (int i = 0; i < size; i++)
		{
			left[i] = (distance / minDistance) * left[i] + (1 - distance / minDistance) * dirac[i];
			right[i] = (distance / minDistance) * right[i] + (1 - distance / minDistance) * dirac[i];
		}
	}
}

int HRTFData::GetAzimuthIndex(float azimuth, bool nearest)
{
	assert(azimuth >= -90 && azimuth <= 90);

	int azimuthIndex = -1;
	float minAzimuthError = 180;

	for (int i = 0; i < AZIMUTH_COUNT; i++)
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

int HRTFData::GetElevationIndex(float elevation, bool nearest)
{
	assert(elevation >= -90 && elevation <= 270);

	int elevationIndex = -1;
	float minElevationIndex = 360;

	for (int i = 0; i < ELEVATION_COUNT; i++)
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