// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VValveAtor.generated.h"

UCLASS()
class LODZGAME_API AVValveAtor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVValveAtor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ValveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ValveBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* InteractionSphere;

	// Valve Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	float MouseSensitivity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	float RequiredRotation = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	float InteractionDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	float WaterLevelDecrease = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	class AActor* WaterLevelManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Valve Settings")
	float MinRotationSpeed = 0.5f;

	// Public functions to use in Blueprints
	UFUNCTION(BlueprintCallable, Category = "Valve")
	void TryStartInteraction(class APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Valve")
	void StopInteraction();

	UFUNCTION(BlueprintCallable, Category = "Valve")
	bool CanInteract() const { return !bIsInteracting && !bValveDetached; }

protected:
	// Internal functions
	void RotateValve(float MouseDeltaX, float MouseDeltaY);
	void DetachValve();
	void LowerWaterLevel();

private:
	bool bIsInteracting = false;
	bool bValveDetached = false;
	float CurrentRotation = 0.0f;
	FVector2D LastMousePosition;
	FVector2D ScreenCenter;
	
	UPROPERTY()
	class APlayerController* InteractingPlayer = nullptr;
	
	FRotator OriginalControlRotation;
	bool bOriginalShowMouseCursor = false;
	
	float CalculateRotationAngle(FVector2D CurrentPos, FVector2D PreviousPos);
};
