// Fill out your copyright notice in the Description page of Project Settings.

#include "ValveInteractionComponent.h"
#include "VValveAtor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UValveInteractionComponent::UValveInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UValveInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UValveInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AVValveAtor* NearbyValve = FindNearbyValve();
	if (NearbyValve && NearbyValve->CanInteract())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Green, TEXT("Press LMB to use valve"));
		}
	}
}

void UValveInteractionComponent::TryInteractWithValve()
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
		return;

	if (CurrentValve)
	{
		StopValveInteraction();
		return;
	}

	AVValveAtor* NearbyValve = FindNearbyValve();
	if (NearbyValve && NearbyValve->CanInteract())
	{
		CurrentValve = NearbyValve;
		CurrentValve->TryStartInteraction(PC);
	}
}

void UValveInteractionComponent::StopValveInteraction()
{
	if (CurrentValve)
	{
		CurrentValve->StopInteraction();
		CurrentValve = nullptr;
	}
}

APlayerController* UValveInteractionComponent::GetPlayerController() const
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		return Cast<APlayerController>(Owner->GetController());
	}
	return nullptr;
}

AVValveAtor* UValveInteractionComponent::FindNearbyValve()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return nullptr;

	TArray<AActor*> FoundValves;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVValveAtor::StaticClass(), FoundValves);

	AVValveAtor* ClosestValve = nullptr;
	float ClosestDistance = InteractionRange;

	for (AActor* ValveActor : FoundValves)
	{
		AVValveAtor* Valve = Cast<AVValveAtor>(ValveActor);
		if (Valve && Valve->CanInteract())
		{
			float Distance = FVector::Dist(Owner->GetActorLocation(), Valve->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestValve = Valve;
			}
		}
	}

	return ClosestValve;
}
