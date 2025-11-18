// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomControl.h"

// Sets default values
ARoomControl::ARoomControl()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

} 

// Called when the game starts or when spawned
void ARoomControl::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoomControl::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoomControl::TriggerNextRoomSpawn(USceneComponent* exitPosition, AActor* caller)
{
	AGenerationEngine* engine = Cast<AGenerationEngine>(Owner);
	if (engine != nullptr) {
		engine->SpawnNextRoom(exitPosition,caller);
	}
}