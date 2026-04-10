#pragma once

#include "../SDK/CoreUObject_structs.hpp"

namespace SDK
{
	class AHUD;
}

class Visuals
{
public:
	static inline bool isModuleEnabled = false;

	static inline bool isSwatLayerEnabled = true;
	static inline bool isSuspectLayerEnabled = true;
	static inline bool isCivilianLayerEnabled = true;
	static inline bool isSniperLayerEnabled = true;
	static inline bool isBaseItemLayerEnabled = true;
	static inline bool isPickupWeaponLayerEnabled = true;
	static inline bool isDoorLayerEnabled = false;
	static inline bool isEvidenceLayerEnabled = false;
	static inline bool isMagazinePickupLayerEnabled = false;

	static inline bool isSwatSnaplineEnabled = false;
	static inline bool isSuspectSnaplineEnabled = false;
	static inline bool isCivilianSnaplineEnabled = false;
	static inline bool isSniperSnaplineEnabled = false;
	static inline bool isBaseItemSnaplineEnabled = false;
	static inline bool isPickupWeaponSnaplineEnabled = false;
	static inline bool isDoorSnaplineEnabled = false;
	static inline bool isEvidenceSnaplineEnabled = false;
	static inline bool isMagazinePickupSnaplineEnabled = false;

	static inline bool isBox3dEnabled = true;
	static inline bool isGradientFillEnabled = false;
	static inline bool isEntityLabelEnabled = true;

	static inline float fBoxLineThickness = 1.5f;
	static inline float fSnaplineThickness = 1.25f;

	static inline SDK::FLinearColor colorSwat = { 0.2f, 0.55f, 1.0f, 1.0f };
	static inline SDK::FLinearColor colorSuspect = { 1.0f, 0.25f, 0.15f, 1.0f };
	static inline SDK::FLinearColor colorCivilian = { 0.35f, 0.95f, 0.4f, 1.0f };
	static inline SDK::FLinearColor colorSniper = { 0.95f, 0.85f, 0.2f, 1.0f };
	static inline SDK::FLinearColor colorBaseItem = { 0.75f, 0.55f, 1.0f, 1.0f };
	static inline SDK::FLinearColor colorPickupWeapon = { 1.0f, 0.6f, 0.15f, 1.0f };
	static inline SDK::FLinearColor colorDoor = { 0.45f, 0.85f, 0.95f, 1.0f };
	static inline SDK::FLinearColor colorEvidence = { 0.95f, 0.9f, 0.35f, 1.0f };
	static inline SDK::FLinearColor colorMagazinePickup = { 0.55f, 0.95f, 0.65f, 1.0f };

	static inline SDK::FLinearColor colorFillTop = { 0.08f, 0.08f, 0.12f, 0.45f };
	static inline SDK::FLinearColor colorFillBottom = { 0.25f, 0.05f, 0.05f, 0.55f };

	static void RenderReceiveDrawHud(SDK::AHUD* hud) noexcept;
	static void RenderPresentOverlay() noexcept;
};
