#include "walk_speed.hpp"
#include "moduleruntime.hpp"
#include "../vars.h"
#include "../SDK/Basic.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/CoreUObject_structs.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/ReadyOrNot_classes.hpp"
#include <chrono>
#include <thread>

namespace
{
	std::thread threadWorker;

	void RunWorker()
	{
		while (moduleruntime::WorkersActive())
		{
			walk_speed::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	}
}

void walk_speed::Apply()
{
	if (!vars::bEnableWalkSpeed)
	{
		return;
	}

	SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
	if (pWorld == nullptr)
	{
		return;
	}

	SDK::APlayerController* pPlayerController = SDK::UGameplayStatics::GetPlayerController(pWorld, 0);
	if (pPlayerController == nullptr)
	{
		return;
	}

	SDK::APawn* pPlayerPawn = pPlayerController->AcknowledgedPawn;
	if (pPlayerPawn == nullptr)
	{
		return;
	}

	SDK::UWorldPartitionSubsystem* pPartitionSubsystem = static_cast<SDK::UWorldPartitionSubsystem*>(
		SDK::USubsystemBlueprintLibrary::GetWorldSubsystem(
			pWorld,
			SDK::TSubclassOf<SDK::UWorldSubsystem>(SDK::UWorldPartitionSubsystem::StaticClass())));
	if (pPartitionSubsystem != nullptr)
	{
		if (!pPartitionSubsystem->IsAllStreamingCompleted())
		{
			return;
		}
	}

	SDK::ACharacter* pCharacter = static_cast<SDK::ACharacter*>(pPlayerPawn);
	if (pCharacter == nullptr)
	{
		return;
	}

	SDK::UCharacterMovementComponent* pMovement = pCharacter->CharacterMovement;
	if (pMovement == nullptr)
	{
		return;
	}

	const float fWalkSpeed = vars::fWalkSpeed;
	const float fCrouchWalkSpeed = vars::fCrouchWalkSpeed;
	const float fRunSpeed = vars::fRunSpeed;

	pMovement->MaxWalkSpeed = fRunSpeed;
	pMovement->MaxWalkSpeedCrouched = fCrouchWalkSpeed;

	SDK::APlayerCharacter* pPlayerCharacter = static_cast<SDK::APlayerCharacter*>(pPlayerPawn);
	pPlayerCharacter->RunSpeed = fWalkSpeed;
	pPlayerCharacter->MaxRunSpeedPercent = 1.0f;
	pPlayerCharacter->CurrentRunSpeedPercent = 1.0f;
	pPlayerCharacter->WalkSpeedMultiplier = 1.0f;
}

void walk_speed::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void walk_speed::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}
