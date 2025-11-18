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
			DefaultWalkSpeed = MovementComponent->MaxWalkSpeed;
			DefaultJumpVelocity = MovementComponent->JumpZVelocity;
			DefaultGravityScale = MovementComponent->GravityScale;
		}
	}

	// Auto-find WaterLevelManager if not assigned
	if (!WaterLevelManager)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterLevelManager::StaticClass(), FoundActors);
		
		if (FoundActors.Num() > 0)
		{
			WaterLevelManager = Cast<AWaterLevelManager>(FoundActors[0]);
		}
	}
}


// Called every frame
void UPlayerWaterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementComponent || !WaterLevelManager)
		return;

	float SubmersionLevel = CalculateSubmersionLevel();
	
	// Calculate target multipliers based on submersion
	float TargetSpeedMultiplier = FMath::Lerp(1.0f, MinSpeedMultiplier, SubmersionLevel);
	float TargetJumpMultiplier = FMath::Lerp(1.0f, MinJumpMultiplier, SubmersionLevel);
	float TargetGravityMultiplier = FMath::Lerp(1.0f, MinGravityMultiplier, SubmersionLevel);
	
	// Smooth transition
	CurrentSpeedMultiplier = FMath::FInterpTo(CurrentSpeedMultiplier, TargetSpeedMultiplier, DeltaTime, TransitionSpeed);
	CurrentJumpMultiplier = FMath::FInterpTo(CurrentJumpMultiplier, TargetJumpMultiplier, DeltaTime, TransitionSpeed);
	CurrentGravityMultiplier = FMath::FInterpTo(CurrentGravityMultiplier, TargetGravityMultiplier, DeltaTime, TransitionSpeed);
	
	// Apply to movement component
	MovementComponent->MaxWalkSpeed = DefaultWalkSpeed * CurrentSpeedMultiplier;
	MovementComponent->JumpZVelocity = DefaultJumpVelocity * CurrentJumpMultiplier;
	MovementComponent->GravityScale = DefaultGravityScale * CurrentGravityMultiplier;
}

float UPlayerWaterComponent::CalculateSubmersionLevel()
{
	if (!WaterLevelManager)
		return 0.0f;
	
	AActor* Owner = GetOwner();
	if (!Owner)
		return 0.0f;
	
	float CharacterZ = Owner->GetActorLocation().Z;
	float WaterZ = WaterLevelManager->WaterLevel;
	
	// Calculate depth in water (0 = feet at water surface, CharacterHeight = fully submerged)
	float DepthInWater = WaterZ - (CharacterZ - CharacterHeight * 0.5f);
	
	// Normalize to 0-1 range
	float SubmersionLevel = FMath::Clamp(DepthInWater / CharacterHeight, 0.0f, 1.0f);
	
	return SubmersionLevel;
}

