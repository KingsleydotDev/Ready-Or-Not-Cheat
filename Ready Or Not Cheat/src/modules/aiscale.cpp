#include "aiscale.hpp"
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
			aiscale::Apply();
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}
}

void aiscale::Apply()
{
	if (!vars::bEnableAIScale)
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

		SDK::AReadyOrNotGameState* pGameState = static_cast<SDK::AReadyOrNotGameState*>(pGameStateBase);
		if (pGameState == nullptr)
		{
			return;
		}

		const float fScale = vars::fAIScale;
		SDK::FVector vScale{ fScale, fScale, fScale };

		// Scale all AI characters (suspects, civilians, SWAT AI)
		auto& aiCharacters = pGameState->AllAICharacters;
		for (int32_t i = 0; i < aiCharacters.Num(); i++)
		{
			SDK::ACyberneticCharacter* pAI = aiCharacters[i];
			if (pAI == nullptr)
			{
				continue;
			}

			pAI->SetActorScale3D(vScale);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Silently ignore
	}
}

void aiscale::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void aiscale::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}