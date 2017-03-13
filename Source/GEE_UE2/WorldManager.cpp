// Fill out your copyright notice in the Description page of Project Settings.

#include "GEE_UE2.h"
#include "WorldManager.h"
#include "Kismet/GameplayStatics.h"
#include "LevelGenerator.h"
#include <string>
#include "EngineUtils.h"
#include "LevelUtils.h"
#include <functional>

AWorldManager::AWorldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AWorldManager::BeginPlay()
{
	Super::BeginPlay(); 
	initalizeLevelSegmentListsAndDictionaries();
	m_generatedLevel = LevelGenerator->GenerateLevel();
	loadWorldRootSegment(m_generatedLevel[0]);
}

//https://forums.unrealengine.com/showthread.php?3851-(39)-Rama-s-Extra-Blueprint-Nodes-for-You-as-a-Plugin-No-C-Required!&p=387524&viewfull=1#post387524
//https://github.com/EverNewJoy/VictoryPlugin/blob/master/Source/VictoryBPLibrary/Private/VictoryBPFunctionLibrary.cpp
ULevelStreaming* AWorldManager::loadLevelInstance(UObject* WorldContextObject, FString MapFolderOffOfContent, FString LevelName, int32 InstanceNumber, FVector Location, FRotator Rotation, bool& Success) const
{
	Success = false;
	if (!WorldContextObject) return nullptr;

	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (!World) return nullptr;
	//~~~~~~~~~~~

	//Full Name
	FString FullName = "/Game/" + MapFolderOffOfContent + "/" + LevelName;

	FName LevelFName = FName(*FullName);
	FString PackageFileName = FullName;

	ULevelStreamingKismet* StreamingLevel = NewObject<ULevelStreamingKismet>(World, ULevelStreamingKismet::StaticClass(), NAME_None, RF_Transient, NULL);
	
	if (!StreamingLevel)
	{
		return nullptr;
	}

	//Long Package Name
	FString LongLevelPackageName = FPackageName::FilenameToLongPackageName(PackageFileName);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Here is where a unique name is chosen for the new level asset
	// 	Ensure unique names to gain ability to have multiple instances of same level!
	//			<3 Rama

	//Create Unique Name based on BP-supplied instance value
	FString UniqueLevelPackageName = LongLevelPackageName;
	UniqueLevelPackageName += FString::FromInt(InstanceNumber);

	//Set!
	StreamingLevel->SetWorldAssetByPackageName(FName(*UniqueLevelPackageName));
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (World->IsPlayInEditor())
	{
		FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
		StreamingLevel->RenameForPIE(WorldContext.PIEInstance);
	}
	
	StreamingLevel->LevelColor = FColor::MakeRandomColor();
	StreamingLevel->bShouldBeLoaded = true;
	StreamingLevel->bShouldBeVisible = true;
	StreamingLevel->bShouldBlockOnLoad = false;
	StreamingLevel->bInitiallyLoaded = true;
	StreamingLevel->bInitiallyVisible = true;

	//Transform
	StreamingLevel->LevelTransform = FTransform(Rotation, Location);

	StreamingLevel->PackageNameToLoad = LevelFName;

	if (!FPackageName::DoesPackageExist(StreamingLevel->PackageNameToLoad.ToString(), NULL, &PackageFileName))
	{
		return nullptr;
	}

	//~~~

	//Actual map package to load
	StreamingLevel->PackageNameToLoad = FName(*LongLevelPackageName);

	//~~~

	// Add the new level to world.
	World->StreamingLevels.Add(StreamingLevel);

	Success = true;
	return StreamingLevel;
}

void AWorldManager::initalizeLevelSegmentListsAndDictionaries()
{
	for (FLevelSegmentMapping mapping : LevelSegmentMapping)
	{
		m_levelSegmentMapping.Add(mapping.Type, mapping.MapName);
		m_levelSegmentTypeCounter.Add(mapping.Type, 0);
	}
}

