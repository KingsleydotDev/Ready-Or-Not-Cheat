#include "spawncivs.hpp"
#include "moduleruntime.hpp"
#include "../vars.h"
#include "../SDK/Basic.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/CoreUObject_structs.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/ReadyOrNot_classes.hpp"
#include <Windows.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>

namespace
{
	std::thread threadWorker;

	// Queued spawn locations — filled on button press, drained one per tick
	std::vector<SDK::FVector> spawnQueue;

	float RandomFloat(float fMin, float fMax)
	{
		float fRand = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
		return fMin + fRand * (fMax - fMin);
	}

	void SpawnOneCivilian(SDK::UWorld* pWorld, SDK::UClass* pCivClass, const SDK::FVector& spawnLocation)
	{
		SDK::FQuat identityQuat{};
		memset(&identityQuat, 0, sizeof(identityQuat));
		reinterpret_cast<double*>(&identityQuat)[3] = 1.0;

		SDK::FVector unitScale{ 1.0, 1.0, 1.0 };

		SDK::FTransform spawnTransform{};
		memset(&spawnTransform, 0, sizeof(spawnTransform));
		spawnTransform.Rotation = identityQuat;
		spawnTransform.Translation = spawnLocation;
		spawnTransform.Scale3D = unitScale;

		SDK::AActor* pNewActor = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(
			pWorld,
			pCivClass,
			spawnTransform,
			SDK::ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn,
			nullptr,
			SDK::ESpawnActorScaleMethod::OverrideRootScale
		);

		if (pNewActor == nullptr)
			return;

		SDK::UGameplayStatics::FinishSpawningActor(
			pNewActor,
			spawnTransform,
			SDK::ESpawnActorScaleMethod::OverrideRootScale
		);

		SDK::APawn* pNewPawn = static_cast<SDK::APawn*>(pNewActor);
		if (pNewPawn != nullptr)
		{
			pNewPawn->SpawnDefaultController();
		}
	}

	void RunWorker()
	{
		while (moduleruntime::WorkersActive())
		{
			spawncivs::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
		}
	}
}

void spawncivs::Apply()
{
	// Button was pressed — snapshot existing civ locations and build the queue
	if (vars::bSpawnCivs)
	{
		vars::bSpawnCivs = false;

		__try
		{
			SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
			if (pWorld == nullptr)
				return;

			SDK::TArray<SDK::AActor*> outActors;
			SDK::UGameplayStatics::GetAllActorsOfClass(
				pWorld,
				SDK::ACivilianCharacter::StaticClass(),
				&outActors
			);

			if (outActors.Num() == 0)
				return;

			const int32_t iSpawnCount = vars::iCivSpawnCount;

			// Snapshot: generate all spawn positions now, spawn them later one by one
			for (int32_t iActor = 0; iActor < outActors.Num(); iActor++)
			{
				SDK::AActor* pExisting = outActors[iActor];
				if (pExisting == nullptr)
					continue;

				SDK::FVector baseLocation = pExisting->K2_GetActorLocation();

				for (int32_t iCount = 0; iCount < iSpawnCount; iCount++)
				{
					SDK::FVector spawnLoc = baseLocation;
					spawnLoc.X += RandomFloat(-600.0f, 600.0f);
					spawnLoc.Y += RandomFloat(-600.0f, 600.0f);
					spawnQueue.push_back(spawnLoc);
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			// Silently ignore
		}

		return;
	}

	// Drip-feed: spawn one civilian per tick from the queue
	if (spawnQueue.empty())
		return;

	__try
	{
		SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
		if (pWorld == nullptr)
		{
			spawnQueue.clear();
			return;
		}

		SDK::UClass* pCivClass = SDK::ACivilianCharacter::StaticClass();
		if (pCivClass == nullptr)
		{
			spawnQueue.clear();
			return;
		}

		SDK::FVector nextLoc = spawnQueue.back();
		spawnQueue.pop_back();

		SpawnOneCivilian(pWorld, pCivClass, nextLoc);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// If something goes wrong, clear the queue to stop trying
		spawnQueue.clear();
	}
}

void spawncivs::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void spawncivs::Stop()
{
	spawnQueue.clear();
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}
