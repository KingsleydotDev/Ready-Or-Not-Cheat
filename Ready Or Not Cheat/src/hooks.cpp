#include "hooks.hpp"
#include "modules/visuals.hpp"
#include "modules/watermark.hpp"
#include <atomic>
#include <stdexcept>

#include <Windows.h>

namespace
{
	std::atomic<bool> bUnloadPending{ false };
	std::atomic<bool> bMinHookTornDown{ false };
	void* pPresentHookTarget = nullptr;
	HANDLE hUnloadDoneEvent = nullptr;

	void RemovePresentHookIfInstalled() noexcept
	{
		if (pPresentHookTarget)
		{
			MH_DisableHook(pPresentHookTarget);
			MH_RemoveHook(pPresentHookTarget);
			pPresentHookTarget = nullptr;
		}
	}
}

void hooks::Setup()
{
	bUnloadPending.store(false, std::memory_order_release);
	bMinHookTornDown.store(false, std::memory_order_release);
	if (hUnloadDoneEvent)
	{
		CloseHandle(hUnloadDoneEvent);
		hUnloadDoneEvent = nullptr;
	}
	pPresentHookTarget = nullptr;

	MH_STATUS eInit = MH_Initialize();
	if (eInit == MH_ERROR_ALREADY_INITIALIZED)
	{
		MH_Uninitialize();
		eInit = MH_Initialize();
	}
	if (eInit != MH_OK)
		throw std::runtime_error("Unable to initialize minhook");

	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	auto featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		NULL,
		&context)))
	{
		throw std::runtime_error("Failed to create D3D11 device and swapchain.");
	}

	void** VTable = *reinterpret_cast<void***>(swapChain);
	void* pPresent = VTable[8];

	if (MH_CreateHook(
		pPresent,
		&Present,
		reinterpret_cast<void**>(&PresentOriginal)
	)) throw std::runtime_error("Unable to hook Present()");

	pPresentHookTarget = pPresent;

	if (MH_EnableHook(MH_ALL_HOOKS))
		throw std::runtime_error("Unable to enable hooks");

	hUnloadDoneEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
	if (!hUnloadDoneEvent)
		throw std::runtime_error("CreateEvent failed");

	device->Release();
	context->Release();
	swapChain->Release();
}

void hooks::BeginUnloadWait() noexcept
{
	bUnloadPending.store(true, std::memory_order_release);
	if (!hUnloadDoneEvent)
		return;
	WaitForSingleObject(hUnloadDoneEvent, INFINITE);
}

bool hooks::MinHookTornDownAfterPresent() noexcept
{
	return bMinHookTornDown.load(std::memory_order_acquire);
}

void hooks::Destroy(bool bHookSessionReady, bool bMinHookTornDownOnPresent) noexcept
{
	if (hUnloadDoneEvent)
	{
		CloseHandle(hUnloadDoneEvent);
		hUnloadDoneEvent = nullptr;
	}
	if (bMinHookTornDownOnPresent)
		return;
	if (!bHookSessionReady)
	{
		if (pPresentHookTarget)
			RemovePresentHookIfInstalled();
		else
			MH_DisableHook(MH_ALL_HOOKS);
		if (!bMinHookTornDown.exchange(true))
			MH_Uninitialize();
	}
}

long __stdcall hooks::Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) noexcept
{
	if (bUnloadPending.load(std::memory_order_acquire))
	{
		const long lResult = PresentOriginal(swapChain, syncInterval, flags);
		if (!bMinHookTornDown.exchange(true))
		{
			RemovePresentHookIfInstalled();
			MH_Uninitialize();
		}
		if (hUnloadDoneEvent)
			SetEvent(hUnloadDoneEvent);
		return lResult;
	}

	if (!gui::setup)
		gui::SetupMenu(swapChain);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (gui::setup)
	{
		if (Visuals::isModuleEnabled)
			Visuals::RenderPresentOverlay();
		watermark::Render();
	}

	if (gui::open)
		gui::Render();

	ImGui::EndFrame();
	ImGui::Render();

	gui::context->OMSetRenderTargets(1, &gui::renderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return PresentOriginal(swapChain, syncInterval, flags);
}
