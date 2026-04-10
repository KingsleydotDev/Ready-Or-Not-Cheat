#include "nightvision.hpp"
#include "../vars.h"
#include "../SDK/Basic.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/CoreUObject_structs.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/ReadyOrNot_classes.hpp"
#include <Windows.h>
#include <thread>
#include <chrono>

void RunNightVisionWorker()
{
	while (!(GetAsyncKeyState(VK_END) & 1))
	{
		SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
		if (pWorld == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		SDK::APlayerController* pPlayerController = SDK::UGameplayStatics::GetPlayerController(pWorld, 0);
		if (pPlayerController == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		SDK::AActor* pViewTarget = pPlayerController->GetViewTarget();
		if (pViewTarget == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		SDK::AReadyOrNotCharacter* pCharacter = static_cast<SDK::AReadyOrNotCharacter*>(pViewTarget);
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

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void nightvision::Start()
{
	std::thread(RunNightVisionWorker).detach();
}
