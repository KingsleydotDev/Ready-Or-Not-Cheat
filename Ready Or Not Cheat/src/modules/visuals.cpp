#include "visuals.hpp"
#include "../../ext/imgui/imgui.h"
#include "../SDK/Basic.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/CoreUObject_structs.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/ReadyOrNot_classes.hpp"
#include "../SDK/ReadyOrNot_structs.hpp"
#include <string>

namespace
{
	struct HudCanvasAccess : SDK::AHUD
	{
		static SDK::UCanvas* ResolveCanvas(SDK::AHUD* hud) noexcept
		{
			return static_cast<HudCanvasAccess*>(hud)->Canvas;
		}
	};

	SDK::FLinearColor LerpLinearColors(SDK::FLinearColor const& from, SDK::FLinearColor const& to, float t) noexcept
	{
		SDK::FLinearColor result;
		result.R = from.R + (to.R - from.R) * t;
		result.G = from.G + (to.G - from.G) * t;
		result.B = from.B + (to.B - from.B) * t;
		result.A = from.A + (to.A - from.A) * t;
		return result;
	}

	ImU32 ToImU32(SDK::FLinearColor const& color) noexcept
	{
		auto toByte = [](float channel) {
			int value = static_cast<int>(channel * 255.0f + 0.5f);
			if (value < 0)
			{
				value = 0;
			}
			if (value > 255)
			{
				value = 255;
			}
			return value;
		};
		return IM_COL32(toByte(color.R), toByte(color.G), toByte(color.B), toByte(color.A));
	}

	float DotWorld(SDK::FVector const& a, SDK::FVector const& b) noexcept
	{
		return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
	}

	SDK::FVector CrossWorld(SDK::FVector const& a, SDK::FVector const& b) noexcept
	{
		SDK::FVector r;
		r.X = a.Y * b.Z - a.Z * b.Y;
		r.Y = a.Z * b.X - a.X * b.Z;
		r.Z = a.X * b.Y - a.Y * b.X;
		return r;
	}

	bool IsScreenPositionStable(float px, float py, float viewW, float viewH) noexcept
	{
		float const margin = 720.0f;
		if (!(px == px) || !(py == py))
		{
			return false;
		}
		return px >= -margin && px <= viewW + margin && py >= -margin && py <= viewH + margin;
	}

