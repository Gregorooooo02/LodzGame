// Fill out your copyright notice in the Description page of Project Settings.

#include "WaterLevelManager.h"
#include "WaterGenerator.h"
#include "Kismet/GameplayStatics.h"

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
	
	// Auto-find WaterGenerator if enabled and not assigned
	if (bUseWaterGenerator && !WaterBody)
	{
		WaterBody = FindWaterGenerator();
	}
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
			if (bFinalCorridorLevel)MaxWaterLevel = -25;
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

	// Trigger Blueprint event
	OnWaterLowering();
}

void AWaterLevelManager::StopLowering()
{
	bIsLowering = false;
	bIsRising = true;
}

void AWaterLevelManager::UpdateMaxWaterLevelForRoom(int RoomID) 
{
	FRoomWaterSettings CurrentSettings;

	if (RoomID >= 0 && RoomID < 3) {
		CurrentSettings = RoomWaterSettings.IsValidIndex(0) ? RoomWaterSettings[0] : FRoomWaterSettings();
	}
	else if (RoomID >= 3 && RoomID < 6) {
		CurrentSettings = RoomWaterSettings.IsValidIndex(1) ? RoomWaterSettings[1] : FRoomWaterSettings();
	}
	else if (RoomID >= 6) {
		CurrentSettings = RoomWaterSettings.IsValidIndex(2) ? RoomWaterSettings[2] : FRoomWaterSettings();
	}
	
	MaxWaterLevel = CurrentSettings.MaxWaterLevel;
	RisingSpeed = CurrentSettings.RisingSpeed;

	if (RoomID == -1) {
		StartLowering(WaterLevel + 25);
		bFinalCorridorLevel = true;
	}

	if (bIsRising) {
		TargetWaterLevel = MaxWaterLevel;
	}
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

AActor* AWaterLevelManager::FindWaterGenerator()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterGenerator::StaticClass(), FoundActors);
	
	if (FoundActors.Num() > 0)
	{
		return FoundActors[0];
	}
	
	return nullptr;
}

