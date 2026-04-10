#include "gui.hpp"
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
				ImGui::Checkbox("EnableFOV", &vars::bEnableFOV);
				ImGui::BeginDisabled(!vars::bEnableFOV);
				ImGui::SliderFloat("FieldOfView", &vars::fFieldOfview, 60.0f, 140.0f, "%.1f");
				ImGui::EndDisabled();
				ImGui::Checkbox("NightVision", &vars::bEnableNightVision);
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
