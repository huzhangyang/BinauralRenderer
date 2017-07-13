// Fill out your copyright notice in the Description page of Project Settings.

#include "BinauralEngine.h"

typedef void(*_AddAudioSource)(const char* filename, const char* sourceID);
typedef void(*_RemoveAudioSource)(const char* sourceID);
typedef void(*_PlayAudioSource)(const char* sourceID);
typedef void(*_StopAudioSource)(const char* sourceID);
typedef void(*_SetAudioSourcePaused)(const char* sourceID, bool paused);
typedef bool(*_IsAudioSourcePlaying)(const char* sourceID);
typedef void(*_SetAudioSourceHRTF)(const char* sourceID, bool enable);
typedef void(*_SetAudioSourcePos)(const char* sourceID, float posx, float posy, float posz);
typedef void(*_UpdateAudioEngine)();
typedef void(*_ReleaseAudioEngine)();
typedef void(*_SetHRIR)(const char * filename);
typedef void(*_SetListener)(float posx, float posy, float posz, float orix, float oriy, float oriz);

void* dllHandle;
_AddAudioSource addAudioSource;
_RemoveAudioSource removeAudioSource;
_PlayAudioSource playAudioSource;
_StopAudioSource stopAudioSource;
_SetAudioSourcePaused setAudioSourcePaused;
_IsAudioSourcePlaying isAudioSourcePlaying;
_SetAudioSourceHRTF setAudioSourceHRTF;
_SetAudioSourcePos setAudioSourcePos;
_UpdateAudioEngine updateAudioEngine;
_ReleaseAudioEngine releaseAudioEngine;
_SetHRIR setHRIR;
_SetListener setListener;

// Sets default values for this component's properties
UBinauralEngine::UBinauralEngine()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBinauralEngine::BeginPlay()
{
	Super::BeginPlay();

	FPlatformProcess::AddDllDirectory(*matlabPath);
	FString path = FPaths::GamePluginsDir() + dllPath;
	FPlatformProcess::AddDllDirectory(*path);

	LoadDll(dllPath);

	SetHRIR(TCHAR_TO_ANSI(*hrirPath));
}

// Called every frame
void UBinauralEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector location = GetOwner()->GetTransform().GetLocation();
	FVector rotation = GetOwner()->GetTransform().GetRotation().Euler();

	//Y,Z,X in Unreal equals X,Y,Z in Unity
	SetListener(location.Y, location.Z, location.X, rotation.Y, rotation.Z, rotation.X);

	UpdateAudioEngine();
}

void UBinauralEngine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ReleaseAudioEngine();
}

void UBinauralEngine::LoadDll(const char* path)
{
	FString procName = FPaths::GamePluginsDir() + path + "BinauralRender.dll";
	dllHandle = FPlatformProcess::GetDllHandle(*procName); // Retrieve the DLL.
	
	if (dllHandle != NULL)
	{
		procName = "AddAudioSource";
		addAudioSource = (_AddAudioSource)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "RemoveAudioSource";
		removeAudioSource = (_RemoveAudioSource)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "PlayAudioSource";
		playAudioSource = (_PlayAudioSource)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "StopAudioSource";
		stopAudioSource = (_StopAudioSource)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "SetAudioSourcePaused";
		setAudioSourcePaused = (_SetAudioSourcePaused)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "IsAudioSourcePlaying";
		isAudioSourcePlaying = (_IsAudioSourcePlaying)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "SetAudioSourceHRTF";
		setAudioSourceHRTF = (_SetAudioSourceHRTF)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "SetAudioSourcePos";
		setAudioSourcePos = (_SetAudioSourcePos)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "UpdateAudioEngine";
		updateAudioEngine = (_UpdateAudioEngine)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "ReleaseAudioEngine";
		releaseAudioEngine = (_ReleaseAudioEngine)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "SetHRIR";
		setHRIR = (_SetHRIR)FPlatformProcess::GetDllExport(dllHandle, *procName);
		procName = "SetListener";
		setListener = (_SetListener)FPlatformProcess::GetDllExport(dllHandle, *procName);
	}
}

void UBinauralEngine::UnloadDll()
{
	ReleaseAudioEngine();
	if (dllHandle != NULL)
	{
		addAudioSource = NULL;
		removeAudioSource = NULL;
		playAudioSource = NULL;
		stopAudioSource = NULL;
		setAudioSourcePaused = NULL;
		isAudioSourcePlaying = NULL;
		setAudioSourceHRTF = NULL;
		setAudioSourcePos = NULL;
		updateAudioEngine = NULL;
		releaseAudioEngine = NULL;
		setHRIR = NULL;
		setListener = NULL;

		FPlatformProcess::FreeDllHandle(dllHandle);
		dllHandle = NULL;
	}
}

void UBinauralEngine::AddAudioSource(const char * filename, const char * sourceID)
{
	if (addAudioSource != NULL)
	{
		addAudioSource(filename, sourceID);
	}
}

void UBinauralEngine::RemoveAudioSource(const char * sourceID)
{
	if (removeAudioSource != NULL)
	{
		removeAudioSource(sourceID);
	}
}

void UBinauralEngine::PlayAudioSource(const char * sourceID)
{
	if (playAudioSource != NULL)
	{
		playAudioSource(sourceID);
	}
}

void UBinauralEngine::StopAudioSource(const char * sourceID)
{
	if (stopAudioSource != NULL)
	{
		stopAudioSource(sourceID);
	}
}

void UBinauralEngine::SetAudioSourcePaused(const char * sourceID, bool paused)
{
	if (setAudioSourcePaused != NULL)
	{
		setAudioSourcePaused(sourceID, paused);
	}
}

bool UBinauralEngine::IsAudioSourcePlaying(const char * sourceID)
{
	if (isAudioSourcePlaying != NULL)
	{
		return isAudioSourcePlaying(sourceID);
	}
	return false;
}

void UBinauralEngine::SetAudioSourceHRTF(const char* sourceID, bool enable)
{
	if (setAudioSourceHRTF != NULL)
	{
		setAudioSourceHRTF(sourceID, enable);
	}
}

void UBinauralEngine::SetAudioSourcePos(const char* sourceID, float posx, float posy, float posz)
{
	if (setAudioSourcePos != NULL)
	{
		setAudioSourcePos(sourceID, posx, posy, posz);
	}
}

void UBinauralEngine::UpdateAudioEngine()
{
	if (updateAudioEngine != NULL)
	{
		updateAudioEngine();
	}
}

void UBinauralEngine::ReleaseAudioEngine()
{
	if (releaseAudioEngine != NULL)
	{
		releaseAudioEngine();
	}
}

void UBinauralEngine::SetHRIR(const char * filename)
{
	if (setHRIR != NULL)
	{
		setHRIR(filename);
	}
}

void UBinauralEngine::SetListener(float posx, float posy, float posz, float orix, float oriy, float oriz)
{
	if (setListener != NULL)
	{
		setListener(posx, posy, posz, orix, oriy, oriz);
	}
}