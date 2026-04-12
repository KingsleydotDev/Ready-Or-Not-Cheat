#include "gui.hpp"
#include "modules/visuals.hpp"
#include <stdexcept>

void gui::SetupMenu(IDXGISwapChain* swapChain) noexcept
{
	if (SUCCEEDED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device)))
	{
		device->GetImmediateContext(&context);
		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		window = desc.OutputWindow;

		ID3D11Texture2D* backBuffer;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
		backBuffer->Release();

		originalWindowProcess = reinterpret_cast<WNDPROC>(
			SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
		);

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(device, context);
		setup = true;
	}
}

void gui::Destroy() noexcept
{
	if (!setup)
		return;

	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	setup = false;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (renderTargetView)
	{
		renderTargetView->Release();
		renderTargetView = nullptr;
	}

	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (context)
	{
		context->Release();
		context = nullptr;
	}
}

void gui::Render() noexcept
{
	//title
	std::string dynamicTitle = MenuTitle + "###MenuID";

	// window size
	ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_FirstUseEver);

	//menu
	if (ImGui::Begin(dynamicTitle.c_str(), &open))
	{
		if (ImGui::BeginTabBar("##MainTabs"))
		{
			if (ImGui::BeginTabItem("Visuals"))
			{
				ImGui::SeparatorText("World ESP");
				ImGui::TextUnformatted("Draws on the D3D11 overlay each frame (same path as watermark).");
				ImGui::Checkbox("Enable world ESP", &Visuals::isModuleEnabled);
				ImGui::BeginDisabled(!Visuals::isModuleEnabled);
				ImGui::SeparatorText("Targets");
				ImGui::Checkbox("SWAT", &Visuals::isSwatLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isSwatLayerEnabled);
				ImGui::Checkbox("SWAT (Snap)", &Visuals::isSwatSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Suspect", &Visuals::isSuspectLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isSuspectLayerEnabled);
				ImGui::Checkbox("Suspect (Snap)", &Visuals::isSuspectSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Civilian", &Visuals::isCivilianLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isCivilianLayerEnabled);
				ImGui::Checkbox("Civilian (Snap)", &Visuals::isCivilianSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Sniper", &Visuals::isSniperLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isSniperLayerEnabled);
				ImGui::Checkbox("Sniper (Snap)", &Visuals::isSniperSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Base item (floor)", &Visuals::isBaseItemLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isBaseItemLayerEnabled);
				ImGui::Checkbox("Base item (Snap)", &Visuals::isBaseItemSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Pickup weapon", &Visuals::isPickupWeaponLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isPickupWeaponLayerEnabled);
				ImGui::Checkbox("Pickup weapon (Snap)", &Visuals::isPickupWeaponSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Door", &Visuals::isDoorLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isDoorLayerEnabled);
				ImGui::Checkbox("Door (Snap)", &Visuals::isDoorSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Evidence pickup", &Visuals::isEvidenceLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isEvidenceLayerEnabled);
				ImGui::Checkbox("Evidence pickup (Snap)", &Visuals::isEvidenceSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::Checkbox("Magazine pickup", &Visuals::isMagazinePickupLayerEnabled);
				ImGui::Indent();
				ImGui::BeginDisabled(!Visuals::isMagazinePickupLayerEnabled);
				ImGui::Checkbox("Magazine pickup (Snap)", &Visuals::isMagazinePickupSnaplineEnabled);
				ImGui::EndDisabled();
				ImGui::Unindent();
				ImGui::SeparatorText("Drawing");
				ImGui::Checkbox("3D box", &Visuals::isBox3dEnabled);
				ImGui::Checkbox("Gradient fill (box faces)", &Visuals::isGradientFillEnabled);
				ImGui::Checkbox("Labels", &Visuals::isEntityLabelEnabled);
				ImGui::SliderFloat("Box line thickness", &Visuals::fBoxLineThickness, 0.5f, 6.0f, "%.1f");
				ImGui::SliderFloat("Snapline thickness", &Visuals::fSnaplineThickness, 0.5f, 6.0f, "%.1f");
				ImGui::SeparatorText("Fill gradient");
				ImGui::ColorEdit4("Fill top", reinterpret_cast<float*>(&Visuals::colorFillTop), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Fill bottom", reinterpret_cast<float*>(&Visuals::colorFillBottom), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::SeparatorText("Outline colors");
				ImGui::ColorEdit4("SWAT", reinterpret_cast<float*>(&Visuals::colorSwat), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Suspect", reinterpret_cast<float*>(&Visuals::colorSuspect), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Civilian", reinterpret_cast<float*>(&Visuals::colorCivilian), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Sniper", reinterpret_cast<float*>(&Visuals::colorSniper), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Base item", reinterpret_cast<float*>(&Visuals::colorBaseItem), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Pickup weapon", reinterpret_cast<float*>(&Visuals::colorPickupWeapon), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Door", reinterpret_cast<float*>(&Visuals::colorDoor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Evidence", reinterpret_cast<float*>(&Visuals::colorEvidence), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::ColorEdit4("Magazine pickup", reinterpret_cast<float*>(&Visuals::colorMagazinePickup), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
				ImGui::EndDisabled();

				ImGui::SeparatorText("Camera");
				ImGui::Checkbox("Fullbright", &vars::bPerfViewmodeUnlit);
				ImGui::Checkbox("Enable FOV", &vars::bEnableFOV);
				ImGui::BeginDisabled(!vars::bEnableFOV);
				ImGui::SliderFloat("Field of view", &vars::fFieldOfview, 60.0f, 140.0f, "%.1f");
				ImGui::EndDisabled();
				ImGui::Checkbox("Night vision", &vars::bEnableNightVision);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Movement"))
			{
				ImGui::Checkbox("Enable Walk Speed", &vars::bEnableWalkSpeed);
				ImGui::BeginDisabled(!vars::bEnableWalkSpeed);
				ImGui::SliderFloat("Walk Speed", &vars::fWalkSpeed, 50.0f, 10500.0f, "%.0f");
				ImGui::SliderFloat("Crouch Walk Speed", &vars::fCrouchWalkSpeed, 50.0f, 10000.0f, "%.0f");
				ImGui::SliderFloat("Run Speed", &vars::fRunSpeed, 50.0f, 20000.0f, "%.0f");
				ImGui::EndDisabled();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Performance"))
			{
				ImGui::TextUnformatted("Each toggle runs a console command when a match is loaded. Turning off sends a restore command (defaults are typical UE values, not your original menu settings).");
				ImGui::SeparatorText("Lighting and shadows");
				ImGui::Checkbox("Fullbright (unlit viewmode)", &vars::bPerfViewmodeUnlit);
				ImGui::Checkbox("Shadow quality minimum", &vars::bPerfShadowQualityLow);
				ImGui::Checkbox("Volumetric fog off", &vars::bPerfVolFogOff);
				ImGui::Checkbox("Distance field shadowing off", &vars::bPerfDistanceFieldShadowOff);
				ImGui::Checkbox("Ray tracing off", &vars::bPerfRayTracingOff);
				ImGui::Checkbox("Lumen reflections off", &vars::bPerfLumenReflectionsOff);
				ImGui::Checkbox("Lumen diffuse indirect off", &vars::bPerfLumenDiffuseOff);
				ImGui::Checkbox("Light shafts off", &vars::bPerfLightShaftsOff);
				ImGui::Checkbox("Light shaft quality minimum", &vars::bPerfLightShaftQualityLow);
				ImGui::SeparatorText("Post process");
				ImGui::Checkbox("Post process AA minimum", &vars::bPerfPostProcessAaLow);
				ImGui::Checkbox("Motion blur off", &vars::bPerfMotionBlurOff);
				ImGui::Checkbox("Depth of field off", &vars::bPerfDofOff);
				ImGui::Checkbox("Bloom off", &vars::bPerfBloomOff);
				ImGui::Checkbox("Lens flare off", &vars::bPerfLensFlareOff);
				ImGui::Checkbox("Film grain off", &vars::bPerfFilmGrainOff);
				ImGui::Checkbox("Scene color fringe off", &vars::bPerfSceneColorFringeOff);
				ImGui::Checkbox("Eye adaptation off", &vars::bPerfEyeAdaptationOff);
				ImGui::SeparatorText("World detail");
				ImGui::Checkbox("View distance scale aggressive", &vars::bPerfViewDistanceAggressive);
				ImGui::Checkbox("Skeletal mesh LOD bias high", &vars::bPerfSkeletalMeshLodBias);
				ImGui::Checkbox("Static mesh LOD distance scale high", &vars::bPerfStaticMeshLodScale);
				ImGui::Checkbox("Max anisotropy minimum", &vars::bPerfMaxAnisoOff);
				ImGui::Checkbox("Material quality low", &vars::bPerfMaterialQualityLow);
				ImGui::Checkbox("Foliage density off", &vars::bPerfFoliageDensityOff);
				ImGui::SeparatorText("Decals and particles");
				ImGui::Checkbox("Decal force delete", &vars::bPerfDecalForceDelete);
				ImGui::Checkbox("CPU particles per emitter minimum", &vars::bPerfCpuParticlesOff);
				ImGui::Checkbox("GPU particles spawned per frame minimum", &vars::bPerfGpuParticlesOff);
				ImGui::Checkbox("Refraction quality off", &vars::bPerfRefractionOff);
				ImGui::SeparatorText("Debug overlay");
				ImGui::Checkbox("Stat fps", &vars::bPerfStatFps);
				ImGui::Checkbox("Stat unit", &vars::bPerfStatUnit);
				ImGui::TextUnformatted("Changing stat options clears overlays then reapplies; stat none is used when both are off.");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{
			    ImGui::Checkbox("God Mode", &vars::bEnableGodMode);
				ImGui::Separator();
				if (ImGui::Button("Force Start (Ready All)"))
					vars::bEnableForceStart = true;
				if (ImGui::Button("Insta Start"))
					vars::bEnableInstaStart = true;
				ImGui::Separator();
				ImGui::Checkbox("AI Scale", &vars::bEnableAIScale);
				ImGui::BeginDisabled(!vars::bEnableAIScale);
				ImGui::SliderFloat("Scale", &vars::fAIScale, 0.5f, 5.0f, "%.1f");
				ImGui::EndDisabled();
				ImGui::Separator();
				ImGui::SliderInt("Civ Spawn Count", &vars::iCivSpawnCount, 1, 50);
				if (ImGui::Button("Spawn Civilians"))
					vars::bSpawnCivs = true;
				ImGui::Separator();
				ImGui::Checkbox("Watermark", &vars::bEnableWatermark);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
)
{
	if (!gui::setup)
		return CallWindowProc(
			gui::originalWindowProcess,
			window,
			message,
			wideParam,
			longParam
		);

	if (GetAsyncKeyState(VK_HOME) & 1) {
		gui::open = !gui::open;
	}
	if (gui::open)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = true;
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = false;
	}

	if (gui::open && ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wideParam,
		longParam
	)) return 1L;

	return CallWindowProc(
		gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}

void gui::Setup()
{
	// This is now handled by the hook
}
