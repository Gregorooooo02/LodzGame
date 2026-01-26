// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWaterComponent.h"
#include "WaterLevelManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UPlayerWaterComponent::UPlayerWaterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerWaterComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
		if (MovementComponent)
		{
			DefaultGravityScale = MovementComponent->GravityScale;
		}
	}

	// Auto-find WaterLevelManager if not assigned
	if (!WaterManager)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterLevelManager::StaticClass(), FoundActors);
		
		if (FoundActors.Num() > 0)
		{
			WaterManager = Cast<AWaterLevelManager>(FoundActors[0]);
		}
	}
}


// Called every frame
void UPlayerWaterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementComponent || !WaterManager)
		return;

	float SubmersionLevel = CalculateSubmersionLevel();
	CurrentSubmersionLevel = SubmersionLevel;
	
	float TargetGravityMultiplier = FMath::Lerp(1.0f, MinGravityMultiplier, SubmersionLevel);
	
	CurrentGravityMultiplier = FMath::FInterpTo(CurrentGravityMultiplier, TargetGravityMultiplier, DeltaTime, TransitionSpeed);
	
	MovementComponent->GravityScale = DefaultGravityScale * CurrentGravityMultiplier;

	UpdateMovementSpeed(SubmersionLevel, DeltaTime);
	UpdateAnimationSpeed(SubmersionLevel, DeltaTime);
}

void UPlayerWaterComponent::UpdateMovementSpeed(float SubmersionLevel, float DeltaTime)
{
	if (!MovementComponent)
		return;

	// Determine base speed based on whether player is sprinting
	float DesiredSpeed = IsSprinting ? SprintSpeed : WalkSpeed;

	// If in water, apply slowdown
	if (SubmersionLevel >= 0.01f)
	{
		float MinMultiplier = IsSprinting ? MinSprintSpeedMultiplier : MinSpeedMultiplier;

		// Calculate target multiplier based on submersion
		float TargetMultiplier = FMath::Lerp(1.0f, MinMultiplier, SubmersionLevel);
		CurrentSpeedMultiplier = FMath::FInterpTo(CurrentSpeedMultiplier, TargetMultiplier, DeltaTime, TransitionSpeed);

		DesiredSpeed *= CurrentSpeedMultiplier;
	}
	else
	{
		// Not in water - reset multiplier to 1.0
		CurrentSpeedMultiplier = FMath::FInterpTo(CurrentSpeedMultiplier, 1.0f, DeltaTime, TransitionSpeed);
		DesiredSpeed *= CurrentSpeedMultiplier;
	}

	// Set both walk and swim speed to ensure proper behavior in both modes
	MovementComponent->MaxWalkSpeed = DesiredSpeed;
	MovementComponent->MaxSwimSpeed = DesiredSpeed;
}

void UPlayerWaterComponent::UpdateAnimationSpeed(float SubmersionLevel, float DeltaTime) 
{
	if (SubmersionLevel >= 0.01f)
	{
		float TargetAnimSpeed = FMath::Lerp(1.0f, MinAnimationSpeedMultiplier, SubmersionLevel);
		CurrentAnimationSpeedMultiplier = FMath::FInterpTo(CurrentAnimationSpeedMultiplier, TargetAnimSpeed, DeltaTime, TransitionSpeed);
	}
	else
	{
		CurrentAnimationSpeedMultiplier = FMath::FInterpTo(CurrentAnimationSpeedMultiplier, 1.0f, DeltaTime, TransitionSpeed);
	}
}

float UPlayerWaterComponent::CalculateSubmersionLevel()
{
	if (!WaterManager)
		return 0.0f;
	
	AActor* Owner = GetOwner();
	if (!Owner)
		return 0.0f;
	
	float CharacterZ = Owner->GetActorLocation().Z;
	float WaterZ = WaterManager->WaterLevel;
	
	// Calculate depth in water (0 = feet at water surface, CharacterHeight = fully submerged)
	float DepthInWater = WaterZ - (CharacterZ - CharacterHeight * 0.5f);
	
	// Normalize to 0-1 range
	float SubmersionLevel = FMath::Clamp(DepthInWater / CharacterHeight, 0.0f, 1.0f);
	
	return SubmersionLevel;
}