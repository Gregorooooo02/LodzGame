// Fill out your copyright notice in the Description page of Project Settings.

#include "WaterGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AWaterGenerator::AWaterGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AWaterGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	if (!PlayerReference)
	{
		PlayerReference = FindPlayer();
	}
	
	if (PlayerReference)
	{
		CurrentPlayerGrid = WorldToGrid(PlayerReference->GetActorLocation());
		GenerateInitialGrid();
	}
}

void AWaterGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerReference)
	{
		PlayerReference = FindPlayer();
		if (!PlayerReference)
			return;
	}

	FIntPoint NewPlayerGrid = WorldToGrid(PlayerReference->GetActorLocation());
	
	if (NewPlayerGrid != CurrentPlayerGrid)
	{
		CurrentPlayerGrid = NewPlayerGrid;
		UpdateGrid();
	}
}

void AWaterGenerator::GenerateInitialGrid()
{
	for (int32 X = -GridRadius; X <= GridRadius; X++)
	{
		for (int32 Y = -GridRadius; Y <= GridRadius; Y++)
		{
			FIntPoint GridPos = CurrentPlayerGrid + FIntPoint(X, Y);
			SpawnTile(GridPos);
		}
	}
}

void AWaterGenerator::UpdateGrid()
{
	// Remove tiles that are too far
	TArray<FIntPoint> TilesToRemove;
	for (auto& Pair : ActiveTiles)
	{
		if (!ShouldTileExist(Pair.Key))
		{
			TilesToRemove.Add(Pair.Key);
		}
	}
	
	for (FIntPoint GridPos : TilesToRemove)
	{
		RemoveTile(GridPos);
	}
	
	// Spawn new tiles in range
	for (int32 X = -GridRadius; X <= GridRadius; X++)
	{
		for (int32 Y = -GridRadius; Y <= GridRadius; Y++)
		{
			FIntPoint GridPos = CurrentPlayerGrid + FIntPoint(X, Y);
			if (!ActiveTiles.Contains(GridPos))
			{
				SpawnTile(GridPos);
			}
		}
	}
}

FIntPoint AWaterGenerator::WorldToGrid(FVector WorldLocation)
{
	FVector LocalLocation = WorldLocation - GetActorLocation();
	
	int32 GridX = FMath::FloorToInt(LocalLocation.X / TileSize);
	int32 GridY = FMath::FloorToInt(LocalLocation.Y / TileSize);
	
	return FIntPoint(GridX, GridY);
}

FVector AWaterGenerator::GridToWorld(FIntPoint GridPos)
{
	float WorldX = (GridPos.X * TileSize) + (TileSize * 0.5f);
	float WorldY = (GridPos.Y * TileSize) + (TileSize * 0.5f);
	
	return GetActorLocation() + FVector(WorldX, WorldY, 0.0f);
}

void AWaterGenerator::SpawnTile(FIntPoint GridPos)
{
	if (!WaterPlaneBP || ActiveTiles.Contains(GridPos))
		return;

	FVector SpawnLocation = GridToWorld(GridPos);
	FRotator SpawnRotation = GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AActor* NewTile = GetWorld()->SpawnActor<AActor>(WaterPlaneBP, SpawnLocation, SpawnRotation, SpawnParams);
	
	if (NewTile)
	{
		// Attach to generator for easy transform updates
		NewTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		
		FWaterTileInfo TileInfo(NewTile, GridPos);
		ActiveTiles.Add(GridPos, TileInfo);
	}
}

void AWaterGenerator::RemoveTile(FIntPoint GridPos)
{
	if (FWaterTileInfo* TileInfo = ActiveTiles.Find(GridPos))
	{
		if (TileInfo->TileActor && IsValid(TileInfo->TileActor))
		{
			TileInfo->TileActor->Destroy();
		}
		ActiveTiles.Remove(GridPos);
	}
}

bool AWaterGenerator::ShouldTileExist(FIntPoint GridPos)
{
	int32 DeltaX = FMath::Abs(GridPos.X - CurrentPlayerGrid.X);
	int32 DeltaY = FMath::Abs(GridPos.Y - CurrentPlayerGrid.Y);
	
	return DeltaX <= GridRadius && DeltaY <= GridRadius;
}

AActor* AWaterGenerator::FindPlayer()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	return PlayerCharacter;
}

