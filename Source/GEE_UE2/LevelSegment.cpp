// Fill out your copyright notice in the Description page of Project Settings.

#include "GEE_UE2.h"
#include "LevelGenerator.h"
#include "LevelSegment.h"
#include "GEE_UE2Character.h"
#include "Kismet/GameplayStatics.h"
#include "WorldManager.h"
#include <string>

// Sets default values
ALevelSegment::ALevelSegment()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");;
	BoxCollider = CreateDefaultSubobject<UBoxComponent>("BoxCollider");
	BoxCollider->bGenerateOverlapEvents = true;
	BoxCollider->SetCollisionProfileName("Trigger");
	BoxCollider->SetupAttachment(RootComponent);
	BoxCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ALevelSegment::OnTriggerBeginOverlap);

	AActor::SetActorHiddenInGame(true);
}

FVector ALevelSegment::GetNeighbourPosition(int doorIndex)
{
	auto door = m_doors[doorIndex];
	auto vectorToDoor = door->GetComponentLocation() - this->GetActorLocation();
	vectorToDoor.Normalize();
	return this->GetActorLocation() + vectorToDoor*SegmentWidth;
}

void ALevelSegment::SetTriggerComponent(UBoxComponent* trigger)
{
	BoxCollider = trigger;
}

void ALevelSegment::AddDoor(USceneComponent* door)
{
	m_doors.Add(door);
}

void ALevelSegment::AddDoors(TArray<USceneComponent*> doors_array)
{
	m_doors.Append(doors_array);
}

TArray<USceneComponent*> ALevelSegment::GetDoors() const
{
	return m_doors;
}

bool ALevelSegment::IsNeeded() const
{
	return m_isNeeded;
}

void ALevelSegment::IsNeeded(bool val)
{
	m_isNeeded = val;
}

void ALevelSegment::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (OtherActor == MyCharacter)
	{
		if(WorldManager)
			WorldManager->LoadLevelSegments(this, NeighbourIndexes);
	}
}

LevelSegmentProperties ALevelSegment::ToLevelSegmentProperties() const
{
	return LevelSegmentProperties(Type, GetActorLocation(), GetActorRotation());
}

