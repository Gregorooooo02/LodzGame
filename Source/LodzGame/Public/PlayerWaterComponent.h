// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerWaterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwimmingStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LODZGAME_API UPlayerWaterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerWaterComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float WalkSpeed = 230.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinSpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinSprintSpeedMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinJumpMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float MinGravityMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	class AWaterLevelManager* WaterManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float CharacterHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	float TransitionSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Slowdown")
	bool IsSprinting = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float SwimmingThreshold = 0.7f;

	UPROPERTY(BlueprintReadWrite, Category = "Swimming")
	bool bIsSwimming = false;

	UPROPERTY(BlueprintReadOnly, Category = "Swimming")
	float CurrentSubmersionLevel = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Swimming")
	FOnSwimmingStateChanged OnStartSwimming;

	UPROPERTY(BlueprintAssignable, Category = "Swimming")
	FOnSwimmingStateChanged OnStopSwimming;

protected:
	virtual void BeginPlay() override;

private:
	class UCharacterMovementComponent* MovementComponent = nullptr;
	
	float DefaultJumpVelocity = 400.0f;
	float DefaultGravityScale = 1.0f;
	
	float LastDesiredSpeed = 230.0f;
	float CurrentSpeedMultiplier = 1.0f;
	float CurrentJumpMultiplier = 1.0f;
	float CurrentGravityMultiplier = 1.0f;
	
	float CalculateSubmersionLevel();
	void UpdateSwimmingState(float SubmersionLevel);
	void UpdateMovementSpeed(float SubmersionLevel, float DeltaTime);
};