void AWorldManager::loadWorldRootSegment(const LevelSegmentProperties& properties)
{
	LoadLevelSegments(nullptr, properties.NeighbourIndexes);
	LoadLevelSegment(properties);
}

void AWorldManager::LoadLevelSegment(const LevelSegmentProperties& properties)
{
	if(isLocationInUse(properties.Location))
	{
		m_loadedLevelSegments[findUsedLocationIndex(properties.Location)]->IsNeeded(true);
		return;
	}

	bool success = false;
	ULevelStreaming* levelInstance = nullptr;
	do
	{
		if (levelInstance != nullptr)
			levelInstance->bIsRequestingUnloadAndRemoval = true;
		levelInstance = loadLevelInstance(this, MapFolderName, m_levelSegmentMapping[properties.Type], m_levelSegmentTypeCounter[properties.Type]++, properties.Location, properties.Rotation, success);
	} while (!success);

	m_newLoadedLevels.Add(TPairInitializer<ULevelStreaming*, LevelSegmentProperties>(levelInstance, properties));
	levelInstance->OnLevelShown.AddDynamic(this, &AWorldManager::setActiveLevelSegments);
}

void AWorldManager::LoadLevelSegments(ALevelSegment* root, const TArray<int>& neighbourIndexes)
{
	for (ALevelSegment* segment : m_loadedLevelSegments)
		segment->IsNeeded(false); 
	
	if (root != nullptr)
		root->IsNeeded(true);

	for (int index : neighbourIndexes)
		LoadLevelSegment(m_generatedLevel[index]);
}

void AWorldManager::unloadUnneededMaps()
{
	TArray<ALevelSegment*> unloadSegments;

	for (ALevelSegment* segment : m_loadedLevelSegments)
	{
		if (segment->IsNeeded())
			continue;
		unloadSegments.Add(segment);
	}

	for (ALevelSegment* segment : unloadSegments)
	{
		removeUsedLocation(segment->GetActorLocation());
		m_loadedLevelSegments.Remove(segment);

		//https://answers.unrealengine.com/questions/205702/unloading-instanced-levels.html !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		ULevelStreaming* level = FLevelUtils::FindStreamingLevel(segment->GetLevel());
		level->bIsRequestingUnloadAndRemoval = true;
	}
}

bool AWorldManager::isLocationInUse(const FVector& location)
{
	for (FVector used_location : m_usedLocations)
	{
		if (used_location.Equals(location))
			return true;
	}
	return false;
}

int AWorldManager::findUsedLocationIndex(const FVector& location)
{
	for(int i = 0; i < m_usedLocations.Num(); ++i)
	{
		if (m_usedLocations[i].Equals(location))
			return i;
	}
	return m_usedLocations.Num();
}

void AWorldManager::removeUsedLocation(const FVector& location)
{
	for(auto a : m_usedLocations)
	{
		if(a.Equals(location))
		{
			m_usedLocations.Remove(a);
			return;
		}
	}
}

void AWorldManager::setActiveLevelSegments()
{
	if (m_newLoadedLevels.Num() == 0) return;

	for (int i = 0; i < m_newLoadedLevels.Num(); ++i)
	{
		auto pair = m_newLoadedLevels[i];
		ULevelStreaming* level = pair.Key;

		if (level && level->IsLevelVisible())
		{
			for (auto a : level->GetLoadedLevel()->Actors)
			{
				ALevelSegment* segment = Cast<ALevelSegment>(a);
				if (segment)
				{
					segment->SetActorLocation(pair.Value.Location);
					segment->SetActorRotation(pair.Value.Rotation);
					segment->IsNeeded(true);
					segment->NeighbourIndexes = pair.Value.NeighbourIndexes;
					segment->WorldManager = this;
					segment->SetActorHiddenInGame(false);
					m_usedLocations.Add(segment->GetActorLocation());
					m_loadedLevelSegments.Add(segment); 
					m_newLoadedLevels.RemoveAt(i,1,false);
					break;
				}
			}
		}
	}
	m_newLoadedLevels.Shrink();

	if(m_newLoadedLevels.Num() == 0)
		unloadUnneededMaps();
}