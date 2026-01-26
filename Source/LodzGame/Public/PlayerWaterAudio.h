// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerWaterAudio.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LODZGAME_API UPlayerWaterAudio : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerWaterAudio();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Detection")
	float WaterCheckRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Detection")
	float WaterCheckHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Detection")
	TArray<FName> WaterTags = { TEXT("Water"), TEXT("WaterBody") };



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Audio")
	float ShallowWaterThreshold = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Audio")
	float MediumWaterThreshold = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Audio")
	float DeepWaterThreshold = 150.0f;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Water|State")
	bool bIsInWater = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Water|State")
	float CurrentWaterDepth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Water|State")
	float WaterSurfaceZ = 0.0f;

	UFUNCTION(BlueprintPure, Category = "Water")
	bool IsInWater() const { return bIsInWater; }

	UFUNCTION(BlueprintPure, Category = "Water")
	float GetWaterDepth() const { return CurrentWaterDepth; }

	UFUNCTION(BlueprintPure, Category = "Water")
	float GetNormalizedWaterDepth();

	UFUNCTION(BlueprintPure, Category = "Water")
	int GetWaterDepthCategory();

	UFUNCTION(BlueprintImplementableEvent, Category = "Water|Events")
	void OnEnterWater(float Depth);

	UFUNCTION(BlueprintImplementableEvent, Category = "Water|Events")
	void OnExitWater();

	UFUNCTION(BlueprintImplementableEvent, Category = "Water|Events")
	void OnWaterDepthChanged(float Depth, int DepthCategory);

protected:
	virtual void BeginPlay() override;

	void CheckWaterOverlap();
	void CalculateWaterDepth();

public:	
	bool bWasInWaterLastFrame = false;
	int LastDepthCategory = 0;
};
