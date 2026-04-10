#pragma once
#include <d3d11.h>
#include <string>
#include "vars.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx11.h"

namespace gui {
	inline std::string MenuTitle = "Ready Or Not by Kingsley & Dismay";

	inline void UpdateMenuTitle() {
		MenuTitle = "Ready Or Not by Kingsley & Dismay";
	}
}

// menu size
#define WIDTH 790
#define HEIGHT 578

namespace gui
{
	// Show menu?
	inline bool open = true;

	// is it setup?
	inline bool setup = false;

	inline HWND window = nullptr;
	inline WNDPROC originalWindowProcess = nullptr;

	// dx11
	inline ID3D11Device* device = nullptr;
	inline ID3D11DeviceContext* context = nullptr;
	inline ID3D11RenderTargetView* renderTargetView = nullptr;

	// Setup Device
	void Setup();

	void SetupMenu(IDXGISwapChain* swapChain) noexcept;
	void Destroy() noexcept;

	void Render() noexcept;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);