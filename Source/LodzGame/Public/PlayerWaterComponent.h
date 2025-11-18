// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerWaterComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LODZGAME_API UPlayerWaterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerWaterComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinSpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinJumpMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinGravityMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	class AWaterLevelManager* WaterLevelManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float CharacterHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float TransitionSpeed = 5.0f;

private:
	class UCharacterMovementComponent* MovementComponent = nullptr;
	
	float DefaultWalkSpeed = 600.0f;
	float DefaultJumpVelocity = 420.0f;
	float DefaultGravityScale = 1.0f;
	
	float CurrentSpeedMultiplier = 1.0f;
	float CurrentJumpMultiplier = 1.0f;
	float CurrentGravityMultiplier = 1.0f;
	
	float CalculateSubmersionLevel();
};