	void DrawBox3dFaceFillsImGui(
		ImDrawList* drawList,
		bool const* projected,
		SDK::FVector2D const* screen,
		SDK::FVector const* cornersWorld,
		float viewW,
		float viewH,
		SDK::FVector const& cameraWorld,
		float globalMinY,
		float globalMaxY,
		SDK::FLinearColor const& topColor,
		SDK::FLinearColor const& bottomColor) noexcept
	{
		int const faces[6][4] = {
			{ 0, 4, 6, 2 },
			{ 1, 3, 7, 5 },
			{ 0, 1, 5, 4 },
			{ 2, 6, 7, 3 },
			{ 0, 2, 3, 1 },
			{ 4, 5, 7, 6 },
		};
		float const spanY = globalMaxY - globalMinY;
		float const invSpanY = (spanY > 2.0f) ? (1.0f / spanY) : 0.0f;

		struct FaceSort
		{
			int idx;
			float depth2;
		};
		FaceSort order[6];
		int orderCount = 0;

		for (int f = 0; f < 6; ++f)
		{
			int const ia = faces[f][0];
			int const ib = faces[f][1];
			int const ic = faces[f][2];
			int const id = faces[f][3];
			if (!projected[ia] || !projected[ib] || !projected[ic] || !projected[id])
			{
				continue;
			}
			if (!IsScreenPositionStable(screen[ia].X, screen[ia].Y, viewW, viewH)
				|| !IsScreenPositionStable(screen[ib].X, screen[ib].Y, viewW, viewH)
				|| !IsScreenPositionStable(screen[ic].X, screen[ic].Y, viewW, viewH)
				|| !IsScreenPositionStable(screen[id].X, screen[id].Y, viewW, viewH))
			{
				continue;
			}

			SDK::FVector const wa = cornersWorld[ia];
			SDK::FVector const wb = cornersWorld[ib];
			SDK::FVector const wc = cornersWorld[ic];
			SDK::FVector const wd = cornersWorld[id];
			SDK::FVector e1 = wb;
			e1.X -= wa.X;
			e1.Y -= wa.Y;
			e1.Z -= wa.Z;
			SDK::FVector e2 = wc;
			e2.X -= wa.X;
			e2.Y -= wa.Y;
			e2.Z -= wa.Z;
			SDK::FVector normal = CrossWorld(e1, e2);
			SDK::FVector center;
			center.X = (wa.X + wb.X + wc.X + wd.X) * 0.25f;
			center.Y = (wa.Y + wb.Y + wc.Y + wd.Y) * 0.25f;
			center.Z = (wa.Z + wb.Z + wc.Z + wd.Z) * 0.25f;

			SDK::FVector viewToCam = cameraWorld;
			viewToCam.X -= center.X;
			viewToCam.Y -= center.Y;
			viewToCam.Z -= center.Z;
			if (DotWorld(normal, viewToCam) <= 0.0f)
			{
				continue;
			}

			float dist2 = viewToCam.X * viewToCam.X + viewToCam.Y * viewToCam.Y + viewToCam.Z * viewToCam.Z;
			order[orderCount].idx = f;
			order[orderCount].depth2 = dist2;
			++orderCount;
		}

		for (int i = 1; i < orderCount; ++i)
		{
			FaceSort key = order[i];
			int j = i - 1;
			while (j >= 0 && order[j].depth2 < key.depth2)
			{
				order[j + 1] = order[j];
				--j;
			}
			order[j + 1] = key;
		}

		for (int o = 0; o < orderCount; ++o)
		{
			int const f = order[o].idx;
			int const ia = faces[f][0];
			int const ib = faces[f][1];
			int const ic = faces[f][2];
			int const id = faces[f][3];
			float avgY = screen[ia].Y + screen[ib].Y + screen[ic].Y + screen[id].Y;
			avgY *= 0.25f;
			float t = (spanY > 2.0f) ? (avgY - globalMinY) * invSpanY : 0.5f;
			if (t < 0.0f)
			{
				t = 0.0f;
			}
			if (t > 1.0f)
			{
				t = 1.0f;
			}
			SDK::FLinearColor fillColor = LerpLinearColors(topColor, bottomColor, t);
			ImU32 col = ToImU32(fillColor);
			ImVec2 const p1(screen[ia].X, screen[ia].Y);
			ImVec2 const p2(screen[ib].X, screen[ib].Y);
			ImVec2 const p3(screen[ic].X, screen[ic].Y);
			ImVec2 const p4(screen[id].X, screen[id].Y);
			drawList->AddQuadFilled(p1, p2, p3, p4, col);
		}
	}

	void DrawBox3dEdgesImGui(
		ImDrawList* drawList,
		bool const* projected,
		SDK::FVector2D const* screen,
		SDK::FLinearColor const& color,
		float thickness) noexcept
	{
		int const edges[12][2] = {
			{ 0, 1 },
			{ 2, 3 },
			{ 4, 5 },
			{ 6, 7 },
			{ 0, 2 },
			{ 1, 3 },
			{ 4, 6 },
			{ 5, 7 },
			{ 0, 4 },
			{ 1, 5 },
			{ 2, 6 },
			{ 3, 7 },
		};
		ImU32 const argb = ToImU32(color);
		for (int e = 0; e < 12; ++e)
		{
			int a = edges[e][0];
			int b = edges[e][1];
			if (!projected[a] || !projected[b])
			{
				continue;
			}
			drawList->AddLine(
				ImVec2(screen[a].X, screen[a].Y),
				ImVec2(screen[b].X, screen[b].Y),
				argb,
				thickness);
		}
	}

