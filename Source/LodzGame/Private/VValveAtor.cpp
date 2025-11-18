// Fill out your copyright notice in the Description page of Project Settings.

#include "VValveAtor.h"
#include "WaterLevelManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AVValveAtor::AVValveAtor()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	ValveBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValveBase"));
	RootComponent = ValveBase;
	ValveBase->SetSimulatePhysics(false);
	ValveBase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ValveBase->SetCollisionObjectType(ECC_WorldStatic);
	
	ValveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValveMesh"));
	ValveMesh->SetupAttachment(RootComponent);
	ValveMesh->SetSimulatePhysics(false);
	ValveMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ValveMesh->SetCollisionObjectType(ECC_WorldDynamic);
	ValveMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ValveMesh->CanCharacterStepUpOn = ECB_No;
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(ValveMesh);
	InteractionSphere->SetSphereRadius(100.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AVValveAtor::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find WaterManager if not set
	if (!WaterLevelManager) {
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterLevelManager::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0) {
			WaterLevelManager = Cast<AWaterLevelManager>(FoundActors[0]);
		}
	}
}

// Called every frame
void AVValveAtor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bValveDetached)
		return;
	if (!bIsInteracting)
		return;

	if (!InteractingPlayer || !IsValid(InteractingPlayer))
	{
		if (bIsInteracting)
		{
			bIsInteracting = false;
			InteractingPlayer = nullptr;
		}
		return;
	}

	ACharacter* PlayerCharacter = InteractingPlayer->GetCharacter();
	if (!PlayerCharacter || !IsValid(PlayerCharacter))
	{
		StopInteraction();
		return;
	}

	float CheckDistance = FVector::Dist(PlayerCharacter->GetActorLocation(), GetActorLocation());
	if (CheckDistance > InteractionDistance)
	{
		StopInteraction();
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;

	if (InteractingPlayer->PlayerCameraManager)
	{
		InteractingPlayer->GetMousePosition(MouseX, MouseY);
	}
	else
	{
		StopInteraction();
		return;
	}

	float MouseDeltaX = MouseX - LastMousePosition.X;
	float MouseDeltaY = MouseY - LastMousePosition.Y;

	// Only process if mouse moved enough
	if (FMath::Abs(MouseDeltaX) > 0.01f || FMath::Abs(MouseDeltaY) > 0.01f)
	{
		RotateValve(MouseDeltaX, MouseDeltaY);
	}
}

void AVValveAtor::TryStartInteraction(APlayerController* PlayerController)
{
	if (!PlayerController || bValveDetached || bIsInteracting)
		return;

	ACharacter* PlayerCharacter = PlayerController->GetCharacter();
	if (!PlayerCharacter)
		return;

	float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), GetActorLocation());
	if (Distance > InteractionDistance)
		return;

	bIsInteracting = true;
	InteractingPlayer = PlayerController;

	// Get screen center
	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	ScreenCenter = FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

	float MouseX, MouseY;
	PlayerController->GetMousePosition(MouseX, MouseY);
	LastMousePosition = FVector2D(MouseX, MouseY);

	OriginalControlRotation = PlayerController->GetControlRotation();
	bOriginalShowMouseCursor = PlayerController->bShowMouseCursor;

	PlayerController->SetIgnoreLookInput(true);
	PlayerController->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	PlayerController->SetInputMode(InputMode);

	UE_LOG(LogTemp, Warning, TEXT("Valve interaction started! Player: %s"), *PlayerController->GetName());
}

void AVValveAtor::StopInteraction()
{
	if (!bIsInteracting || !InteractingPlayer)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Stopping valve interaction for player: %s"), *InteractingPlayer->GetName());

	InteractingPlayer->ResetIgnoreLookInput();
	InteractingPlayer->bShowMouseCursor = bOriginalShowMouseCursor;

	if (bOriginalShowMouseCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InteractingPlayer->SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		InteractingPlayer->SetInputMode(InputMode);
	}

	bIsInteracting = false;
	InteractingPlayer = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Valve interaction stopped! Camera should be unlocked now."));
}

void AVValveAtor::RotateValve(float MouseDeltaX, float MouseDeltaY)
{
	if (!ValveMesh || bValveDetached)
		return;

	// Calculate rotation angle based on circular mouse movement
	float MouseX, MouseY;
	InteractingPlayer->GetMousePosition(MouseX, MouseY);
	FVector2D CurrentMousePosition(MouseX, MouseY);

	float RotationDelta = CalculateRotationAngle(CurrentMousePosition, LastMousePosition);
	
	// Only allow clockwise rotation (positive values)
	if (RotationDelta > MinRotationSpeed)
	{
		CurrentRotation += RotationDelta;

		FRotator CurrentRot = ValveMesh->GetRelativeRotation();
		CurrentRot.Roll += RotationDelta;
		ValveMesh->SetRelativeRotation(CurrentRot);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Yellow, 
				FString::Printf(TEXT("Rotation: %.1f / %.1f"), CurrentRotation, RequiredRotation));
		}
	}

	LastMousePosition = CurrentMousePosition;

	if (CurrentRotation >= RequiredRotation)
	{
		DetachValve();
	}
}

