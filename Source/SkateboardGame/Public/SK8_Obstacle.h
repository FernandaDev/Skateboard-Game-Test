// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SK8_Obstacle.generated.h"

class UBoxComponent;

UCLASS()
class SKATEBOARDGAME_API ASK8_Obstacle : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	USceneComponent* RootSceneComponent;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* ScoreDetection;

	UPROPERTY(EditAnywhere)
	FName DetectionTag = "Player";

public:	
	// Sets default values for this actor's properties
	ASK8_Obstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
								 const FHitResult& SweepResult);
};
