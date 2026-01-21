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

	if (spawningNewRoom) {
		if (deletePreviousRoom) {
			
			for (AActor* segment : RoomSegments) {
				if (segment == previousCoridorPtr) continue;
				GetWorld()->DestroyActor(segment);
			}
			RoomSegments.clear();
			RoomSegments.push_back(previousCoridorPtr);
			deletePreviousRoom = false;
		}
		if (SpawnQueue.IsEmpty()) {
			spawningNewRoom = false;
			deletePreviousRoom = true;
			return;
		}
		SpawnParams Inst;
		const double StartTime = FPlatformTime::Seconds();
		while (SpawnQueue.Dequeue(Inst)) {
			RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Inst.ActorToSpawn,Inst.position,Inst.rotation,Inst.spawningParams));
			double elapsed = FPlatformTime::Seconds() - StartTime;
			if (elapsed > spawningTimeBudget) {
				break;
			}
		}

	}

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

void AGenerationEngine::SpawnNextRoomAsync(USceneComponent* exitPosition, AActor* previousCoridor, float waterLevel)
{
	double rotation = exitPosition->GetComponentRotation().Yaw; 
	FVector startingPoint = exitPosition->GetComponentLocation();
	previousCoridorPtr = previousCoridor;
	spawningNewRoom = true;
	if (currentRoomID == roomLimit) {

		//finalRoom
		SpawnParams params;
		params.ActorToSpawn = finalRoom;
		params.position = startingPoint;
		params.rotation = FRotator(0, rotation, 0);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		params.spawningParams = SpawnParams;
		SpawnQueue.Enqueue(params);
		return;
	}

	Async(EAsyncExecution::ThreadPool, [this,rotation, startingPoint,waterLevel]()
	{
		TArray<SpawnParams> LocalParams;
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FActorSpawnParameters Coridorparams;
		Coridorparams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Coridorparams.Owner = this;

		int32 dimX = (FMath::Rand() % (1 + maxRoomDim - minRoomDim)) + minRoomDim;
		int32 dimY = (FMath::Rand() % (1 + maxRoomDim - minRoomDim)) + minRoomDim;

		FVector XOffset(PartSize, 0, 0);
		FVector parallelOffset = XOffset.RotateAngleAxis(rotation, FVector::UpVector);
		FVector perpendicularOffset = parallelOffset.RotateAngleAxis(90, FVector::UpVector);

		FVector startPoint = startingPoint + 0.5f * parallelOffset;

		int32 offset = dimY / 2;

		FVector currentPoint = startPoint - offset * perpendicularOffset;

		bool deletedCorners[] = { false,false,false,false };
		int deletionCounter = 0;

		TArray<FVector> SitesPoints;


		for (int i = 0; i < dimX; ++i) {
			for (int j = 0; j < dimY; ++j) {
				if ((i == 0 || i == dimX - 1) && (j == 0 || j == dimY - 1)) {
					int32 segmentDeletionRoll = (FMath::Rand() % 100);
					if (segmentDeletionRoll > segmentDeletionChance) {
						LocalParams.Emplace(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params);
						//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
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
					LocalParams.Emplace(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params);
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(RoomSegment, currentPoint + (i * parallelOffset) + (j * perpendicularOffset), FRotator::ZeroRotator, params));
					SitesPoints.Add(currentPoint + (i * parallelOffset) + (j * perpendicularOffset));
				}
			}
		}

		TArray<FVector> previousValves;
		for (uint32 i = 0; i < valvesPerRoom; i++) {
			FVector randomSectorLocation = SitesPoints[FMath::Rand() % SitesPoints.Num()];
			{
				int xyRand = FMath::Rand() % 4;

				FVector valveFinalOffset;
				float finalRotation;

				switch (xyRand)
				{
				case 0: {
					valveFinalOffset = 0.5f * perpendicularOffset + 0.5f * parallelOffset;
					valveFinalOffset -= valveFinalOffset * valveOffsetPart;
					finalRotation = rotation - 135.0f;
					break;
				}
				case 1: {
					valveFinalOffset = 0.5f * perpendicularOffset - 0.5f * parallelOffset;
					valveFinalOffset -= valveFinalOffset * valveOffsetPart;
					finalRotation = rotation - 45.0f;
					break;
				}
				case 2: {
					valveFinalOffset = -0.5f * perpendicularOffset + 0.5f * parallelOffset;
					valveFinalOffset -= valveFinalOffset * valveOffsetPart;
					finalRotation = rotation + 135.0f;
					break;
				}
				case 3: {
					valveFinalOffset = -0.5f * perpendicularOffset - 0.5f * parallelOffset;
					valveFinalOffset -= valveFinalOffset * valveOffsetPart;
					finalRotation = rotation + 45.0f;
					break;
				}
				default:
					break;
				}

				valveFinalOffset = randomSectorLocation + valveFinalOffset + FVector(0, 0, 70);
				bool isBad = false;
				for (FVector& pos : previousValves) {
					if (FVector::Dist(pos, valveFinalOffset) < valveMinimumDistance) {
						isBad = true;
						break;
					}
				}
				if (isBad) {
					--i;
					continue;
				}

				previousValves.Push(valveFinalOffset);

				FActorSpawnParameters valveParams;
				valveParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				valveParams.Owner = this;

				//AActor* ValveActor = GetWorld()->SpawnActor<AActor>(BP_Valve, valveFinalOffset, FRotator(0, finalRotation, 0), valveParams);

				LocalParams.Emplace(BP_Valve, valveFinalOffset, FRotator(0, finalRotation, 0), valveParams);
			}
		}

		{
			FActorSpawnParameters BoxParams;
			BoxParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FVector verticalOffset(0,0,waterLevel);

			for (int i = 0; i < SitesPoints.Num(); i++) {
				FVector2D voronoiOffset = FMath::RandPointInCircle(maxVoronoiOffset);
				SitesPoints[i].X += voronoiOffset.X;
				SitesPoints[i].Y += voronoiOffset.Y;
				//SitesPoints[i] += FVector(0, 0, 50);
			}
			int boxVariantCount = Boxes.Num();
			for (int i = 0; i < SitesPoints.Num(); i++) {
				int boxCount = (FMath::Rand() % (maxBoxCount - minBoxCount)) + minBoxCount;
				std::vector<float> spawnedBoxRadii;
				std::vector<FVector> spawnedBoxPositions;

				TSubclassOf<ABaseBox> firstBox = Boxes[FMath::Rand() % boxVariantCount];
				LocalParams.Emplace(firstBox, SitesPoints[i] + verticalOffset, FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
				//AActor* firstBoxPtr = GetWorld()->SpawnActor<AActor>(firstBox, SitesPoints[i], FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
				//RoomSegments.push_back(firstBoxPtr);
				float radiusCenter = firstBox->GetDefaultObject<ABaseBox>()->boxSize;
				spawnedBoxPositions.push_back(SitesPoints[i]);
				spawnedBoxRadii.push_back(radiusCenter);

				for (int j = 0; j < (boxCount - 1); j++) {
					TSubclassOf<ABaseBox> thirdBox = Boxes[FMath::Rand() % boxVariantCount];
					float radiusThird = thirdBox->GetDefaultObject<ABaseBox>()->boxSize;
					float currentSearchRange = maxBoxIslandStartSize;
					for (uint32 k = 0; k < maxBoxSpawnAttemps; k++) {
						FVector2D boxOffset = FMath::RandPointInCircle(currentSearchRange);
						FVector newPosition = FVector(SitesPoints[i].X + boxOffset.X, SitesPoints[i].Y + boxOffset.Y, SitesPoints[i].Z);
						bool isGood = true;
						for (int l = 0; l < spawnedBoxPositions.size(); l++) {
							if (FVector::Dist(spawnedBoxPositions[l], newPosition) < spawnedBoxRadii[l] + radiusThird) {
								isGood = false;
								currentSearchRange = currentSearchRange + boxIslandSizeIncrement <= maxBoxIslandSize ? currentSearchRange + boxIslandSizeIncrement : maxBoxIslandSize;
								break;
							}
						}
						if (isGood) {
							//AActor* newBoxPtr = GetWorld()->SpawnActor<AActor>(thirdBox, newPosition, FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
							//RoomSegments.push_back(newBoxPtr);
							LocalParams.Emplace(thirdBox, newPosition + verticalOffset, FRotator(0, FMath::RandRange(0.0, 360.0), 0), BoxParams);
							spawnedBoxPositions.push_back(newPosition);
							spawnedBoxRadii.push_back(radiusThird);
						}
					}

				}
			}
		}

		{
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
					else if ((exitRoll >= twoSum) && (exitRoll < DoorWeightSum)) {
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
			int maxRooms = doorCount - 1;

			int deletionHalfSum = deletedCorners[0] + deletedCorners[1];

			//forward
			if ((exitMask & 1) > 0) {
				int startIndex = deletedCorners[2];
				int endIndex = deletedCorners[3] ? dimY - 2 : dimY - 1;
				int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;

				FVector exitPos = currentPoint + ((dimX - 0.5f) * parallelOffset) + doorPosIndex * perpendicularOffset;
				doorIndices[0] = doorPosIndex;
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation, 0), params));
				LocalParams.Emplace(Doorframe, exitPos, FRotator(0, rotation + 180, 0), params);

				int neighbourSegmentIndex = (dimX - 1) * dimY - (deletionHalfSum + deletedCorners[2]) + doorPosIndex;
				LocalParams[neighbourSegmentIndex].ActorToSpawn = AlternateRoomSegment;
				
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation, 0), Coridorparams));
				int32 optionalRoomRoll = (FMath::Rand() % 100);
				if (optionalRoomRoll > optionalRoomSpawnChance && maxRooms > 0) {
					int roomIndex = FMath::Rand() % OptionalRooms.Num();
					LocalParams.Emplace(OptionalRooms[roomIndex], exitPos, FRotator(0, rotation, 0), Coridorparams);
					maxRooms--;
				}
				else {
					int coridorIndex = FMath::Rand() % Coridors.size();
					LocalParams.Emplace(Coridors[coridorIndex], exitPos, FRotator(0, rotation, 0), Coridorparams);
					
				}
				
			}

			//left
			if ((exitMask & 2) > 0) {
				int startIndex = deletedCorners[1];
				int endIndex = deletedCorners[3] ? dimX - 2 : dimX - 1;
				int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
				FVector exitPos = currentPoint + ((dimY - 0.5f) * perpendicularOffset) + doorPosIndex * parallelOffset;
				doorIndices[1] = doorPosIndex;
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation + 90, 0), params));
				LocalParams.Emplace(Doorframe, exitPos, FRotator(0, rotation - 90, 0), params);
				
				int neighbourSegmentIndex;
				if (doorPosIndex == dimX - 1) {
					neighbourSegmentIndex = dimX * dimY - (deletionHalfSum + deletedCorners[2] + 1);
				}
				else {
					neighbourSegmentIndex = (doorPosIndex + 1) * dimY - (deletionHalfSum + 1);
				}
				LocalParams[neighbourSegmentIndex].ActorToSpawn = AlternateRoomSegment;

				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation + 90, 0), Coridorparams));
				int32 optionalRoomRoll = (FMath::Rand() % 100);
				if (optionalRoomRoll > optionalRoomSpawnChance && maxRooms > 0) {
					int roomIndex = FMath::Rand() % OptionalRooms.Num();
					LocalParams.Emplace(OptionalRooms[roomIndex], exitPos, FRotator(0, rotation + 90, 0), Coridorparams);
					maxRooms--;
				}
				else {
					int coridorIndex = FMath::Rand() % Coridors.size();
					LocalParams.Emplace(Coridors[coridorIndex], exitPos, FRotator(0, rotation + 90, 0), Coridorparams);
				}			
			}

			//right
			if ((exitMask & 4) > 0) {
				int startIndex = deletedCorners[0];
				int endIndex = deletedCorners[2] ? dimX - 2 : dimX - 1;
				int doorPosIndex = (FMath::Rand() % (1 + endIndex - startIndex)) + startIndex;
				FVector exitPos = currentPoint - 0.5f * perpendicularOffset + doorPosIndex * parallelOffset;
				doorIndices[2] = doorPosIndex;
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, exitPos, FRotator(0, rotation - 90, 0), params));
				LocalParams.Emplace(Doorframe, exitPos, FRotator(0, rotation + 90, 0), params);

				int neighbourSegmentIndex = doorPosIndex * dimY - deletionHalfSum;
				LocalParams[neighbourSegmentIndex].ActorToSpawn = AlternateRoomSegment;

				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Coridors[coridorIndex], exitPos, FRotator(0, rotation - 90, 0), Coridorparams));
				int32 optionalRoomRoll = (FMath::Rand() % 100);
				if (optionalRoomRoll > optionalRoomSpawnChance && maxRooms > 0) {
					int roomIndex = FMath::Rand() % OptionalRooms.Num();
					LocalParams.Emplace(OptionalRooms[roomIndex], exitPos, FRotator(0, rotation - 90, 0), Coridorparams);
					maxRooms--;
				}
				else {
					int coridorIndex = FMath::Rand() % Coridors.size();
					LocalParams.Emplace(Coridors[coridorIndex], exitPos, FRotator(0, rotation - 90, 0), Coridorparams);
				}
			}
		
		
			//Back wall
			FVector startWallPos = currentPoint - 0.5f * parallelOffset;
			for (int i = 0; i < dimY; i++) {
				if (deletedCorners[0] && i == 0) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params);
					continue;
				}
				if (deletedCorners[1] && i == dimY - 1) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params);
					continue;
				}
				if (i == offset) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(Doorframe, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params));
					LocalParams.Emplace(Doorframe, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params);
					continue;
				}
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params));
				LocalParams.Emplace(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation, 0), params);
			}

			//Left wall
			startWallPos += 0.5f * parallelOffset - 0.5f * perpendicularOffset;
			for (int i = 0; i < dimX; i++) {
				if (deletedCorners[0] && i == 0) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + 0.5f * perpendicularOffset + 0.5f * parallelOffset, FRotator(0, rotation, 0), params);
					continue;
				}
				if (deletedCorners[2] && i == dimX - 1) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
						//(ExternalWall, startWallPos + (i - 0.5f) * parallelOffset + 0.5f * perpendicularOffset, FRotator(0, rotation + 180, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + (i - 0.5f) * parallelOffset + 0.5f * perpendicularOffset, FRotator(0, rotation + 180, 0), params);
					continue;
				}
				if (doorIndices[2] != -1 && i == doorIndices[2]) {
					continue;
				}
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * parallelOffset, FRotator(0, rotation - 90, 0), params));
				LocalParams.Emplace(ExternalWall, startWallPos + i * parallelOffset, FRotator(0, rotation - 90, 0), params);
			}

			//Forward wall
			startWallPos += (dimX - 0.5f) * parallelOffset + 0.5f * perpendicularOffset;
			for (int i = 0; i < dimY; i++) {
				if (deletedCorners[2] && i == 0) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation + 90, 0), params);
					continue;
				}
				if (deletedCorners[3] && i == dimY - 1) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
						//(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos + (i - 0.5f) * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation - 90, 0), params);
					continue;
				}
				if (doorIndices[0] != -1 && i == doorIndices[0]) {
					continue;
				}
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation + 180, 0), params));
				LocalParams.Emplace(ExternalWall, startWallPos + i * perpendicularOffset, FRotator(0, rotation + 180, 0), params);
			}

			//Right wall
			startWallPos += -0.5f * parallelOffset + (dimY - 0.5f) * perpendicularOffset;
			for (int i = 0; i < dimX; i++) {
				if (deletedCorners[3] && i == 0) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
						//(ExternalWall, startWallPos - 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation + 180, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos - 0.5f * perpendicularOffset - 0.5f * parallelOffset, FRotator(0, rotation + 180, 0), params);
					continue;
				}
				if (deletedCorners[1] && i == dimX - 1) {
					//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>
						//(ExternalWall, startWallPos - (i - 0.5f) * parallelOffset - 0.5f * perpendicularOffset, FRotator(0, rotation, 0), params));
					LocalParams.Emplace(ExternalWall, startWallPos - (i - 0.5f) * parallelOffset - 0.5f * perpendicularOffset, FRotator(0, rotation, 0), params);
					continue;
				}
				if (doorIndices[1] != -1 && i == (dimX - (doorIndices[1] + 1))) {
					continue;
				}
				//RoomSegments.push_back(GetWorld()->SpawnActor<AActor>(ExternalWall, startWallPos - i * parallelOffset, FRotator(0, rotation + 90, 0), params));
				LocalParams.Emplace(ExternalWall, startWallPos - i * parallelOffset, FRotator(0, rotation + 90, 0), params);
			}



		}
		++currentRoomID;
		for (const SpawnParams& Inst : LocalParams)
		{
			SpawnQueue.Enqueue(Inst);
		}
	});
}

