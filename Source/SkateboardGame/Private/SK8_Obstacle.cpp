// Fill out your copyright notice in the Description page of Project Settings.


#include "SK8_Obstacle.h"

#include "Components/BoxComponent.h"
#include "SkateboardGame/SkateboardGameCharacter.h"

// Sets default values
ASK8_Obstacle::ASK8_Obstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootSceneComponent;
	
	ScoreDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Trigger"));
	ScoreDetection->SetupAttachment(RootSceneComponent);
}

// Called when the game starts or when spawned
void ASK8_Obstacle::BeginPlay()
{
	Super::BeginPlay();
	ScoreDetection->OnComponentBeginOverlap.AddDynamic(this, &ASK8_Obstacle::OnBoxOverlapBegin);
}

void ASK8_Obstacle::OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(auto playerActor = CastChecked<ASkateboardGameCharacter>(OtherActor))
	{
		playerActor->OnScored();
		ScoreDetection->OnComponentBeginOverlap.RemoveDynamic(this, &ASK8_Obstacle::OnBoxOverlapBegin);
		UE_LOG(LogTemp, Warning, TEXT("Overlaping!"));
	}
}