float AVValveAtor::CalculateRotationAngle(FVector2D CurrentPos, FVector2D PreviousPos)
{
	FVector2D PrevVector = PreviousPos - ScreenCenter;
	FVector2D CurrentVector = CurrentPos - ScreenCenter;

	PrevVector.Normalize();
	CurrentVector.Normalize();

	// Calculate cross product (Z component in 2D space)
	float CrossZ = PrevVector.X * CurrentVector.Y - PrevVector.Y * CurrentVector.X;
	
	float DotProduct = FVector2D::DotProduct(PrevVector, CurrentVector);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	float Angle = FMath::Acos(DotProduct) * (180.0f / PI);

	// Clockwise = negative CrossZ in screen space (Y-axis points down)
	float DirectionalAngle = (CrossZ < 0) ? Angle : -Angle;

	return DirectionalAngle * MouseSensitivity;
}

void AVValveAtor::DetachValve()
{
	if (bValveDetached || !ValveMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("DetachValve called but already detached or no mesh!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting valve detachment..."));
	bValveDetached = true;

	StopInteraction();

	if (!IsValid(ValveMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("ValveMesh is not valid!"));
		return;
	}

	UStaticMesh* StaticMeshAsset = ValveMesh->GetStaticMesh();
	if (!StaticMeshAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("ValveMesh has no Static Mesh asset assigned! Cannot enable physics."));
		return;
	}

	if (!StaticMeshAsset->GetBodySetup())
	{
		UE_LOG(LogTemp, Error, TEXT("ValveMesh has no collision! Add collision in Static Mesh Editor."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Mesh is valid, proceeding with detachment..."));

	FTransform CurrentTransform = ValveMesh->GetComponentTransform();
	ValveMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	ValveMesh->SetWorldTransform(CurrentTransform);
	
	ValveMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ValveMesh->SetCollisionObjectType(ECC_PhysicsBody);
	ValveMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ValveMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	ValveMesh->SetMassOverrideInKg(NAME_None, 10.0f, true); // 10 kg
	
	ValveMesh->SetSimulatePhysics(true);
	ValveMesh->SetEnableGravity(true);
	
	if (!ValveMesh->IsSimulatingPhysics())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to enable physics on ValveMesh!"));
		ValveMesh->SetSimulatePhysics(false);
		ValveMesh->SetSimulatePhysics(true);
		
		if (!ValveMesh->IsSimulatingPhysics())
		{
			UE_LOG(LogTemp, Error, TEXT("Physics still not working! Check collision setup."));
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Physics enabled successfully!"));
	
	FVector ImpulseDirection = GetActorForwardVector() + FVector(0, 0, 0.5f);
	ImpulseDirection.Normalize();
	ValveMesh->AddImpulse(ImpulseDirection * 500.0f, NAME_None, true);

	FVector AngularVelocity = FVector(FMath::RandRange(-50.0f, 50.0f), 
									  FMath::RandRange(-50.0f, 50.0f), 
									  FMath::RandRange(-50.0f, 50.0f));
	ValveMesh->SetPhysicsAngularVelocityInDegrees(AngularVelocity);

	UE_LOG(LogTemp, Warning, TEXT("Valve detached successfully!"));

	LowerWaterLevel();
	PrimaryActorTick.bCanEverTick = false;
}

void AVValveAtor::LowerWaterLevel()
{
	if (!WaterLevelManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Water Level Manager not set! Skipping water level lowering."));
		return;
	}

	if (!IsValid(WaterLevelManager))
	{
		UE_LOG(LogTemp, Error, TEXT("Water Level Manager is not valid!"));
		return;
	}

	// Try to call Blueprint function LowerWater first
	UFunction* LowerWaterFunc = WaterLevelManager->FindFunction(FName("LowerWater"));
	if (LowerWaterFunc)
	{
		struct FLowerWaterParams
		{
			float Amount;
		};
		
		FLowerWaterParams Params;
		Params.Amount = WaterLevelDecrease;
		
		WaterLevelManager->ProcessEvent(LowerWaterFunc, &Params);
		UE_LOG(LogTemp, Warning, TEXT("Called LowerWater Blueprint function with amount: %.1f"), WaterLevelDecrease);
		return;
	}

	// Fallback: Direct property modification
	UClass* ManagerClass = WaterLevelManager->GetClass();
	if (!ManagerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get class from WaterLevelManager!"));
		return;
	}

	FProperty* WaterLevelProp = ManagerClass->FindPropertyByName(FName("WaterLevel"));
	if (!WaterLevelProp)
	{
		UE_LOG(LogTemp, Error, TEXT("WaterLevel property not found in BP_WaterLevelManager!"));
		return;
	}

	if (FFloatProperty* FloatProp = CastField<FFloatProperty>(WaterLevelProp))
	{
		float* CurrentWaterLevel = FloatProp->ContainerPtrToValuePtr<float>(WaterLevelManager);
		if (CurrentWaterLevel)
		{
			float OldLevel = *CurrentWaterLevel;
			*CurrentWaterLevel -= WaterLevelDecrease;
			
			UE_LOG(LogTemp, Warning, TEXT("Water level changed from %.1f to %.1f"), OldLevel, *CurrentWaterLevel);
			return;
		}
	}
	else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(WaterLevelProp))
	{
		double* CurrentWaterLevel = DoubleProp->ContainerPtrToValuePtr<double>(WaterLevelManager);
		if (CurrentWaterLevel)
		{
			double OldLevel = *CurrentWaterLevel;
			*CurrentWaterLevel -= WaterLevelDecrease;
			
			UE_LOG(LogTemp, Warning, TEXT("Water level changed from %.1f to %.1f"), OldLevel, *CurrentWaterLevel);
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("WaterLevel property type not supported (must be Float or Double)"));
}

