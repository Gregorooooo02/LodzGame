// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterGenerator.generated.h"

USTRUCT(BlueprintType)
struct FWaterTileInfo
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* TileActor = nullptr;

	FIntPoint GridPosition;
	
	FWaterTileInfo() : GridPosition(0, 0) {}
	FWaterTileInfo(AActor* Actor, FIntPoint Pos) : TileActor(Actor), GridPosition(Pos) {}
};

UCLASS()
class LODZGAME_API AWaterGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaterGenerator();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
	TSubclassOf<AActor> WaterPlaneBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
	float TileSize = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
	int32 GridRadius = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
	float UpdateCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
	AActor* PlayerReference = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TMap<FIntPoint, FWaterTileInfo> ActiveTiles;
	FIntPoint CurrentPlayerGrid;
	
	void GenerateInitialGrid();
	void UpdateGrid();
	FIntPoint WorldToGrid(FVector WorldLocation);
	FVector GridToWorld(FIntPoint GridPos);
	void SpawnTile(FIntPoint GridPos);
	void RemoveTile(FIntPoint GridPos);
	bool ShouldTileExist(FIntPoint GridPos);
	AActor* FindPlayer();
};
