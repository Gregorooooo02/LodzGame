// Fill out your copyright notice in the Description page of Project Settings.

#include "BuoyancyCustomComponent.h"
#include "WaterLevelManager.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBuoyancyCustomComponent::UBuoyancyCustomComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBuoyancyCustomComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (Owner)
	{
		MeshComponent = Owner->FindComponentByClass<UPrimitiveComponent>();
		
		// Automatically enable physics if not already enabled
		if (MeshComponent && !MeshComponent->IsSimulatingPhysics())
		{
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetEnableGravity(true);
		}
		
		// Increase angular damping for more stable rotation
		if (MeshComponent && bLimitRotation)
		{
			MeshComponent->SetAngularDamping(3.0f);
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
void UBuoyancyCustomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bApplyBuoyancy || !MeshComponent || !WaterManager)
		return;

	if (!MeshComponent->IsSimulatingPhysics())
		return;

	ApplyBuoyancyForces(DeltaTime);
	
	if (bLimitRotation)
	{
		StabilizeRotation(DeltaTime);
	}
}


float UBuoyancyCustomComponent::CalculateSubmersionRatio()
{
	if (!WaterManager || !MeshComponent)
		return 0.0f;

	AActor* Owner = GetOwner();
	if (!Owner)
		return 0.0f;

	float ObjectZ = Owner->GetActorLocation().Z;
	float WaterZ = WaterManager->WaterLevel;

	// Calculate how deep the object is in water
	float DepthInWater = WaterZ - (ObjectZ - SubmersionDepth * 0.5f);
	
	// Normalize to 0-1 range (0 = not in water, 1 = fully submerged)
	float SubmersionRatio = FMath::Clamp(DepthInWater / SubmersionDepth, 0.0f, 1.0f);
	
	return SubmersionRatio;
}

void UBuoyancyCustomComponent::ApplyBuoyancyForces(float DeltaTime)
{
	float SubmersionRatio = CalculateSubmersionRatio();
	
	if (SubmersionRatio <= 0.0f)
		return;

	// Get object mass for proper force calculation
	float ObjectMass = MeshComponent->GetMass();
	
	// Apply upward buoyancy force proportional to submersion and mass
	FVector BuoyancyVector = FVector(0.0f, 0.0f, BuoyancyForce * SubmersionRatio * ObjectMass);
	MeshComponent->AddForce(BuoyancyVector, NAME_None, false);

	// Apply water drag to slow down movement
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	FVector DragForce = -Velocity * WaterDrag * SubmersionRatio * ObjectMass;
	MeshComponent->AddForce(DragForce, NAME_None, false);

	// Dampen angular velocity (rotation) in water
	FVector AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();
	FVector AngularDrag = -AngularVelocity * WaterDrag * SubmersionRatio;
	MeshComponent->AddTorqueInDegrees(AngularDrag, NAME_None, false);
}

void UBuoyancyCustomComponent::StabilizeRotation(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !MeshComponent)
		return;

	FRotator CurrentRotation = Owner->GetActorRotation();
	
	// Calculate tilt from upright position
	float RollTilt = FMath::Abs(FMath::FindDeltaAngleDegrees(0.0f, CurrentRotation.Roll));
	float PitchTilt = FMath::Abs(FMath::FindDeltaAngleDegrees(0.0f, CurrentRotation.Pitch));
	
	// If tilted beyond max angle, apply corrective torque
	if (RollTilt > MaxTiltAngle || PitchTilt > MaxTiltAngle)
	{
		// Target rotation (upright, but keep yaw)
		FRotator TargetRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
		
		// Calculate rotation difference
		FRotator RotationDelta = (TargetRotation - CurrentRotation).GetNormalized();
		
		// Apply corrective torque
		FVector CorrectionTorque = FVector(
			RotationDelta.Pitch * RotationStabilizationSpeed * 1000.0f,
			RotationDelta.Yaw * RotationStabilizationSpeed * 100.0f,
			RotationDelta.Roll * RotationStabilizationSpeed * 1000.0f
		);
		
		MeshComponent->AddTorqueInDegrees(CorrectionTorque, NAME_None, false);
	}
	
	// Dampen excessive angular velocity
	FVector AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();
	if (AngularVelocity.SizeSquared() > 1000.0f)
	{
		FVector ExcessiveDamping = -AngularVelocity * 0.5f;
		MeshComponent->AddTorqueInDegrees(ExcessiveDamping, NAME_None, false);
	}
}

