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
	//RoomSegments = std::vector<AActor*>();
	DoorWeightSum = forwardDoorWeight + leftDoorWeight + rightDoorWeight;
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
	for (AActor* segment : RoomSegments) {
		GetWorld()->DestroyActor(segment);
	}
	RoomSegments.clear();

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 dimX = (FMath::Rand() % (1 + maxRoomDim - minRoomDim)) + minRoomDim;
	int32 dimY = (FMath::Rand() % (1 + maxRoomDim - minRoomDim)) + minRoomDim;

	double rotation = exitPosition->GetComponentRotation().Yaw;

	FVector XOffset(PartSize, 0, 0);
	FVector parallelOffset = XOffset.RotateAngleAxis(rotation, FVector::UpVector);
	FVector perpendicularOffset = parallelOffset.RotateAngleAxis(90, FVector::UpVector);

	FVector startPoint = exitPosition->GetComponentLocation();

	startPoint += 0.5f * parallelOffset;

	int32 offset = dimY / 2;

	FVector currentPoint = startPoint - offset * perpendicularOffset;

	bool deletedCorners[] = {false,false,false,false };
	int deletionCounter = 0;
	
	for (int i = 0; i < dimX; ++i) {
		for (int j = 0; j < dimY; ++j) {
			if ((i == 0 || i == dimX - 1) && (j == 0 || j == dimY - 1)) {
				int32 segmentDeletionRoll = (FMath::Rand() % 100);
				if (segmentDeletionRoll > segmentDeletionChance) {
					RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
				}
				else {
					deletedCorners[deletionCounter] = true;
				}
				deletionCounter++;
			}
			else {
				RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
				
			}
		}
	}

	int32 doorCount = (FMath::Rand() % 3) + 1;
	
	//Mask Forward->left->right
	BYTE exitMask = 0;

	
	if (doorCount != 3) {
		unsigned int twoSum = forwardDoorWeight + leftDoorWeight;
		while (doorCount > 0) {
			unsigned int exitRoll = FMath::Rand() % DoorWeightSum;
			BYTE exitValue = 0;
			if (exitRoll < forwardDoorWeight) {
				exitValue = 1;
			}
			else if ((exitRoll >= forwardDoorWeight) && (exitRoll < twoSum)) {
				exitValue = 2;
			} 
			else if((exitRoll >= twoSum) && (exitRoll < DoorWeightSum)){
				exitValue = 4;
			}
			if ((exitMask & exitValue) == 0) {
				doorCount--;
				exitMask += exitValue;
			}
		}
	}
	else {
		exitMask = 7;
	}
	
	int doorIndices[3] = { -1,-1,-1 };
	
	//forward
	if ((exitMask & 1) > 0) {
		int startIndex = deletedCorners[2];
		int endIndex = deletedCorners[3] ? dimY - 2 : dimY - 1;
		int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;

		FVector exitPos = currentPoint + ((dimX - 0.5f) * parallelOffset) + doorPosIndex * perpendicularOffset;
		doorIndices[0] = doorPosIndex;
		GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0,rotation,0), params);
	}

	//left
	if ((exitMask & 2) > 0) {
		int startIndex = deletedCorners[1];
		int endIndex = deletedCorners[3] ? dimX - 2 : dimX - 1;
		int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
		FVector exitPos = currentPoint + ((dimY - 0.5f) * perpendicularOffset) + doorPosIndex * parallelOffset;
		doorIndices[1] = doorPosIndex;
		GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation - 90, 0), params);
	}

	//right
	if ((exitMask & 4) > 0) {
		int startIndex = deletedCorners[0];
		int endIndex = deletedCorners[2] ? dimX - 2 : dimX - 1;
		int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
		FVector exitPos = currentPoint - 0.5f * perpendicularOffset + doorPosIndex * parallelOffset;
		doorIndices[2] = doorPosIndex;
		GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation + 90, 0), params);
	}
	 
	//Back wall
	FVector startWallPos = currentPoint - 0.5f * parallelOffset;
	for (int i = 0; i < dimY; i++) {
		if (deletedCorners[0] && i == 0) {
			GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params);
			continue;
		}
		if (deletedCorners[1] && i == dimY - 1) { 
			GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params);
			continue; 
		}
		if (i == offset) {
			GetWorld()->SpawnActor<AActor>(Doorframe, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params);
			continue;
		}
		GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params);
	}

	//Left wall
	startWallPos += 0.5f * parallelOffset - 0.5f * perpendicularOffset;
	for (int i = 0; i < dimX; i++) {
		if (deletedCorners[0] && i == 0) {
			GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation, 0), params);
			continue;
		}
		if (deletedCorners[3] && i == dimX - 1) {
			GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + (i - 0.5f) * parallelOffset + 0.5f * perpendicularOffset, FRotator(0, rotation + 180, 0), params);
			continue;
		}
		if (doorIndices[2] != -1 && i == doorIndices[2]) {
			continue;
		}
		GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * parallelOffset, FRotator(0, rotation - 90, 0), params);
	}
	
}