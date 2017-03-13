// Fill out your copyright notice in the Description page of Project Settings.

#include "GEE_UE2.h"
#include "LevelGenerator.h"

ALevelGenerator::ALevelGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

TArray<LevelSegmentProperties> ALevelGenerator::GenerateLevel()
{
	m_randomStream.Initialize(RandomSeed);
	auto root = createRandomSegment(FVector::ZeroVector);

	TArray<LevelSegmentProperties> segmentProperties {root->ToLevelSegmentProperties()};
	TArray<ALevelSegment*> segmentInstances {root};

	for(int i = 1; i <= LevelSegments; ++i)
	{
		int neighbourIndex = i - 1;

		if (neighbourIndex >= segmentInstances.Num())
			break;

		auto neighbour = segmentInstances[neighbourIndex];
		for(int doorIndex = 0; doorIndex < neighbour->GetDoors().Num(); ++doorIndex)
		{
			auto location = neighbour->GetNeighbourPosition(doorIndex);
			bool continueLoop = false;

			for (auto instance : segmentInstances)
			{
				if(instance->GetActorLocation().Equals(location, 1))
				{
					continueLoop = true;
					segmentProperties[neighbourIndex].NeighbourIndexes.Add(segmentInstances.Find(instance));
					break;
				}
			}

			if (continueLoop || segmentInstances.Num() >= LevelSegments)
				continue;

			auto suitableSegment = createSuitableSegment(neighbour, location);
			segmentInstances.Add(suitableSegment);
			segmentProperties.Add(suitableSegment->ToLevelSegmentProperties());
			segmentProperties[neighbourIndex].NeighbourIndexes.Add(segmentInstances.Find(suitableSegment));
		}
	}

	for (auto segment_instance : segmentInstances)
		GetWorld()->DestroyActor(segment_instance);
	
	return segmentProperties;
}

ALevelSegment* ALevelGenerator::createRandomSegment(FVector location)
{
	int randomSegmentIndex = m_randomStream.RandRange(0, LevelSegmentBlueprints.Num() - 1);
	FRotator randomRotation = FRotator::MakeFromEuler(FVector(0, 0, m_randomStream.RandRange(0, 3) * 90));
	AActor* actor =  GetWorld()->SpawnActor(LevelSegmentBlueprints[randomSegmentIndex]);
	actor->SetActorLocationAndRotation(location, randomRotation);
	return Cast<ALevelSegment>(actor);
}

bool ALevelGenerator::checkIfNeighbourDoor(ALevelSegment* segment, ALevelSegment* other)
{
	for (auto segmentDoor : segment->GetDoors())
	{
		for (auto otherDoor : other->GetDoors())
		{
			if (segmentDoor->GetComponentLocation().Equals(otherDoor->GetComponentLocation(), 1))
				return true;
		}
	}
	return false;
}

ALevelSegment* ALevelGenerator::createSuitableSegment(ALevelSegment* suitableFor, FVector location)
{
	ALevelSegment* newSegment = nullptr;
	do
	{
		if(newSegment != nullptr)
		{
			GetWorld()->DestroyActor(newSegment);
			newSegment = nullptr;
		}
		newSegment = createRandomSegment(location);
	} while (!checkIfNeighbourDoor(suitableFor, newSegment));
	return newSegment;
}