	void BuildWorldCornersFromActorBounds(SDK::AActor* actor, SDK::FVector* corners8) noexcept
	{
		SDK::FVector origin;
		SDK::FVector extent;
		actor->GetActorBounds(false, &origin, &extent, false);
		float maxExtent = extent.X;
		if (extent.Y > maxExtent)
		{
			maxExtent = extent.Y;
		}
		if (extent.Z > maxExtent)
		{
			maxExtent = extent.Z;
		}
		float const minUsefulExtent = 18.0f;
		if (maxExtent < minUsefulExtent)
		{
			SDK::FVector loc = actor->K2_GetActorLocation();
			origin = loc;
			extent.X = 35.0f;
			extent.Y = 25.0f;
			extent.Z = 12.0f;
		}

		for (int i = 0; i < 8; ++i)
		{
			float sx = (i & 1) ? extent.X : -extent.X;
			float sy = (i & 2) ? extent.Y : -extent.Y;
			float sz = (i & 4) ? extent.Z : -extent.Z;
			corners8[i].X = origin.X + sx;
			corners8[i].Y = origin.Y + sy;
			corners8[i].Z = origin.Z + sz;
		}
	}

	void BuildWorldCornersFromCharacterCapsule(SDK::ACharacter* ch, SDK::FVector* corners8) noexcept
	{
		SDK::UCapsuleComponent* cap = ch->CapsuleComponent;
		if (!cap)
		{
			BuildWorldCornersFromActorBounds(ch, corners8);
			return;
		}
		float r = cap->GetScaledCapsuleRadius();
		float hh = cap->GetScaledCapsuleHalfHeight();
		if (r < 1.0f || hh < 1.0f)
		{
			BuildWorldCornersFromActorBounds(ch, corners8);
			return;
		}
		SDK::FTransform xform = cap->K2_GetComponentToWorld();
		for (int i = 0; i < 8; ++i)
		{
			float sx = (i & 1) ? r : -r;
			float sy = (i & 2) ? r : -r;
			float sz = (i & 4) ? hh : -hh;
			SDK::FVector local;
			local.X = sx;
			local.Y = sy;
			local.Z = sz;
			SDK::FVector rotated = SDK::UKismetMathLibrary::Quat_RotateVector(xform.Rotation, local);
			corners8[i].X = rotated.X + xform.Translation.X;
			corners8[i].Y = rotated.Y + xform.Translation.Y;
			corners8[i].Z = rotated.Z + xform.Translation.Z;
		}
	}

	void BuildWorldCornersForEsp(SDK::AActor* actor, SDK::FVector* corners8) noexcept
	{
		if (actor->IsA(SDK::ACharacter::StaticClass()))
		{
			BuildWorldCornersFromCharacterCapsule(static_cast<SDK::ACharacter*>(actor), corners8);
			return;
		}
		BuildWorldCornersFromActorBounds(actor, corners8);
	}

	void NormalizeScreenRect(float* minX, float* minY, float* maxX, float* maxY) noexcept
	{
		if (*minX > *maxX)
		{
			float t = *minX;
			*minX = *maxX;
			*maxX = t;
		}
		if (*minY > *maxY)
		{
			float t = *minY;
			*minY = *maxY;
			*maxY = t;
		}
	}

	bool TryProjectCorners(
		SDK::APlayerController* playerController,
		SDK::FVector const* cornersWorld,
		bool* outProjected,
		SDK::FVector2D* outScreen,
		int cornerCount,
		bool usePlayerViewportRelative) noexcept
	{
		bool hasAny = false;
		for (int i = 0; i < cornerCount; ++i)
		{
			outProjected[i] = playerController->ProjectWorldLocationToScreen(
				cornersWorld[i],
				&outScreen[i],
				usePlayerViewportRelative);
			if (outProjected[i])
			{
				hasAny = true;
			}
		}
		return hasAny;
	}

