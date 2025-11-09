// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnFirstCorridor();
};
