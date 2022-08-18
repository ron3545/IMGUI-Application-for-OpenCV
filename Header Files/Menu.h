#pragma once

#include "d3d9.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <thread>
#include "..\fonts/icons.h"
#include "FaceRecognition.h"
#include <shlobj.h>
#include <iostream>
#include <tchar.h> 
#include "PrivateHeader.h"
#include "DataBase.h"

#define WINDOW_BACKGROUND		ImColor(58,52,102, 255)
#define BUTTON_HIGHLIGHTED		ImColor(42, 56, 76, 255)
#define BUTTON_DEFAULT_COLOR	ImColor(25, 28, 54, 255)
#define BUTTON_HOVERED			ImColor(42, 56, 76, 255)

static const float minimum_indentation = 20.0f;

namespace rtd
{
	void Centered_Text(std::string text);
}


inline ImVec4 to_vec4(float R, float G, float B, float A)
{
	return ImVec4(R / 255.0, G / 255.0, B / 255.0, A / 255.0);
}


namespace MENU
{
	void Theme();
	std::string charTostring(char* characters);
}

namespace System
{
	bool loginform(jdbc::SQLDataBase& DB);
	void Menu(jdbc::Family& FamInfo) noexcept;
}

namespace ImGui // https://github.com/ocornut/imgui/issues/4722
{
	bool CallendarEx(const char* label, const char* hint,std::string& buff, int buf_size, ImGuiButtonFlags flags,
		const ImVec2& size = ImVec2(0,0)) noexcept;

	bool ColoredButton(const char* label, const ImVec2& size, ImU32 text_color, 
		ImU32 bg_color_1, ImU32 bg_color_2);
}
