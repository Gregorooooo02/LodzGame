// Fill out your copyright notice in the Description page of Project Settings.

#include "GenerationEngine.h"
#include <UMG.h>
#include <NavigationSystem.h>
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/NavMeshBoundsVolume.h"


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

	RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(StartCorridor, StartExitLocation, StartExitRotation, params));
	RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(StartingRoom, FVector::ZeroVector, FRotator::ZeroRotator, params));
}

void AGenerationEngine::SpawnNextRoom(USceneComponent* exitPosition, AActor* previousCoridor)
{
	for (AActor* segment : RoomSegments) {
		if (segment == previousCoridor) continue;
		GetWorld()->DestroyActor(segment);
	}
	RoomSegments.clear();
	RoomSegments.push_back(previousCoridor);

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FActorSpawnParameters Coridorparams;
	Coridorparams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Coridorparams.Owner = this;

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
	
	TArray<FVector> SitesPoints;
	

	for (int i = 0; i < dimX; ++i) {
		for (int j = 0; j < dimY; ++j) {
			if ((i == 0 || i == dimX - 1) && (j == 0 || j == dimY - 1)) {
				int32 segmentDeletionRoll = (FMath::Rand() % 100);
				if (segmentDeletionRoll > segmentDeletionChance) {
					RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
					int indexX = (i == 0 ? -0.5f : i + 0.5f);
					int indexY = (j == 0 ? -0.5f : j + 0.5f);
					SitesPoints.Add(currentPoint + (i * parallelOffset) + (j * perpendicularOffset));
				}
				else {
					deletedCorners[deletionCounter] = true;
				}
				deletionCounter++;
			}
			else {
				RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
				SitesPoints.Add(currentPoint + (i * parallelOffset) + (j * perpendicularOffset));
			}
		}
	}
	
	FVector randomSectorLocation = SitesPoints[FMath::Rand() % SitesPoints.Num()];
	
	SpawnValve(randomSectorLocation, parallelOffset, perpendicularOffset, rotation);
	
	GenerateBoxIslands(SitesPoints);
	
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
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0,rotation,0), params));
		int coridorIndex = FMath::Rand() % Coridors.size();
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation, 0), Coridorparams));
	}

	//left
	if ((exitMask & 2) > 0) {
		int startIndex = deletedCorners[1];
		int endIndex = deletedCorners[3] ? dimX - 2 : dimX - 1;
		int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
		FVector exitPos = currentPoint + ((dimY - 0.5f) * perpendicularOffset) + doorPosIndex * parallelOffset;
		doorIndices[1] = doorPosIndex;
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation + 90, 0), params));
		int coridorIndex = FMath::Rand() % Coridors.size();
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation + 90, 0), Coridorparams));
	}

	//right
	if ((exitMask & 4) > 0) {
		int startIndex = deletedCorners[0];
		int endIndex = deletedCorners[2] ? dimX - 2 : dimX - 1;
		int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
		FVector exitPos = currentPoint - 0.5f * perpendicularOffset + doorPosIndex * parallelOffset;
		doorIndices[2] = doorPosIndex;
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation - 90, 0), params));
		int coridorIndex = FMath::Rand() % Coridors.size();
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation - 90, 0), Coridorparams));
	}
	 
	//Back wall
	FVector startWallPos = currentPoint - 0.5f * parallelOffset;
	for (int i = 0; i < dimY; i++) {
		if (deletedCorners[0] && i == 0) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params));
			continue;
		}
		if (deletedCorners[1] && i == dimY - 1) { 
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params));
			continue; 
		}
		if (i == offset) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params));
			continue;
		}
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params));
	}

	//Left wall
	startWallPos += 0.5f * parallelOffset - 0.5f * perpendicularOffset;
	for (int i = 0; i < dimX; i++) {
		if (deletedCorners[0] && i == 0) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation, 0), params));
			continue;
		}
		if (deletedCorners[2] && i == dimX - 1) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
				(ExternalWall, startWallPos + (i - 0.5f) * parallelOffset + 0.5f * perpendicularOffset, FRotator(0, rotation + 180, 0), params));
			continue;
		}
		if (doorIndices[2] != -1 && i == doorIndices[2]) {
			continue;
		}
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * parallelOffset, FRotator(0, rotation - 90, 0), params));
	}

	//Forward wall
	startWallPos += (dimX - 0.5f) * parallelOffset + 0.5f * perpendicularOffset;
	for (int i = 0; i < dimY; i++) {
		if (deletedCorners[2] && i == 0) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params));
			continue;
		}
		if (deletedCorners[3] && i == dimY - 1) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
				(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params));
			continue;
		}
		if (doorIndices[0] != -1 && i == doorIndices[0]) {
			continue;
		}
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation + 180, 0), params));
	}
	
	//Right wall
	startWallPos += -0.5f * parallelOffset + (dimY - 0.5f) * perpendicularOffset;
	for (int i = 0; i < dimX; i++) {
		if (deletedCorners[3] && i == 0) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
				(ExternalWall, startWallPos - 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation+180, 0), params));
			continue;
		}
		if (deletedCorners[1] && i == dimX - 1) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
				(ExternalWall, startWallPos - (i - 0.5f) * parallelOffset - 0.5f * perpendicularOffset, FRotator(0, rotation, 0), params));
			continue;
		}
		if (doorIndices[1] != -1 && i == (dimX - (doorIndices[1] + 1))) {
			continue;
		}
		RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos - i * parallelOffset, FRotator(0, rotation + 90, 0), params));
	}

	//Update NavMesh position
	FVector roomCenter = currentPoint + ((dimX - 1) * 0.5f * parallelOffset) + ((dimY - 1) * 0.5f * perpendicularOffset);
	UpdateRecastNavMeshPosition(roomCenter);

	// TEMPORARY: Spawn valve in room center
	// FVector roomCenter = currentPoint + ((dimX - 1) * 0.5f * parallelOffset) + ((dimY - 1) * 0.5f * perpendicularOffset);
	//SpawnValveInRoomCenter(roomCenter, rotation);
}

