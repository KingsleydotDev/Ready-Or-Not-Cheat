#pragma once
#include "gui.hpp"
#include <stdexcept>
#include <intrin.h>

#include "..//ext/minhook/minhook.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx11.h"

namespace hooks
{
	void Setup();
	void BeginUnloadWait() noexcept;
	bool MinHookTornDownAfterPresent() noexcept;
	void Destroy(bool bHookSessionReady, bool bMinHookTornDownOnPresent) noexcept;

	// Helper to get Virtual Function address
	constexpr void* VirtualFunction(void* thisptr, size_t index) noexcept
	{
		return (*static_cast<void***>(thisptr))[index];
	}

	// Present
	using PresentFn = long(__stdcall*)(IDXGISwapChain*, UINT, UINT);
	inline PresentFn PresentOriginal = nullptr;
	long __stdcall Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) noexcept;
}