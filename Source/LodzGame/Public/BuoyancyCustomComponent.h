// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuoyancyCustomComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LODZGAME_API UBuoyancyCustomComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuoyancyCustomComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	class AWaterLevelManager* WaterManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float BuoyancyForce = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float WaterDrag = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float SubmersionDepth = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	bool bApplyBuoyancy = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy|Rotation")
	bool bLimitRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy|Rotation")
	float MaxTiltAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy|Rotation")
	float RotationStabilizationSpeed = 2.0f;

protected:
	virtual void BeginPlay() override;

private:
	class UPrimitiveComponent* MeshComponent = nullptr;
	
	float CalculateSubmersionRatio();
	void ApplyBuoyancyForces(float DeltaTime);
	void StabilizeRotation(float DeltaTime);
};
