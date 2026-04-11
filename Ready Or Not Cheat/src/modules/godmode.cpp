#include "godmode.hpp"
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
			godmode::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	}
}

void godmode::Apply()
{
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

	SDK::AReadyOrNotCharacter* pCharacter = static_cast<SDK::AReadyOrNotCharacter*>(pPlayerPawn);
	if (pCharacter == nullptr)
	{
		return;
	}

	// Set the game's built-in god mode flag directly in memory
	// This is checked internally by the damage system to skip all damage/death
	pCharacter->bGodMode = vars::bEnableGodMode ? 1 : 0;
}

void godmode::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void godmode::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}
