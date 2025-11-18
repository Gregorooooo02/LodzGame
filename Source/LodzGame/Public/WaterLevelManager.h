// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterLevelManager.generated.h"

UCLASS()
class LODZGAME_API AWaterLevelManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaterLevelManager();
	virtual void Tick(float DeltaTime) override;

	// Water settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	float WaterLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	float MaxWaterLevel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	float MinWaterLevel = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	float RisingSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	float LoweringSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
	AActor* WaterBody = nullptr;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Water")
	void LowerWater(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StartLowering(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StopLowering();

protected:
	virtual void BeginPlay() override;

private:
	float TargetWaterLevel = 0.0f;
	bool bIsLowering = false;
	bool bIsRising = true;
	float StartWaterLevel = 0.0f;
	
	void UpdateWaterPosition();
};