	bool TryBuildScreenBounds(
		bool const* projected,
		SDK::FVector2D const* screen,
		int cornerCount,
		float* minX,
		float* minY,
		float* maxX,
		float* maxY,
		float viewW,
		float viewH) noexcept
	{
		bool has = false;
		float x0 = 0.0f;
		float y0 = 0.0f;
		float x1 = 0.0f;
		float y1 = 0.0f;
		for (int i = 0; i < cornerCount; ++i)
		{
			if (!projected[i])
			{
				continue;
			}
			float px = screen[i].X;
			float py = screen[i].Y;
			if (!IsScreenPositionStable(px, py, viewW, viewH))
			{
				continue;
			}
			if (!has)
			{
				x0 = px;
				y0 = py;
				x1 = px;
				y1 = py;
				has = true;
			}
			else
			{
				if (px < x0)
				{
					x0 = px;
				}
				if (px > x1)
				{
					x1 = px;
				}
				if (py < y0)
				{
					y0 = py;
				}
				if (py > y1)
				{
					y1 = py;
				}
			}
		}
		if (!has)
		{
			return false;
		}
		*minX = x0;
		*minY = y0;
		*maxX = x1;
		*maxY = y1;
		return true;
	}

	void DrawBottomCenterSnaplineImGui(
		ImDrawList* drawList,
		float viewW,
		float viewH,
		bool const* projected,
		SDK::FVector2D const* screen,
		SDK::FLinearColor const& color,
		float thickness) noexcept
	{
		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		if (!TryBuildScreenBounds(projected, screen, 8, &minX, &minY, &maxX, &maxY, viewW, viewH))
		{
			return;
		}
		ImU32 const argb = ToImU32(color);
		drawList->AddLine(
			ImVec2(viewW * 0.5f, viewH),
			ImVec2((minX + maxX) * 0.5f, maxY),
			argb,
			thickness);
	}

	void DrawCenteredLabelAboveImGui(
		ImDrawList* drawList,
		SDK::FString const& text,
		float centerX,
		float topY,
		SDK::FLinearColor const& color) noexcept
	{
		std::string const utf8 = text.ToString();
		if (utf8.empty())
		{
			return;
		}
		char const* cstr = utf8.c_str();
		ImVec2 const size = ImGui::CalcTextSize(cstr);
		float drawX = centerX - size.x * 0.5f;
		float drawY = topY - size.y - 4.0f;
		drawList->AddText(ImVec2(drawX + 1.0f, drawY + 1.0f), IM_COL32(0, 0, 0, 220), cstr);
		drawList->AddText(ImVec2(drawX, drawY), ToImU32(color), cstr);
	}

	void DrawBox3dEdges(
		SDK::UCanvas* canvas,
		bool const* projected,
		SDK::FVector2D const* screen,
		SDK::FLinearColor const& color,
		float thickness) noexcept
	{
		int const edges[12][2] = {
			{ 0, 1 },
			{ 2, 3 },
			{ 4, 5 },
			{ 6, 7 },
			{ 0, 2 },
			{ 1, 3 },
			{ 4, 6 },
			{ 5, 7 },
			{ 0, 4 },
			{ 1, 5 },
			{ 2, 6 },
			{ 3, 7 },
		};
		for (int e = 0; e < 12; ++e)
		{
			int a = edges[e][0];
			int b = edges[e][1];
			if (!projected[a] || !projected[b])
			{
				continue;
			}
			canvas->K2_DrawLine(screen[a], screen[b], thickness, color);
		}
	}

	void DrawBottomCenterSnapline(
		SDK::UCanvas* canvas,
		float viewW,
		float viewH,
		bool const* projected,
		SDK::FVector2D const* screen,
		SDK::FLinearColor const& color,
		float thickness) noexcept
	{
		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		if (!TryBuildScreenBounds(projected, screen, 8, &minX, &minY, &maxX, &maxY, viewW, viewH))
		{
			return;
		}
		SDK::FVector2D foot;
		foot.X = (minX + maxX) * 0.5f;
		foot.Y = maxY;
		SDK::FVector2D start;
		start.X = viewW * 0.5f;
		start.Y = viewH;
		canvas->K2_DrawLine(start, foot, thickness, color);
	}

