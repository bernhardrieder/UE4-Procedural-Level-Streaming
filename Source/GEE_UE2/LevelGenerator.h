// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "LevelSegment.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

struct LevelSegmentProperties
{
	LevelSegmentProperties() : LevelSegmentProperties(ELevelSegmentTypes::LST_None, FVector::ZeroVector, FRotator::ZeroRotator) {	}
	LevelSegmentProperties(ELevelSegmentTypes type, FVector location, FRotator rotation) : Type(type), Location(location), Rotation(rotation) { }

	ELevelSegmentTypes Type;
	FVector Location;
	FRotator Rotation;
	TArray<int> NeighbourIndexes;
};

UCLASS()
class GEE_UE2_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<ALevelSegment>> LevelSegmentBlueprints;

	UPROPERTY(EditAnywhere)
	int RandomSeed;

	UPROPERTY(EditAnywhere)
	int LevelSegments;
	
public:	
	ALevelGenerator();
	TArray<LevelSegmentProperties> GenerateLevel();

private:
	ALevelSegment* createRandomSegment(FVector location);
	bool checkIfNeighbourDoor(ALevelSegment* segment, ALevelSegment* other);
	ALevelSegment* createSuitableSegment(ALevelSegment* suitableFor, FVector location);
	
	FRandomStream m_randomStream;
};
