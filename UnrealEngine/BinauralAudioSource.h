// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Components/ActorComponent.h"
#include "BinauralAudioSource.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BINAURALRENDERER_API UBinauralAudioSource : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBinauralAudioSource();

	/*Path of the audio file.*/
	UPROPERTY(EditAnywhere)
	FString audioPath = "C://test2.mp3";
	/*An unique id used to identify the audio source object.*/
	UPROPERTY(EditAnywhere)
	FString sourceID = "1";
	/*Specify the hrtf effect.*/
	UPROPERTY(EditAnywhere)
	bool hrtf = true;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called every frame
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:	
	char* _sourceID;
};