void AGenerationEngine::SpawnValve(const FVector& segmentLocation, const FVector& parallelOffset, const FVector& perpendicularOffset, float roomRotation)
{
	if (!BP_Valve)
		return;

	//TODO: Make valves not spawn in outer walls

	int xyRand = FMath::Rand() % 8;

	FVector valveFinalOffset;
	float finalRotation;

	switch (xyRand)
	{
	case 0: {
		valveFinalOffset = 0.5f * perpendicularOffset + (0.5f - valveOffsetPart) * parallelOffset;
		finalRotation = roomRotation - 180.0f;
		break;
	}
	case 1: {
		valveFinalOffset = -0.5f * perpendicularOffset + (0.5f - valveOffsetPart) * parallelOffset;
		finalRotation = roomRotation - 180.0f;
		break;
	}
	case 2: {
		valveFinalOffset = 0.5f * perpendicularOffset + (-0.5f + valveOffsetPart) * parallelOffset;
		finalRotation = roomRotation;
		break;
	}
	case 3: {
		valveFinalOffset = -0.5f * perpendicularOffset + (-0.5f + valveOffsetPart) * parallelOffset;
		finalRotation = roomRotation;
		break;
	}
	case 4: {
		valveFinalOffset = 0.5f * parallelOffset + (0.5f - valveOffsetPart) * perpendicularOffset;
		finalRotation = roomRotation - 90.0f;
		break;
	}
	case 5: {
		valveFinalOffset = -0.5f * parallelOffset + (0.5f - valveOffsetPart) * perpendicularOffset;
		finalRotation = roomRotation - 90.0f;
		break;
	}
	case 6: {
		valveFinalOffset = 0.5f * parallelOffset + (-0.5f + valveOffsetPart) * perpendicularOffset;
		finalRotation = roomRotation + 90.0f;
		break;
	}
	case 7: {
		valveFinalOffset = -0.5f * parallelOffset + (-0.5f + valveOffsetPart) * perpendicularOffset;
		finalRotation = roomRotation + 90.0f;
		break;
	}
	default:
		break;
	}

	valveFinalOffset = segmentLocation + valveFinalOffset + FVector(0, 0, 70);

	FActorSpawnParameters valveParams;
	valveParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	valveParams.Owner = this;

	AActor* ValveActor = GetWorld()->SpawnActor<AActor>(BP_Valve, valveFinalOffset, FRotator(0, finalRotation, 0), valveParams);

	if (ValveActor)
	{
		RoomSegments.push_back(ValveActor);
	}
}
/*
void AGenerationEngine::GenerateBoxIslands(TArray<FVector>& SegmentLocations, FBox& BoundingVolume)
{
	FActorSpawnParameters BoxParams;
	BoxParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < SegmentLocations.Num(); i++) {
		FVector2D offset = FMath::RandPointInCircle(maxVoronoiOffset);
		SegmentLocations[i].X += offset.X;
		SegmentLocations[i].Y += offset.Y;
		SegmentLocations[i] += FVector(0, 0, 50);
	}
	int boxVariantCount = Boxes.Num();
	for (int i = 0; i < SegmentLocations.Num(); i++) {
		int boxCount = (FMath::Rand() % (maxBoxCount - minBoxCount)) + minBoxCount;
		std::vector<float> spawnedBoxRadii;
		std::vector<FVector> spawnedBoxPositions;

		TSubclassOf<ABaseBox> firstBox = Boxes[FMath::Rand()%boxVariantCount];
		AActor* firstBoxPtr = GetWorld()->SpawnActor<AActor>(firstBox, SegmentLocations[i], FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams);
		float radiusCenter = firstBox->GetDefaultObject<ABaseBox>()->boxSize;
		RoomSegments.push_back(firstBoxPtr);
		spawnedBoxPositions.push_back(SegmentLocations[i]);
		spawnedBoxRadii.push_back(radiusCenter);
		

		TSubclassOf<ABaseBox> secondBox = Boxes[FMath::Rand() % boxVariantCount];
		float radiusSecond = secondBox->GetDefaultObject<ABaseBox>()->boxSize;
		AActor* previousBox = GetWorld()->SpawnActor<AActor>(secondBox, SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0), FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams);
		RoomSegments.push_back(previousBox);
		spawnedBoxPositions.push_back(SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0));
		spawnedBoxRadii.push_back(radiusSecond);



		for (int j = 0; j < (boxCount - 2); j++) {
			TSubclassOf<ABaseBox> thirdBox = Boxes[FMath::Rand() % boxVariantCount];
			float radiusThird = thirdBox->GetDefaultObject<ABaseBox>()->boxSize;

			FVector firstCandidate;
			FVector secondCandidate;

			if (FindThirdVertex(SegmentLocations[i], SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0),
				radiusCenter, radiusSecond, radiusThird, firstCandidate, secondCandidate)) {

				bool first = true;
				for (int k = 0; k < spawnedBoxRadii.size(); k++) {
					if (FVector::Dist(firstCandidate, spawnedBoxPositions[k]) < radiusThird + spawnedBoxRadii[k]) {
						first = false;
						break;
					}
				}
				if (!first) {
					for (int k = 0; k < spawnedBoxRadii.size(); k++) {
						if (FVector::Dist(secondCandidate, spawnedBoxPositions[k]) < radiusThird + spawnedBoxRadii[k]) {
							UE_LOG(LogTemp, Warning, TEXT("Print On Tick"));
							break;
						}
					}
				}
				RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(secondBox,first ? firstCandidate : secondCandidate , FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams));
				spawnedBoxPositions.push_back(first ? firstCandidate : secondCandidate);
				spawnedBoxRadii.push_back(radiusThird);
			}
			else {
				break;
			}
		}	
	}
	
	return;
}
*/
/*
void AGenerationEngine::GenerateBoxIslands(TArray<FVector>& SegmentLocations, FBox& BoundingVolume)
{
	FActorSpawnParameters BoxParams;
	BoxParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int i = 0; i < SegmentLocations.Num(); i++) {
		FVector2D offset = FMath::RandPointInCircle(maxVoronoiOffset);
		SegmentLocations[i].X += offset.X;
		SegmentLocations[i].Y += offset.Y;
		SegmentLocations[i] += FVector(0, 0, 50);
	}
	int boxVariantCount = Boxes.Num();
	for (int i = 0; i < SegmentLocations.Num(); i++) {
		int boxCount = (FMath::Rand() % (maxBoxCount - minBoxCount)) + minBoxCount;
		std::vector<float> spawnedBoxRadii;
		std::vector<FVector> spawnedBoxPositions;

		TSubclassOf<ABaseBox> firstBox = Boxes[FMath::Rand() % boxVariantCount];
		AActor* firstBoxPtr = GetWorld()->SpawnActor<AActor>(firstBox, SegmentLocations[i], FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams);
		float radiusCenter = firstBox->GetDefaultObject<ABaseBox>()->boxSize;
		RoomSegments.push_back(firstBoxPtr);
		spawnedBoxPositions.push_back(SegmentLocations[i]);
		spawnedBoxRadii.push_back(radiusCenter);


		TSubclassOf<ABaseBox> secondBox = Boxes[FMath::Rand() % boxVariantCount];
		float radiusSecond = secondBox->GetDefaultObject<ABaseBox>()->boxSize;
		AActor* previousBox = GetWorld()->SpawnActor<AActor>(secondBox, SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0), FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams);
		RoomSegments.push_back(previousBox);
		spawnedBoxPositions.push_back(SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0));
		spawnedBoxRadii.push_back(radiusSecond);

		float maxDist = 0.0f;
		float biggestRadius = 0.0f;

		for (int j = 0; j < (boxCount - 2); j++) {
			TSubclassOf<ABaseBox> thirdBox = Boxes[FMath::Rand() % boxVariantCount];
			float radiusThird = thirdBox->GetDefaultObject<ABaseBox>()->boxSize;

			biggestRadius = radiusThird > biggestRadius ? radiusThird : biggestRadius;

			FVector firstCandidate;
			FVector secondCandidate;

			if (FindThirdVertex(SegmentLocations[i], SegmentLocations[i] + FVector(radiusCenter + radiusSecond, 0, 0),
				radiusCenter + maxDist, radiusSecond + maxDist, radiusThird, firstCandidate, secondCandidate)) {
				for (int k = 0; k < spawnedBoxRadii.size(); k++) {
					if (FVector::Dist(firstCandidate, spawnedBoxPositions[k]) < radiusThird + spawnedBoxRadii[k]) {
						firstCandidate.X += radiusThird;
						maxDist += biggestRadius;
						biggestRadius = 0.0f;
						break;
					}
				}
				RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(secondBox, firstCandidate, FRotator(0, 0, FMath::RandRange(0.0, 360.0)), BoxParams));
				spawnedBoxPositions.push_back(firstCandidate);
				spawnedBoxRadii.push_back(radiusThird);
			}
			else {
				break;
			}
		}
	}

	return;
}
*/
void AGenerationEngine::GenerateBoxIslands(TArray<FVector>& SegmentLocations)
{
	FActorSpawnParameters BoxParams;
	BoxParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int i = 0; i < SegmentLocations.Num(); i++) {
		FVector2D offset = FMath::RandPointInCircle(maxVoronoiOffset);
		SegmentLocations[i].X += offset.X;
		SegmentLocations[i].Y += offset.Y;
		SegmentLocations[i] += FVector(0, 0, 50);
	}
	int boxVariantCount = Boxes.Num();
	for (int i = 0; i < SegmentLocations.Num(); i++) {
		int boxCount = (FMath::Rand() % (maxBoxCount - minBoxCount)) + minBoxCount;
		std::vector<float> spawnedBoxRadii;
		std::vector<FVector> spawnedBoxPositions;

		TSubclassOf<ABaseBox> firstBox = Boxes[FMath::Rand() % boxVariantCount];
		AActor* firstBoxPtr = GetWorld()->SpawnActor<AActor>(firstBox, SegmentLocations[i], FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
		float radiusCenter = firstBox->GetDefaultObject<ABaseBox>()->boxSize;
		RoomSegments.push_back(firstBoxPtr);
		spawnedBoxPositions.push_back(SegmentLocations[i]);
		spawnedBoxRadii.push_back(radiusCenter);

		for (int j = 0; j < (boxCount - 1); j++) {
			TSubclassOf<ABaseBox> thirdBox = Boxes[FMath::Rand() % boxVariantCount];
			float radiusThird = thirdBox->GetDefaultObject<ABaseBox>()->boxSize;
			float currentSearchRange = maxBoxIslandStartSize;
			for (uint32 k = 0; k < maxBoxSpawnAttemps; k++) {
				FVector2D boxOffset = FMath::RandPointInCircle(currentSearchRange);
				FVector newPosition = FVector(SegmentLocations[i].X + boxOffset.X, SegmentLocations[i].Y + boxOffset.Y, SegmentLocations[i].Z);
				bool isGood = true;
				for (int l = 0; l < spawnedBoxPositions.size(); l++) {
					if (FVector::Dist(spawnedBoxPositions[l], newPosition) < spawnedBoxRadii[l] + radiusThird) {
						isGood = false;
						currentSearchRange = currentSearchRange + boxIslandSizeIncrement <= maxBoxIslandSize ? currentSearchRange + boxIslandSizeIncrement : maxBoxIslandSize;
						break;
					}
				}
				if (isGood) {
					AActor* newBoxPtr = GetWorld()->SpawnActor<AActor>(thirdBox, newPosition, FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
					RoomSegments.push_back(newBoxPtr);
					spawnedBoxPositions.push_back(newPosition);
					spawnedBoxRadii.push_back(radiusThird);
				}
			}
			
		}
	}

	return;
}

bool AGenerationEngine::FindThirdVertex(const FVector& firstVertex, const FVector& secondVertex, float radius1, float radius2, float radius3, FVector& resultVertex1, FVector& resultVertex2)
{
	//float dx = secondVertex.X - firstVertex.X;
	//float dy = secondVertex.Y - firstVertex.Y;

	//float c = radius1 + radius2;
	//float b = radius1 + radius3;
	//float a = radius2 + radius3;

	//if (c == 0 || b + a <= c || b + c <= a || a + c <= b)
	//	return false;

	//float x = (b * b - a * a + c * c) / (2 * c);
	//float temp = b * b - x * x;
	//if (temp < 0) temp = 0;
	//float y = sqrtf(temp);

	//float exx = dx / c;
	//float exy = dy / c;

	//float eyx = -exy;
	//float eyy = exx;

	//resultVertex1 = FVector(firstVertex.X + x * exx + y * eyx, firstVertex.Y + x * exy + y * eyy, firstVertex.Z);
	//resultVertex2 = FVector(firstVertex.X + x * exx - y * eyx, firstVertex.Y + x * exy - y * eyy, firstVertex.Z);

	float d2 = radius1 + radius2;
	float d3 = radius1 + radius3;
	float d1 = radius2 + radius3;

	float k = (d2 * d2 + d1 *d1 - d3 *d3) / (2 * d2);
	float h = sqrt(d1 * d1 - k  * k);

	resultVertex1.X = secondVertex.X + (k / d2) * (firstVertex.X - secondVertex.X) - (h / d2) * (firstVertex.Y - secondVertex.Y);
	resultVertex1.Y = secondVertex.Y + (k / d2) * (firstVertex.Y - secondVertex.Y) + (h / d2) * (firstVertex.X - secondVertex.X);
	resultVertex1.Z = 5.0f;

	resultVertex2.X= secondVertex.X + (k / d2) * (firstVertex.X - secondVertex.X) + (h / d2) * (firstVertex.Y - secondVertex.Y);
	resultVertex2.Y = secondVertex.Y + (k / d2) * (firstVertex.Y - secondVertex.Y) - (h / d2) * (firstVertex.X - secondVertex.X);
	resultVertex2.Z = 5.0f;

	return true;
}

void AGenerationEngine::LoadCoridor(TSubclassOf	<AActor> coridor)
{
	Coridors.push_back(coridor);
}

void AGenerationEngine::UpdateRecastNavMeshPosition(const FVector newCenter)
{
	UE_LOG(LogTemp, Warning, TEXT("WESZLEM"));

	ANavMeshBoundsVolume* navMesh = Cast<ANavMeshBoundsVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ANavMeshBoundsVolume::StaticClass()));
	navMesh->SetActorLocation(newCenter);

	UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	navSystem->OnNavigationBoundsUpdated(navMesh);
	UE_LOG(LogTemp, Warning, TEXT("Wyszlem"));
}