	void DrawCenteredLabelAbove(
		SDK::UCanvas* canvas,
		SDK::FString const& text,
		float centerX,
		float topY,
		SDK::FLinearColor const& color,
		float textScale) noexcept
	{
		SDK::FVector2D scale;
		scale.X = textScale;
		scale.Y = textScale;
		SDK::FVector2D textDims = canvas->K2_TextSize(nullptr, text, scale);
		float drawX = centerX - textDims.X * 0.5f;
		float drawY = topY - textDims.Y - 4.0f;
		SDK::FVector2D position;
		position.X = drawX;
		position.Y = drawY;
		SDK::FLinearColor shadowColor;
		shadowColor.R = 0.0f;
		shadowColor.G = 0.0f;
		shadowColor.B = 0.0f;
		shadowColor.A = 0.85f;
		SDK::FVector2D shadowOffset;
		shadowOffset.X = 1.0f;
		shadowOffset.Y = 1.0f;
		SDK::FLinearColor outlineColor;
		outlineColor.R = 0.0f;
		outlineColor.G = 0.0f;
		outlineColor.B = 0.0f;
		outlineColor.A = 1.0f;
		canvas->K2_DrawText(nullptr, text, position, scale, color, 0.0f, shadowColor, shadowOffset, false, false, false, outlineColor);
	}

	bool IsActorAttachedToLocalPawn(SDK::AActor* actor, SDK::APawn* localPawn) noexcept
	{
		if (!localPawn || !actor)
		{
			return false;
		}
		for (SDK::AActor* walk = actor->GetAttachParentActor(); walk; walk = walk->GetAttachParentActor())
		{
			if (walk == localPawn)
			{
				return true;
			}
		}
		return false;
	}

	bool ShouldSkipItemActor(SDK::ABaseItem* item, SDK::APawn* localPawn) noexcept
	{
		return IsActorAttachedToLocalPawn(item, localPawn);
	}

	bool ShouldSkipPickupWeaponActor(SDK::APickupWeaponActor* pickup, SDK::APawn* localPawn) noexcept
	{
		return IsActorAttachedToLocalPawn(pickup, localPawn);
	}

	bool ShouldSkipEspDeadReadyOrNotActor(SDK::AActor* actor) noexcept
	{
		if (!actor || !actor->IsA(SDK::AReadyOrNotCharacter::StaticClass()))
		{
			return false;
		}
		SDK::AReadyOrNotCharacter* ron = static_cast<SDK::AReadyOrNotCharacter*>(actor);
		SDK::UCharacterHealthComponent* hp = ron->GetHealthComponent();
		if (!hp)
		{
			return false;
		}
		return hp->GetHealthStatus() == SDK::EPlayerHealthStatus::HS_Dead;
	}

	void DrawEspForActor(
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::AActor* actor,
		SDK::FString const& label,
		SDK::FLinearColor const& color,
		SDK::APawn* localPawn,
		bool snapThisLayer) noexcept
	{
		if (!actor)
		{
			return;
		}
		if (actor == localPawn)
		{
			return;
		}
		if (ShouldSkipEspDeadReadyOrNotActor(actor))
		{
			return;
		}

		SDK::FVector cornersWorld[8];
		BuildWorldCornersForEsp(actor, cornersWorld);

		bool projected[8];
		SDK::FVector2D screen[8];
		bool hasAny = TryProjectCorners(
			playerController,
			cornersWorld,
			projected,
			screen,
			8,
			usePlayerViewportProjection);
		if (!hasAny && usePlayerViewportProjection)
		{
			hasAny = TryProjectCorners(
				playerController,
				cornersWorld,
				projected,
				screen,
				8,
				false);
		}
		if (!hasAny)
		{
			return;
		}

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		bool hasBounds = TryBuildScreenBounds(projected, screen, 8, &minX, &minY, &maxX, &maxY, viewW, viewH);
		if (hasBounds)
		{
			NormalizeScreenRect(&minX, &minY, &maxX, &maxY);
		}

		if (Visuals::isGradientFillEnabled && imguiList && hasBounds && playerController && playerController->PlayerCameraManager)
		{
			SDK::FVector cameraWorld = playerController->PlayerCameraManager->GetCameraLocation();
			DrawBox3dFaceFillsImGui(
				imguiList,
				projected,
				screen,
				cornersWorld,
				viewW,
				viewH,
				cameraWorld,
				minY,
				maxY,
				Visuals::colorFillTop,
				Visuals::colorFillBottom);
		}

		if (Visuals::isBox3dEnabled)
		{
			if (imguiList)
			{
				DrawBox3dEdgesImGui(imguiList, projected, screen, color, Visuals::fBoxLineThickness);
			}
			else if (canvas)
			{
				DrawBox3dEdges(canvas, projected, screen, color, Visuals::fBoxLineThickness);
			}
		}

		if (snapThisLayer)
		{
			if (imguiList)
			{
				DrawBottomCenterSnaplineImGui(
					imguiList,
					viewW,
					viewH,
					projected,
					screen,
					color,
					Visuals::fSnaplineThickness);
			}
			else if (canvas)
			{
				DrawBottomCenterSnapline(
					canvas,
					viewW,
					viewH,
					projected,
					screen,
					color,
					Visuals::fSnaplineThickness);
			}
		}

		if (Visuals::isEntityLabelEnabled && hasBounds)
		{
			float centerX = (minX + maxX) * 0.5f;
			if (imguiList)
			{
				DrawCenteredLabelAboveImGui(imguiList, label, centerX, minY, color);
			}
			else if (canvas)
			{
				DrawCenteredLabelAbove(canvas, label, centerX, minY, color, 1.0f);
			}
		}
	}