void AGenerationEngine::SpawnValve(const FVector& segmentLocation, const FVector& parallelOffset, const FVector& perpendicularOffset, float roomRotation)
{
	if (!BP_Valve)
		return;

	int xyRand = FMath::Rand() % 4;

	FVector valveFinalOffset;
	float finalRotation;

	switch (xyRand)
	{
	case 0: {
		valveFinalOffset = 0.5f * perpendicularOffset + 0.5f * parallelOffset;
		valveFinalOffset -= valveFinalOffset * valveOffsetPart;
		finalRotation = roomRotation - 135.0f;
		break;
	}
	case 1: {
		valveFinalOffset = 0.5f * perpendicularOffset -0.5f * parallelOffset;
		valveFinalOffset -= valveFinalOffset * valveOffsetPart;
		finalRotation = roomRotation - 45.0f;
		break;
	}
	case 2: {
		valveFinalOffset = -0.5f * perpendicularOffset + 0.5f * parallelOffset;
		valveFinalOffset -= valveFinalOffset * valveOffsetPart;
		finalRotation = roomRotation + 135.0f;
		break;
	}
	case 3: {
		valveFinalOffset = -0.5f * perpendicularOffset -0.5f * parallelOffset;
		valveFinalOffset -= valveFinalOffset * valveOffsetPart;
		finalRotation = roomRotation + 45.0f;
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



