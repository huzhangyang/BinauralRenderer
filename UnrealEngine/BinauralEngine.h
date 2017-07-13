// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Components/ActorComponent.h"
#include "BinauralEngine.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BINAURALRENDERER_API UBinauralEngine : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBinauralEngine();

	UPROPERTY(EditAnywhere)
	FString matlabPath = "C://Program Files/MATLAB/R2016b/bin/win64/";
	UPROPERTY(EditAnywhere)
	FString hrirPath = "C://test.mat";
	//DLL functions
	static void AddAudioSource(const char* filename, const char* sourceID);
	static void RemoveAudioSource(const char* sourceID);
	static void PlayAudioSource(const char* sourceID);
	static void StopAudioSource(const char* sourceID);
	static void SetAudioSourcePaused(const char * sourceID, bool paused);
	static bool IsAudioSourcePlaying(const char* sourceID);
	static void SetAudioSourceHRTF(const char* sourceID, bool enable);
	static void SetAudioSourcePos(const char* sourceID, float posx, float posy, float posz);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called every frame
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:	
	//DLL related
	const char* dllPath = "BinauralRenderer/";
	static void LoadDll(const char* path);
	static void UnloadDll();
	//DLL functions
	static void UpdateAudioEngine();
	static void ReleaseAudioEngine();
	static void SetHRIR(const char * filename);
	static void SetListener(float posx, float posy, float posz, float orix, float oriy, float oriz);
};
