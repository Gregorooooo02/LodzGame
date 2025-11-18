// Fill out your copyright notice in the Description page of Project Settings.

#include "WaterLevelManager.h"

// Sets default values
AWaterLevelManager::AWaterLevelManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWaterLevelManager::BeginPlay()
{
	Super::BeginPlay();
	
	TargetWaterLevel = MaxWaterLevel;
	StartWaterLevel = WaterLevel;
}

// Called every frame
void AWaterLevelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLowering)
	{
		// Smooth interpolation when lowering
		WaterLevel = FMath::FInterpTo(WaterLevel, TargetWaterLevel, DeltaTime, LoweringSpeed);
		
		// Stop lowering when close enough to target
		if (FMath::IsNearlyEqual(WaterLevel, TargetWaterLevel, 0.5f))
		{
			WaterLevel = TargetWaterLevel;
			bIsLowering = false;
			bIsRising = true; // Resume rising after lowering is done
		}
	}
	else if (bIsRising)
	{
		// Gradually rise back to max level
		if (WaterLevel < MaxWaterLevel)
		{
			WaterLevel = FMath::FInterpTo(WaterLevel, MaxWaterLevel, DeltaTime, RisingSpeed);
			
			if (FMath::IsNearlyEqual(WaterLevel, MaxWaterLevel, 0.1f))
			{
				WaterLevel = MaxWaterLevel;
			}
		}
	}

	// Clamp water level to min/max bounds
	WaterLevel = FMath::Clamp(WaterLevel, MinWaterLevel, MaxWaterLevel);
	
	// Update water body position
	UpdateWaterPosition();
}

void AWaterLevelManager::LowerWater(float Amount)
{
	StartLowering(Amount);
}

void AWaterLevelManager::StartLowering(float Amount)
{
	StartWaterLevel = WaterLevel;
	TargetWaterLevel = WaterLevel - Amount;
	TargetWaterLevel = FMath::Clamp(TargetWaterLevel, MinWaterLevel, MaxWaterLevel);
	
	bIsLowering = true;
	bIsRising = false;
}

void AWaterLevelManager::StopLowering()
{
	bIsLowering = false;
	bIsRising = true;
}

void AWaterLevelManager::UpdateWaterPosition()
{
	if (WaterBody && IsValid(WaterBody))
	{
		FVector CurrentLocation = WaterBody->GetActorLocation();
		CurrentLocation.Z = WaterLevel;
		WaterBody->SetActorLocation(CurrentLocation);
	}
}

