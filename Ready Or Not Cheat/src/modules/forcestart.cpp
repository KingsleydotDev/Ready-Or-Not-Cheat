#include "forcestart.hpp"
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

namespace
{
	std::thread threadWorker;

	void RunWorker()
	{
		while (moduleruntime::WorkersActive())
		{
			forcestart::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}

void forcestart::Apply()
{
	if (!vars::bEnableForceStart)
	{
		return;
	}

	__try
	{
		SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
		if (pWorld == nullptr)
		{
			return;
		}

		SDK::AGameStateBase* pGameStateBase = pWorld->GameState;
		if (pGameStateBase == nullptr)
		{
			return;
		}

		// Get all player states from the game state
		auto& playerArray = pGameStateBase->PlayerArray;

		for (int32_t i = 0; i < playerArray.Num(); i++)
		{
			SDK::APlayerState* pBaseState = playerArray[i];
			if (pBaseState == nullptr)
			{
				continue;
			}

			// Cast to ReadyOrNotPlayerState to access bReady
			SDK::AReadyOrNotPlayerState* pState = static_cast<SDK::AReadyOrNotPlayerState*>(pBaseState);
			if (pState == nullptr)
			{
				continue;
			}

			// Force everyone to ready
			pState->bReady = true;
		}

		// Also call ReadyUp on the local player controller for good measure
		SDK::APlayerController* pPC = SDK::UGameplayStatics::GetPlayerController(pWorld, 0);
		if (pPC != nullptr)
		{
			SDK::AReadyOrNotPlayerController* pRONPC = static_cast<SDK::AReadyOrNotPlayerController*>(pPC);
			if (pRONPC != nullptr)
			{
				pRONPC->ReadyUp();
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Silently ignore
	}

	// One-shot: disable after firing
	vars::bEnableForceStart = false;
}

void forcestart::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void forcestart::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}