	void DrawLayerSwat(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isSwatLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ASWATCharacter::StaticClass(), &actors);
		SDK::FString label(L"SWAT");
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AActor* actor = actors[i];
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				actor,
				label,
				Visuals::colorSwat,
				localPawn,
				Visuals::isSwatSnaplineEnabled);
		}
	}

	void DrawLayerSuspect(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isSuspectLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ASuspectCharacter::StaticClass(), &actors);
		SDK::FString label(L"Suspect");
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AActor* actor = actors[i];
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				actor,
				label,
				Visuals::colorSuspect,
				localPawn,
				Visuals::isSuspectSnaplineEnabled);
		}
	}

	void DrawLayerCivilian(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isCivilianLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ACivilianCharacter::StaticClass(), &actors);
		SDK::FString label(L"Civilian");
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AActor* actor = actors[i];
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				actor,
				label,
				Visuals::colorCivilian,
				localPawn,
				Visuals::isCivilianSnaplineEnabled);
		}
	}

	void DrawLayerSniper(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isSniperLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ASniperCharacter::StaticClass(), &actors);
		SDK::FString label(L"Sniper");
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AActor* actor = actors[i];
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				actor,
				label,
				Visuals::colorSniper,
				localPawn,
				Visuals::isSniperSnaplineEnabled);
		}
	}

	void DrawLayerBaseItems(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isBaseItemLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ABaseItem::StaticClass(), &actors);
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::ABaseItem* item = static_cast<SDK::ABaseItem*>(actors[i]);
			if (ShouldSkipItemActor(item, localPawn))
			{
				continue;
			}
			SDK::FString label = SDK::UKismetSystemLibrary::GetObjectName(item);
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				item,
				label,
				Visuals::colorBaseItem,
				localPawn,
				Visuals::isBaseItemSnaplineEnabled);
		}
	}

	void DrawLayerPickupWeapons(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isPickupWeaponLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::APickupWeaponActor::StaticClass(), &actors);
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::APickupWeaponActor* pickup = static_cast<SDK::APickupWeaponActor*>(actors[i]);
			if (ShouldSkipPickupWeaponActor(pickup, localPawn))
			{
				continue;
			}
			SDK::FString label = SDK::UKismetSystemLibrary::GetObjectName(pickup);
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				pickup,
				label,
				Visuals::colorPickupWeapon,
				localPawn,
				Visuals::isPickupWeaponSnaplineEnabled);
		}
	}

	void DrawLayerDoors(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isDoorLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::ADoor::StaticClass(), &actors);
		SDK::FString label(L"Door");
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AActor* actor = actors[i];
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				actor,
				label,
				Visuals::colorDoor,
				localPawn,
				Visuals::isDoorSnaplineEnabled);
		}
	}

	void DrawLayerEvidence(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isEvidenceLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::AEvidenceActor::StaticClass(), &actors);
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::AEvidenceActor* evidence = static_cast<SDK::AEvidenceActor*>(actors[i]);
			if (IsActorAttachedToLocalPawn(evidence, localPawn))
			{
				continue;
			}
			SDK::FString label = SDK::UKismetSystemLibrary::GetObjectName(evidence);
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				evidence,
				label,
				Visuals::colorEvidence,
				localPawn,
				Visuals::isEvidenceSnaplineEnabled);
		}
	}

	void DrawLayerMagazinePickups(
		SDK::UObject* worldContext,
		SDK::AHUD* hud,
		SDK::UCanvas* canvas,
		ImDrawList* imguiList,
		SDK::APlayerController* playerController,
		float viewW,
		float viewH,
		bool usePlayerViewportProjection,
		SDK::APawn* localPawn) noexcept
	{
		if (!Visuals::isMagazinePickupLayerEnabled)
		{
			return;
		}
		SDK::TArray<SDK::AActor*> actors;
		SDK::UGameplayStatics::GetAllActorsOfClass(worldContext, SDK::APickupMagazineActor::StaticClass(), &actors);
		for (int i = 0; i < actors.Num(); ++i)
		{
			SDK::APickupMagazineActor* mag = static_cast<SDK::APickupMagazineActor*>(actors[i]);
			if (IsActorAttachedToLocalPawn(mag, localPawn))
			{
				continue;
			}
			SDK::FString label = SDK::UKismetSystemLibrary::GetObjectName(mag);
			DrawEspForActor(
				hud,
				canvas,
				imguiList,
				playerController,
				viewW,
				viewH,
				usePlayerViewportProjection,
				mag,
				label,
				Visuals::colorMagazinePickup,
				localPawn,
				Visuals::isMagazinePickupSnaplineEnabled);
		}
	}
}

