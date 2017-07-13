// Fill out your copyright notice in the Description page of Project Settings.

#include "BinauralAudioSource.h"
#include "BinauralEngine.h"

// Sets default values for this component's properties
UBinauralAudioSource::UBinauralAudioSource()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBinauralAudioSource::BeginPlay()
{
	Super::BeginPlay();

	_sourceID = TCHAR_TO_ANSI(*sourceID);

	UBinauralEngine::AddAudioSource(TCHAR_TO_ANSI(*audioPath), _sourceID);
	UBinauralEngine::PlayAudioSource(_sourceID);

	//Y,Z,X in Unreal equals X,Y,Z in Unity
	FVector location = GetOwner()->GetTransform().GetLocation();
	UBinauralEngine::SetAudioSourcePos(_sourceID, location.Y, location.Z, location.X);
}


// Called every frame
void UBinauralAudioSource::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UBinauralEngine::SetAudioSourceHRTF(_sourceID, hrtf);

	//Y,Z,X in Unreal equals X,Y,Z in Unity
	FVector location = GetOwner()->GetTransform().GetLocation();
	UBinauralEngine::SetAudioSourcePos(_sourceID, location.Y, location.Z, location.X);
}

void UBinauralAudioSource::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UBinauralEngine::StopAudioSource(_sourceID);
	UBinauralEngine::RemoveAudioSource(_sourceID);
}
