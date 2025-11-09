// Fill out your copyright notice in the Description page of Project Settings.


#include "GenerationEngine.h"

// Sets default values
AGenerationEngine::AGenerationEngine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGenerationEngine::BeginPlay()
{
	Super::BeginPlay();
	SpawnFirstCorridor();
}

// Called every frame
void AGenerationEngine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGenerationEngine::SpawnFirstCorridor()
{
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = this;

	AActor* spawned = GetWorld()->SpawnActor<AActor>(StartCorridor, StartExitLocation, StartExitRotation, params);
}

void AGenerationEngine::SpawnNextRoom(USceneComponent* exitPosition)
{
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* spawned = GetWorld()->SpawnActor<AActor>(Corridor, exitPosition->GetComponentLocation(), exitPosition->GetComponentRotation(), params);
}