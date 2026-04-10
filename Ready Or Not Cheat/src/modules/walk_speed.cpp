#include "walk_speed.hpp"
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

void RunWalkSpeedWorker()
{
    while (!(GetAsyncKeyState(VK_END) & 1))
    {
        if (!vars::bEnableWalkSpeed)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

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

        SDK::ACharacter* pCharacter = static_cast<SDK::ACharacter*>(pViewTarget);
        if (pCharacter == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        SDK::UCharacterMovementComponent* pMovement = pCharacter->CharacterMovement;
        if (pMovement == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        const float fWalkSpeed = vars::fWalkSpeed;
        const float fCrouchWalkSpeed = vars::fCrouchWalkSpeed;
        const float fRunSpeed = vars::fRunSpeed;

        // Directly write to movement component fields (like FOV does with camera fields)
        pMovement->MaxWalkSpeed = fRunSpeed;
        pMovement->MaxWalkSpeedCrouched = fCrouchWalkSpeed;

        // Also set the player character specific speed fields
        SDK::APlayerCharacter* pPlayerCharacter = static_cast<SDK::APlayerCharacter*>(pViewTarget);
        pPlayerCharacter->RunSpeed = fWalkSpeed;
        pPlayerCharacter->MaxRunSpeedPercent = 1.0f;
        pPlayerCharacter->CurrentRunSpeedPercent = 1.0f;
        pPlayerCharacter->WalkSpeedMultiplier = 1.0f;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void walk_speed::Start()
{
    std::thread(RunWalkSpeedWorker).detach();
}
