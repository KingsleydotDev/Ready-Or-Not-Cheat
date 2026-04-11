#include "instastart.hpp"
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
			instastart::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void instastart::Apply()
{
	if (!vars::bEnableInstaStart)
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

		// Check if a mission is currently counting down
		bool bStarting = false;
		float fCountdown = 0.0f;

		SDK::AMissionPortal::IsMissionStarting(&bStarting, &fCountdown);

		// If the countdown is active and has more than 0.5 seconds left, force it to near-zero
		if (bStarting && fCountdown > 0.5f)
		{
			// Find the MissionPortal actor in the world
			SDK::AActor* pFoundActor = SDK::UGameplayStatics::GetActorOfClass(pWorld, SDK::AMissionPortal::StaticClass());
			if (pFoundActor == nullptr)
			{
				return;
			}

			SDK::AMissionPortal* pPortal = static_cast<SDK::AMissionPortal*>(pFoundActor);
			if (pPortal == nullptr)
			{
				return;
			}

			// Fire the timer with a very short countdown to start instantly
			pPortal->Multicast_SetTimer(true, 0.1f);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Silently ignore any access violations
	}

	// One-shot: disable after firing
	vars::bEnableInstaStart = false;
}

void instastart::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void instastart::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}