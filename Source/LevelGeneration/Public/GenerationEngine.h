// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "GenerationEngine.generated.h"


UCLASS()
class LEVELGENERATION_API AGenerationEngine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerationEngine();

	UFUNCTION(BlueprintCallable, Category="Generation")
		void SpawnNextRoom(USceneComponent* exitPosition);

	UPROPERTY(EditAnywhere)
		TSubclassOf	<AActor> Corridor;
	UPROPERTY(EditAnywhere)
		float PartSize = 1000.0f;

	UPROPERTY(EditAnywhere)
		TSubclassOf	<AActor> StartCorridor;

	UPROPERTY(EditDefaultsOnly)
		FVector StartExitLocation;

	UPROPERTY(EditDefaultsOnly)
		FRotator StartExitRotation;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf	<AActor> RoomSegment;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf	<AActor> Doorframe;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf	<AActor> ExternalWall;

	UPROPERTY(EditDefaultsOnly)
		unsigned int minRoomDim = 3;

	UPROPERTY(EditDefaultsOnly)
		unsigned int maxRoomDim = 7;

	UPROPERTY(EditDefaultsOnly)
		unsigned int forwardDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly)
		unsigned int leftDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly)
		unsigned int rightDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly)
		float segmentDeletionChance = 10.0f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnFirstCorridor();
	std::vector<AActor*> RoomSegments;
	unsigned int DoorWeightSum;

};
