#include "nightvision.hpp"
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
			nightvision::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	}
}

void nightvision::Apply()
{
	if (!vars::bEnableNightVision && !vars::bNightVisionState)
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

	SDK::AActor* pViewTarget = pPlayerController->GetViewTarget();
	if (pViewTarget == nullptr)
	{
		return;
	}

	SDK::AReadyOrNotCharacter* pCharacter = static_cast<SDK::AReadyOrNotCharacter*>(pViewTarget);
	if (pCharacter == nullptr)
	{
		return;
	}

	if (vars::bEnableNightVision && !vars::bNightVisionState)
	{
		pCharacter->EnableNightVisionGoggles();
		vars::bNightVisionState = true;
	}
	else if (!vars::bEnableNightVision && vars::bNightVisionState)
	{
		pCharacter->DisableNightVisionGoggles();
		vars::bNightVisionState = false;
	}
}

void nightvision::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void nightvision::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}
