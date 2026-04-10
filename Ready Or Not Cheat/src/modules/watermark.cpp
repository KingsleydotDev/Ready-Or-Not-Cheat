#include "watermark.hpp"
#include "../vars.h"
#include "../../ext/imgui/imgui.h"
#include <cmath>
#include <cstdio>
#include <ctime>

void watermark::Render() noexcept
{
	if (!vars::bEnableWatermark)
	{
		return;
	}

	time_t rawTime = 0;
	time(&rawTime);
	struct tm timeInfo {};
	localtime_s(&timeInfo, &rawTime);
	char timeBuffer[80];
	strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);

	char const* redText = "[DEV BUILD]";
	char middleBuffer[200];
	sprintf_s(
		middleBuffer,
		sizeof(middleBuffer),
		" | %d FPS | %s | Ready Or Not | By ",
		static_cast<int>(ImGui::GetIO().Framerate),
		timeBuffer);

	char const* kingsleyText = "Kingsley";
	char const* ampersandText = " & ";
	char const* dismayText = "Dismay";

	ImVec2 redSize = ImGui::CalcTextSize(redText);
	ImVec2 midSize = ImGui::CalcTextSize(middleBuffer);
	ImVec2 kingsleySize = ImGui::CalcTextSize(kingsleyText);
	ImVec2 ampersandSize = ImGui::CalcTextSize(ampersandText);
	ImVec2 dismaySize = ImGui::CalcTextSize(dismayText);

	float totalWidth = redSize.x + midSize.x + kingsleySize.x + ampersandSize.x + dismaySize.x;
	float padding = 5.0f;
	float x = ImGui::GetIO().DisplaySize.x - totalWidth - 15.0f;
	float y = 15.0f;

	float t = static_cast<float>(ImGui::GetTime());
	ImU32 rainbowColor = static_cast<ImU32>(ImColor::HSV(t * 0.5f, 1.0f, 1.0f));

	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	if (drawList == nullptr)
	{
		return;
	}

	drawList->AddRectFilled(
		ImVec2(x - padding, y - padding),
		ImVec2(x + totalWidth + padding, y + redSize.y + padding),
		IM_COL32(0, 0, 0, 200),
		4.0f);

	drawList->AddText(ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), redText);
	drawList->AddText(ImVec2(x, y), IM_COL32(255, 0, 0, 255), redText);

	float xMid = x + redSize.x;
	drawList->AddText(ImVec2(xMid + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), middleBuffer);
	drawList->AddText(ImVec2(xMid, y), IM_COL32(255, 255, 255, 255), middleBuffer);

	float xKingsley = xMid + midSize.x;
	drawList->AddText(ImVec2(xKingsley + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), kingsleyText);
	drawList->AddText(ImVec2(xKingsley, y), rainbowColor, kingsleyText);

	float xAmp = xKingsley + kingsleySize.x;
	drawList->AddText(ImVec2(xAmp + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), ampersandText);
	drawList->AddText(ImVec2(xAmp, y), IM_COL32(255, 255, 255, 255), ampersandText);

	float xDismay = xAmp + ampersandSize.x;
	float pulse = std::sin(t * 3.0f) * 0.5f + 0.5f;
	int dismayR = static_cast<int>(pulse * 255.0f);
	ImU32 dismayColor = IM_COL32(dismayR, 0, 0, 255);
	drawList->AddText(ImVec2(xDismay + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 255), dismayText);
	drawList->AddText(ImVec2(xDismay, y), dismayColor, dismayText);
}
