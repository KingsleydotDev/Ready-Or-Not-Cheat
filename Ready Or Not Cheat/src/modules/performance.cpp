#include "performance.hpp"
#include "moduleruntime.hpp"
#include "../vars.h"
#include "../SDK/Basic.hpp"
#include "../SDK/Engine_classes.hpp"
#include <array>
#include <chrono>
#include <thread>

namespace
{
	std::thread threadWorker;

	struct PerfToggleRow
	{
		bool* pWantOn;
		const wchar_t* onCommand;
		const wchar_t* offCommand;
		bool bApplied;
	};

	std::array<PerfToggleRow, 27> toggleTable{ {
		{ &vars::bPerfViewmodeUnlit, L"viewmode unlit", L"viewmode lit", false },
		{ &vars::bPerfShadowQualityLow, L"r.ShadowQuality 0", L"r.ShadowQuality 3", false },
		{ &vars::bPerfVolFogOff, L"r.VolumetricFog 0", L"r.VolumetricFog 1", false },
		{ &vars::bPerfPostProcessAaLow, L"r.PostProcessAAQuality 0", L"r.PostProcessAAQuality 4", false },
		{ &vars::bPerfDecalForceDelete, L"r.Decal.ForceDelete 1", L"r.Decal.ForceDelete 0", false },
		{ &vars::bPerfCpuParticlesOff, L"fx.MaxCPUParticlesPerEmitter 0", L"fx.MaxCPUParticlesPerEmitter 50", false },
		{ &vars::bPerfDistanceFieldShadowOff, L"r.DistanceFieldShadowing 0", L"r.DistanceFieldShadowing 1", false },
		{ &vars::bPerfRayTracingOff, L"r.RayTracing 0", L"r.RayTracing 1", false },
		{ &vars::bPerfLumenReflectionsOff, L"r.Lumen.Reflections.Allow 0", L"r.Lumen.Reflections.Allow 1", false },
		{ &vars::bPerfLumenDiffuseOff, L"r.Lumen.DiffuseIndirect.Allow 0", L"r.Lumen.DiffuseIndirect.Allow 1", false },
		{ &vars::bPerfLightShaftsOff, L"r.LightShafts 0", L"r.LightShafts 1", false },
		{ &vars::bPerfLightShaftQualityLow, L"r.LightShaftQuality 0", L"r.LightShaftQuality 4", false },
		{ &vars::bPerfMotionBlurOff, L"r.MotionBlurQuality 0", L"r.MotionBlurQuality 4", false },
		{ &vars::bPerfDofOff, L"r.DepthOfFieldQuality 0", L"r.DepthOfFieldQuality 2", false },
		{ &vars::bPerfBloomOff, L"r.BloomQuality 0", L"r.BloomQuality 5", false },
		{ &vars::bPerfLensFlareOff, L"r.LensFlareQuality 0", L"r.LensFlareQuality 2", false },
		{ &vars::bPerfFilmGrainOff, L"r.Tonemapper.GrainQuantization 0", L"r.Tonemapper.GrainQuantization 1", false },
		{ &vars::bPerfSceneColorFringeOff, L"r.SceneColorFringeQuality 0", L"r.SceneColorFringeQuality 1", false },
		{ &vars::bPerfEyeAdaptationOff, L"r.EyeAdaptationQuality 0", L"r.EyeAdaptationQuality 2", false },
		{ &vars::bPerfViewDistanceAggressive, L"r.ViewDistanceScale 0.4", L"r.ViewDistanceScale 1", false },
		{ &vars::bPerfSkeletalMeshLodBias, L"r.SkeletalMeshLODBias 2", L"r.SkeletalMeshLODBias 0", false },
		{ &vars::bPerfStaticMeshLodScale, L"r.StaticMeshLODDistanceScale 2", L"r.StaticMeshLODDistanceScale 1", false },
		{ &vars::bPerfMaxAnisoOff, L"r.MaxAnisotropy 0", L"r.MaxAnisotropy 8", false },
		{ &vars::bPerfMaterialQualityLow, L"r.MaterialQualityLevel 0", L"r.MaterialQualityLevel 1", false },
		{ &vars::bPerfFoliageDensityOff, L"foliage.DensityScale 0", L"foliage.DensityScale 1", false },
		{ &vars::bPerfGpuParticlesOff, L"fx.MaxGPUParticlesSpawnedPerFrame 0", L"fx.MaxGPUParticlesSpawnedPerFrame 65536", false },
		{ &vars::bPerfRefractionOff, L"r.RefractionQuality 0", L"r.RefractionQuality 2", false },
	} };

	SDK::UWorld* pWorldLast = nullptr;
	SDK::UWorld* pWorldStatsLast = nullptr;
	bool bStatFpsPrev = false;
	bool bStatUnitPrev = false;

	void RunConsole(SDK::UWorld* pWorld, const wchar_t* wszCommand)
	{
		if (pWorld == nullptr || wszCommand == nullptr)
		{
			return;
		}
		SDK::UKismetSystemLibrary::ExecuteConsoleCommand(pWorld, SDK::FString(wszCommand), nullptr);
	}

	void SyncStatOverlay(SDK::UWorld* pWorld)
	{
		if (pWorld == nullptr)
		{
			return;
		}

		const bool bWantFps = vars::bPerfStatFps;
		const bool bWantUnit = vars::bPerfStatUnit;

		if (pWorld != pWorldStatsLast)
		{
			pWorldStatsLast = pWorld;
			bStatFpsPrev = !bWantFps;
			bStatUnitPrev = !bWantUnit;
		}

		if (bWantFps == bStatFpsPrev && bWantUnit == bStatUnitPrev)
		{
			return;
		}

		RunConsole(pWorld, L"stat none");
		if (bWantFps)
		{
			RunConsole(pWorld, L"stat fps");
		}
		if (bWantUnit)
		{
			RunConsole(pWorld, L"stat unit");
		}
		bStatFpsPrev = bWantFps;
		bStatUnitPrev = bWantUnit;
	}

	void SyncToggles(SDK::UWorld* pWorld)
	{
		if (pWorld == nullptr)
		{
			return;
		}

		if (pWorld != pWorldLast)
		{
			pWorldLast = pWorld;
			for (PerfToggleRow& row : toggleTable)
			{
				row.bApplied = false;
			}
		}

		for (PerfToggleRow& row : toggleTable)
		{
			const bool bWant = *row.pWantOn;
			if (bWant && !row.bApplied)
			{
				RunConsole(pWorld, row.onCommand);
				row.bApplied = true;
			}
			else if (!bWant && row.bApplied)
			{
				RunConsole(pWorld, row.offCommand);
				row.bApplied = false;
			}
		}
	}

	void RunWorker()
	{
		while (moduleruntime::WorkersActive())
		{
			SDK::UWorld* pWorld = SDK::UWorld::GetWorld();
			if (pWorld == nullptr)
			{
				pWorldLast = nullptr;
				pWorldStatsLast = nullptr;
				bStatFpsPrev = !vars::bPerfStatFps;
				bStatUnitPrev = !vars::bPerfStatUnit;
			}
			else
			{
				SyncToggles(pWorld);
				SyncStatOverlay(pWorld);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(120));
		}
	}
}

void performance::Start()
{
	if (threadWorker.joinable())
	{
		return;
	}
	threadWorker = std::thread(RunWorker);
}

void performance::Stop()
{
	if (threadWorker.joinable())
	{
		threadWorker.join();
	}
}
