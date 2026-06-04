#pragma once

#include "imgui.h"

struct AppTheme
{
    ImVec4 orangePrimary = ImVec4(1.0f, 0.498f, 0.0f, 1.0f);
    ImVec4 orangeMuted = ImVec4(0.7f, 0.3f, 0.0f, 1.0f);
    ImVec4 orangeActive = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
    ImVec4 bgDark = ImVec4(0.04f, 0.04f, 0.05f, 0.95f);
    ImVec4 bgPanel = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    ImVec4 textNormal = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    ImVec4 textMuted = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    ImVec4 textAlert = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
    ImVec4 textSuccess = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
    ImVec4 border = ImVec4(0.6f, 0.25f, 0.0f, 0.5f);
    ImVec4 rowHover = ImVec4(1.0f, 0.498f, 0.0f, 0.18f);
    ImVec4 rowHoverActive = ImVec4(1.0f, 0.498f, 0.0f, 0.22f);
    ImVec4 rowSelected = ImVec4(1.0f, 0.498f, 0.0f, 0.32f);

    float buttonWidthLarge = 240.0f;
    float buttonHeightLarge = 50.0f;
    float buttonWidthMedium = 200.0f;
    float buttonHeightMedium = 40.0f;
    float buttonWidthSmall = 100.0f;
    float buttonHeightSmall = 30.0f;
    float buttonWidthPager = 80.0f;
    float progressBarHeight = 30.0f;

    static void Apply(const AppTheme& theme)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Text] = theme.textNormal;
        style.Colors[ImGuiCol_WindowBg] = theme.bgDark;
        style.Colors[ImGuiCol_ChildBg] = theme.bgPanel;
        style.Colors[ImGuiCol_PopupBg] = theme.bgDark;
        style.Colors[ImGuiCol_Border] = theme.border;
        style.Colors[ImGuiCol_FrameBg] = theme.bgDark;
        style.Colors[ImGuiCol_FrameBgHovered] = theme.orangeMuted;
        style.Colors[ImGuiCol_FrameBgActive] = theme.orangePrimary;
        style.Colors[ImGuiCol_TitleBg] = theme.bgPanel;
        style.Colors[ImGuiCol_TitleBgActive] = theme.orangeMuted;
        style.Colors[ImGuiCol_Button] = theme.orangeMuted;
        style.Colors[ImGuiCol_ButtonHovered] = theme.orangeActive;
        style.Colors[ImGuiCol_ButtonActive] = theme.orangePrimary;
        style.Colors[ImGuiCol_Header] = theme.orangeMuted;
        style.Colors[ImGuiCol_HeaderHovered] = theme.orangeActive;
        style.Colors[ImGuiCol_HeaderActive] = theme.orangePrimary;
        style.Colors[ImGuiCol_SliderGrab] = theme.orangePrimary;
        style.Colors[ImGuiCol_SliderGrabActive] = theme.orangeActive;

        style.WindowRounding = 12.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 10.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;

        style.WindowPadding = ImVec2(24.0f, 24.0f);
        style.FramePadding = ImVec2(12.0f, 8.0f);
        style.ItemSpacing = ImVec2(16.0f, 16.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
    }
};
