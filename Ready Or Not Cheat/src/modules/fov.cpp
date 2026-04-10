#include "fov.hpp"
#include "../vars.h"
#include "../SDK/Basic.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/CoreUObject_structs.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/ReadyOrNot_classes.hpp"

void fov::Apply()
{
	if (!vars::bEnableFOV)
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

	SDK::APlayerCameraManager* pCameraManager = pPlayerController->PlayerCameraManager;
	if (pCameraManager == nullptr)
	{
		pCameraManager = SDK::UGameplayStatics::GetPlayerCameraManager(pWorld, 0);
	}
	if (pCameraManager == nullptr)
	{
		return;
	}

	const float fFov = vars::fFieldOfview;

	pPlayerController->FOV(fFov);
	pCameraManager->DefaultFOV = fFov;
	pCameraManager->ViewTarget.POV.FOV = fFov;
	pCameraManager->ViewTarget.POV.DesiredFOV = fFov;
	pCameraManager->CameraCachePrivate.POV.FOV = fFov;
	pCameraManager->CameraCachePrivate.POV.DesiredFOV = fFov;
	pCameraManager->LastFrameCameraCachePrivate.POV.FOV = fFov;
	pCameraManager->LastFrameCameraCachePrivate.POV.DesiredFOV = fFov;
}
