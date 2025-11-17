// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ValveInteractionComponent.generated.h"

/**
 * Komponent do dodania do Character gracza - obs³uguje interakcjê z zaworem
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LODZGAME_API UValveInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UValveInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Binding input
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteractWithValve();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopValveInteraction();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 300.0f;

private:
	UPROPERTY()
	class AVValveAtor* CurrentValve = nullptr;

	class APlayerController* GetPlayerController() const;
	class AVValveAtor* FindNearbyValve();
};
