// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWaterAudio.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UPlayerWaterAudio::UPlayerWaterAudio()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerWaterAudio::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerWaterAudio::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckWaterOverlap();
}

void UPlayerWaterAudio::CheckWaterOverlap() 
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character) return;

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return;

	FVector FeetLocation = Character->GetActorLocation();
	FeetLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		FeetLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(WaterCheckRadius),
		QueryParams
	);

	bIsInWater = false;

	if (bHit)
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			if (Result.GetActor())
			{
				for (const FName& Tag : WaterTags)
				{
					if (Result.GetActor()->ActorHasTag(Tag))
					{
						bIsInWater = true;

						if (Result.GetComponent())
						{
							FBox Bounds = Result.GetComponent()->Bounds.GetBox();
							WaterSurfaceZ = Bounds.Max.Z;
						}

						break;
					}
				}
			}

			if (bIsInWater) break;
		}
	}

	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugSphere(GetWorld(), FeetLocation, WaterCheckRadius, 12, bIsInWater ? FColor::Blue : FColor::Red, false, 0.15f);
	}
}