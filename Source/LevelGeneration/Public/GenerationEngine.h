// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "BaseBox.h"
#include "GenerationEngine.generated.h"




UCLASS()
class LEVELGENERATION_API AGenerationEngine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerationEngine();

	UFUNCTION(BlueprintCallable, Category="Generation")
		void SpawnNextRoom(USceneComponent* exitPosition,AActor* previousCoridor);

	UFUNCTION(BlueprintCallable, Category = "Generation")
		void SpawnNextRoomAsync(USceneComponent* exitPosition, AActor* previousCoridor);

	UFUNCTION(BlueprintCallable, Category = "Generation")
		void LoadCoridor(TSubclassOf<AActor> coridor);

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
		TSubclassOf	<AActor> StartingRoom;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> BP_Valve;

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

	UPROPERTY(EditDefaultsOnly)
		float maxVoronoiOffset = 350.0f;
	UPROPERTY(EditDefaultsOnly)
		float maxBoxIslandSize = 300.0f;

	UPROPERTY(EditDefaultsOnly)
		uint32 minBoxCount = 3;

	UPROPERTY(EditDefaultsOnly)
		uint32 maxBoxCount = 5;

	UPROPERTY(EditDefaultsOnly)
		uint32 maxBoxSpawnAttemps = 5;

	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<ABaseBox>> Boxes;

	UPROPERTY(EditDefaultsOnly)
		float maxBoxIslandStartSize = 100.0f;

	UPROPERTY(EditDefaultsOnly)
		float boxIslandSizeIncrement = 100.0f;

	UPROPERTY(EditDefaultsOnly)
		float valveOffsetPart = 0.1f;

	UPROPERTY(EditDefaultsOnly)
		double spawningTimeBudget = 0.015;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnFirstCorridor();
	void SpawnValve(const FVector &segmentLocation, const FVector& parallelOffset, const FVector& perpendicularOffset, float roomRotation);
	void GenerateBoxIslands(TArray<FVector>& SegmentLocations);
	std::vector<AActor*> RoomSegments;
	std::vector<TSubclassOf	<AActor>> Coridors;
	unsigned int DoorWeightSum;
	bool FindThirdVertex(const FVector& firstVertex, const FVector& secondVertex, float radius1, float radius2, float radius3, FVector& resultVertex1, FVector& resultVertex2);

	struct SpawnParams {
		TSubclassOf	<AActor> ActorToSpawn;
		FVector position;
		FRotator rotation;
		FActorSpawnParameters spawningParams;
	};

	TQueue<SpawnParams, EQueueMode::Mpsc> SpawnQueue;

	AActor* previousCoridorPtr = nullptr;
	bool spawningNewRoom = false;
	bool deletePreviousRoom = true;
};
