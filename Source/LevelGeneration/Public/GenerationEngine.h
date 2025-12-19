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
		void SpawnNextRoomAsync(USceneComponent* exitPosition, AActor* previousCoridor,float waterLevel);

	UFUNCTION(BlueprintCallable, Category = "Generation")
		void LoadCoridor(TSubclassOf<AActor> coridor);

	UPROPERTY(EditAnywhere, Category = "General Parameters")
		float PartSize = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Start Room")
		TSubclassOf	<AActor> StartCorridor;
	UPROPERTY(EditDefaultsOnly, Category = "Start Room")
		FVector StartExitLocation;
	UPROPERTY(EditDefaultsOnly, Category = "Start Room")
		FRotator StartExitRotation;
	UPROPERTY(EditDefaultsOnly, Category = "Start Room")
		TSubclassOf	<AActor> StartingRoom;

	UPROPERTY(EditDefaultsOnly, Category = "General Parts")
		TSubclassOf	<AActor> RoomSegment;
	UPROPERTY(EditDefaultsOnly, Category = "General Parts")
		TSubclassOf	<AActor> Doorframe;
	UPROPERTY(EditDefaultsOnly, Category = "General Parts")
		TSubclassOf	<AActor> ExternalWall;
	UPROPERTY(EditAnywhere, Category = "General Parts")
		TSubclassOf<AActor> BP_Valve;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		unsigned int minRoomDim = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		unsigned int maxRoomDim = 7;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		unsigned int forwardDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		unsigned int leftDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		unsigned int rightDoorWeight = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		float segmentDeletionChance = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		float maxVoronoiOffset = 350.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		float maxBoxIslandSize = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		uint32 minBoxCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		uint32 maxBoxCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		uint32 maxBoxSpawnAttemps = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		TArray<TSubclassOf<ABaseBox>> Boxes;



	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		float maxBoxIslandStartSize = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Box Parameters")
		float boxIslandSizeIncrement = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		float valveOffsetPart = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		double spawningTimeBudget = 0.015;

	UPROPERTY(EditDefaultsOnly, Category = "General Parts")
		TArray<TSubclassOf<AActor>> OptionalRooms;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		float optionalRoomSpawnChance = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Generation Parameters")
		uint32 roomLimit = 6;

	UPROPERTY(EditAnywhere)
		uint32 currentRoomID = 0;

	UPROPERTY(EditAnywhere, Category = "General Parts")
		TSubclassOf<AActor> finalRoom;


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
	void UpdateRecastNavMeshPosition(const FVector newCenter);
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