void Visuals::RenderReceiveDrawHud(SDK::AHUD* hud) noexcept
{
	if (!Visuals::isModuleEnabled)
	{
		return;
	}
	if (!hud)
	{
		return;
	}

	SDK::APlayerController* playerController = hud->PlayerOwner;
	if (!playerController)
	{
		return;
	}

	SDK::UCanvas* canvas = HudCanvasAccess::ResolveCanvas(hud);
	if (!canvas)
	{
		return;
	}

	float viewW = static_cast<float>(canvas->SizeX);
	float viewH = static_cast<float>(canvas->SizeY);
	if (viewW <= 1.0f || viewH <= 1.0f)
	{
		return;
	}

	SDK::APawn* localPawn = playerController->AcknowledgedPawn;

	DrawLayerSwat(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerSuspect(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerCivilian(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerSniper(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerBaseItems(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerPickupWeapons(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerDoors(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerEvidence(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
	DrawLayerMagazinePickups(hud, hud, canvas, nullptr, playerController, viewW, viewH, false, localPawn);
}

void Visuals::RenderPresentOverlay() noexcept
{
	if (!Visuals::isModuleEnabled)
	{
		return;
	}

	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	if (!drawList)
	{
		return;
	}

	SDK::UWorld* world = SDK::UWorld::GetWorld();
	if (!world)
	{
		return;
	}

	SDK::APlayerController* playerController = SDK::UGameplayStatics::GetPlayerController(world, 0);
	if (!playerController)
	{
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	float viewW = io.DisplaySize.x;
	float viewH = io.DisplaySize.y;
	if (viewW <= 2.0f || viewH <= 2.0f)
	{
		return;
	}

	SDK::APawn* localPawn = playerController->AcknowledgedPawn;

	DrawLayerSwat(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerSuspect(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerCivilian(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerSniper(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerBaseItems(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerPickupWeapons(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerDoors(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerEvidence(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
	DrawLayerMagazinePickups(world, nullptr, nullptr, drawList, playerController, viewW, viewH, true, localPawn);
}
