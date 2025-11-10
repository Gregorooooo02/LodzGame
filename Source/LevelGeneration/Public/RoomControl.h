// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenerationEngine.h"
#include "RoomControl.generated.h"

UCLASS()
class LEVELGENERATION_API ARoomControl : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomControl();

	UFUNCTION(BlueprintCallable, Category = "Rooms")
		void TriggerNextRoomSpawn(USceneComponent* exitPosition, AActor* caller);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
