#include "..\Header Files\Menu.h"
#include "..\Fonts\IconsFontAwesome5.h"
#include "..\imgui\imgui_stdlib.h"
#include <windows.h>
#include <string>
#include <shlobj.h>
#include <iostream>
#include <sstream>
#include "..\Header Files\Log.h"
#include <array>
#include <map>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "..\imgui\imgui_internal.h"

extern ImFont* font;
using namespace std;
using namespace ImGui;
//helper function
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char*)lpData;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

template<typename T>
inline T CalcIndentation(T& a, T& b) {
	return a - b;
}

void rtd::Centered_Text(std::string text)
{
	float windows_width = ImGui::GetWindowSize().x;
	float Text_width = ImGui::CalcTextSize(text.c_str()).x;
	float Text_indentation = (windows_width - Text_width) * 0.5f;

	if (Text_indentation <= minimum_indentation)
		Text_indentation = minimum_indentation;

	ImGui::SameLine(Text_indentation);
	ImGui::PushTextWrapPos(CalcIndentation(windows_width, Text_indentation));
	ImGui::TextWrapped(text.c_str());
	ImGui::PopTextWrapPos();
}

std::string BrowseFolder()
{
	std::string saved_path = "C:\\";
	TCHAR path[MAX_PATH];
	const char* path_param = saved_path.c_str();
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = ("Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}
		return path;
	}
	return "";
}



void MENU::Theme()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowTitleAlign = ImVec2(0.5, 0.5);
	style->FramePadding = ImVec2(8, 4);

#ifdef BUTTON_DEFAULT_COLOR
	style->Colors[ImGuiCol_Button] = BUTTON_DEFAULT_COLOR;
#endif //BUTTON_DEFAULT_COLOR

#ifdef BUTTON_HOVERED
	style->Colors[ImGuiCol_ButtonHovered] = BUTTON_HOVERED;
#endif //BUTTON_HOVERED

#ifdef BUTTON_HIGHLIGHTED
	style->Colors[ImGuiCol_ButtonActive] = BUTTON_HIGHLIGHTED;
#endif //BUTTON_HIGHLIGHTED
}

std::string MENU::charTostring(char* characters)
{
	int char_size = strlen(characters);
	std::string str = " ";
	for (int i = 0; i < char_size; i++)
		str += characters[i];
	return str;
}

#pragma region MENU
void System::Menu(jdbc::Family& PersonInfo) noexcept
{
	typedef enum class  ButtonId : char
	{
		Default, Register,
		Cam_control_config,
		Import_DataBase,
		Export_DataBase,
	} ButtonId;

	static ButtonId label = ButtonId::Default;
	bool entered = false;
	string export_data_loc;
	string import_data_loc;
	
	const ImVec4 pressed = to_vec4(42, 56, 76, 255);
	const ImVec4 not_pressed = to_vec4(25, 28, 54, 255);

	ImGui::Columns(2);
	ImGui::SetColumnOffset(1, 233);

	{
		//Left side button groups
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, label == ButtonId::Register ? pressed : not_pressed);
		if (ImGui::Button(ICON_FA_KEY " Register", ImVec2(215, 41)))
			label = ButtonId::Register;

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, label == ButtonId::Import_DataBase ? pressed : not_pressed);
		if (ImGui::Button("import DataBase", ImVec2(215, 41)))
		{
			label = ButtonId::Import_DataBase;
			import_data_loc = BrowseFolder();
		}
		
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, label == ButtonId::Export_DataBase ? pressed : not_pressed);
		if (ImGui::Button("Export DataBase", ImVec2(215, 41)))
		{
			label = ButtonId::Export_DataBase;
			export_data_loc = BrowseFolder();
		}
		
		ImGui::PopStyleColor(3);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 30);
	}
	ImGui::NextColumn();
	
	if(label == ButtonId::Register )
	{
		ImGui::TextWrapped("This section allows you to register your family identity, \nwhich includes the full name of the person, their face picture, \nand their part or connection on the family .");
		ImGui::NewLine();
		ImGui::Spacing();

		ImGui::Text("First Name:");
		ImGui::InputTextWithHint("##First", "First Name", &PersonInfo.FirstName);
		ImGui::Spacing();

		ImGui::Text("Middle Name:");
		ImGui::InputTextWithHint("##Middle", "Middle Name", &PersonInfo.MiddleName);
		ImGui::Spacing();

		ImGui::Text("Last Name:");
		ImGui::InputTextWithHint("##Last", "Last Name", &PersonInfo.LastName);
		ImGui::NewLine();
		ImGui::Spacing();

		static int CurrentItem = 0;
		static const char* Sexes[] = { "male", "female" };

		static int CurrentItem_s = 0;
		static const char* Family_relationship[] = { "Father", "Mother", "Brother","Sister","Cousin", "Family Relative" };

		ImGui::Combo("Sex", &CurrentItem, Sexes, IM_ARRAYSIZE(Sexes));
		PersonInfo.Sex = Sexes[CurrentItem];

		ImGui::Combo("Family Relationship", &CurrentItem_s, Family_relationship, IM_ARRAYSIZE(Family_relationship));
		PersonInfo.Relation = Family_relationship[CurrentItem_s];

		ImGui::Text("Date of Birth");
		ImGui::CallendarEx("##Date of Birth", "Date of Birth", PersonInfo.Birthday, PersonInfo.Birthday.capacity(), ImGuiButtonFlags_None);

		ImGui::PushStyleColor(ImGuiCol_Button, entered == true ? pressed : not_pressed);
		if (ImGui::Button(ICON_FA_CHECK "Submit", ImVec2(100, 30)) || cv::waitKey(10) == 13) {
			entered = true;
		}
		
		ImGui::PopStyleColor(1);
	}
	
}
#pragma endregion

#pragma region HELPER_FUNCTIONS

void Login(bool& Connected, bool& continue_,
	bool& is_pressed, jdbc::Account& UD, jdbc::SQLDataBase& DB) 
{
	is_pressed = true;
	if (!UD.IsEmpty())
	{
		DB.SetAccount(UD);
		if (DB.Connect())
		{
			Connected = true;
			continue_ = false;
		}
		else
		{
			is_pressed = Connected = false;
			continue_ = true;
			UD.ClearEntry();
		}
	}
}

void get_data_login(jdbc::Account& UD)
{
	UD.m_ptchHost = "tcp://127.0.0.1:3306";
	UD.m_ptchSchema = "tdap";

	rtd::Centered_Text("CONNECT TO DATABASE");
	ImGui::NewLine();
	ImGui::Spacing();

	ImGui::SameLine(150);
	ImGui::Text("User Name:");
	ImGui::Spacing();

	ImGui::SameLine(150);
	ImGui::InputTextWithHint("##User", "User Name", &UD.m_ptchUser);
	ImGui::Spacing();

	ImGui::SameLine(150);
	ImGui::Text("Password:");
	ImGui::Spacing();

	ImGui::SameLine(150);
	ImGui::InputTextWithHint("##Password", "Password", &UD.m_ptchPassword, ImGuiInputTextFlags_Password);
	ImGui::NewLine();
	ImGui::Spacing();
}
#pragma endregion 

bool System::loginform(jdbc::SQLDataBase& DB)
{
	const float InputText_Width = 1;
	int qstate;
	static bool Connected = false;
	bool continue_ = true;
	static jdbc::Account UD;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize	|
									ImGuiWindowFlags_NoMove		|
									ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoSavedSettings;
	//optimize full viewport 
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	const ImVec4 pressed = to_vec4(42, 56, 76, 255);
	const ImVec4 not_pressed = to_vec4(25, 28, 54, 255);

	ImGui::Begin(" ", &continue_, window_flags);
	{
		get_data_login(UD);

		bool is_pressed = false;

		ImGui::PushStyleColor(ImGuiCol_Button, is_pressed == true ? pressed : not_pressed);
		ImGui::Spacing();  ImGui::SameLine(50, 100);
		if (ImGui::Button("Connect", ImVec2(150, 41)))
		{
			if (!UD.IsEmpty())
			{
				is_pressed = true;
				Login(Connected, continue_, is_pressed, UD, DB);
			}
		}
	} ImGui::End();
	return Connected;
}

#pragma region CALENDAR

bool ImGui::CallendarEx(const char* label, const char* hint, std::string& buf, int buf_size, ImGuiButtonFlags flags,const ImVec2& size) noexcept
{
	ImGuiWindow* wnd = GetCurrentWindow();
	if (wnd->SkipItems)
		return false;
	
	IM_ASSERT(buf != NULL && buf_size >= 0);

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = wnd->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImVec2 frame_size = CalcItemSize(size, CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
	const ImVec2 total_size = ImVec2(frame_size.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), frame_size.y);

	//bounding box data
	const ImRect frame_bb(wnd->DC.CursorPos, wnd->DC.CursorPos + frame_size);
	const ImRect total_bb(frame_bb.Min, frame_bb.Min + total_size);

	ImVec2 inner_size = frame_size;
	ImGuiWindow* draw_window = wnd;

	// support for internal
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(frame_bb, id))
		return false;

	if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	const bool is_displaying_hint = hint != NULL && (buf.empty() || g.ActiveId == id); //is_displaying_hint? ImGuiCol_TextDisabled : ImGuiCol_Text
	const bool hovered = ItemHoverable(frame_bb, id);
	const bool  user_clicked = hovered && io.MouseClicked[0];
	//change the cursor when hovering on the bounding box
	if (hovered)
		g.MouseCursor = ImGuiMouseCursor_TextInput;

#pragma region Popup Callendar
	{
		ImGuiWindowFlags flags =	ImGuiWindowFlags_NoResize			| 
									ImGuiWindowFlags_NoMove				| 
									ImGuiWindowFlags_NoScrollbar		| 
									ImGuiWindowFlags_NoScrollWithMouse	| 
									ImGuiWindowFlags_NoCollapse			|
									ImGuiWindowFlags_NoTitleBar;


		const int starting_year = 1950;
		const int end_year = 2020;

		static const char* months[] =	{"January","Febuary","March", "April", "May", "June", "July", "August", "September", "october", "November", "December"};
		static int month_index = 0;

		const std::array<std::string, 7> weeks	 =	{ {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday","Saturday"} };
		std::string name_of_week_ended = " "; // stores the last day a month iterated

		bool is_common_year = false, is_leap_year = false;
		const int leap_yr = 29, common_yr = 28;

		std::vector<std::string> years; //calculated data with int data type will be converted to a required data type of the BeginCombo ImGui API
		static unsigned int current_year = starting_year; // used to add into the vector int
		static std::string display_current_year = to_string(starting_year); // This will be modified as we iterate on the selection

		if (years.empty()) 
		{
			const int maximum = end_year - starting_year;
			for (int i = 0; i <= maximum; ++i)
			{
				years.push_back(to_string(current_year));
				++current_year;
			}
		}

		const std::map<std::string, int> number_of_days_per_month = { {months[0], 31} };
		const std::map<std::string, int> months_id_number;

		if (user_clicked )
			ImGui::OpenPopup("callendar");
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 5);
		
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(1.00f, 1.00f, 1.00f, 0.94f));
		if (ImGui::BeginPopup("callendar", flags))
		{
			ImGui::Spacing();
			ImGui::SameLine(30);
			if (ImGui::ArrowButton("Left", ImGuiDir_Left)) {
				--month_index;
			}
			
			ImGuiStyle& m_style = ImGui::GetStyle();
			float spacing = m_style.ItemInnerSpacing.x;
			
			ImGui::SameLine(50, spacing);
			ImGui::Spacing();
			ImGui::Combo("##Months", &month_index, months, IM_ARRAYSIZE(months));
			
			{
				ImGui::SameLine(70);
				ImGui::Spacing();
				if (ImGui::BeginCombo("##years", display_current_year.c_str()))
				{
					size_t num_element = years.size();
					for (int n = 0; n < num_element; n++)
					{
						bool is_selected = (display_current_year == years[n]);
						if (ImGui::Selectable(years[n].c_str(), is_selected))
							display_current_year = years[n];
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			if (ImGui::ArrowButton("Left", ImGuiDir_Left)) {
				++month_index;
			}

			ImGui::Separator();
			
			ImGui::PopStyleVar(2);
			ImGui::EndPopup();
		}
	}
#pragma endregion

	//Render Frame
	const ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg, 0.67);
	RenderNavHighlight(frame_bb, id);
	RenderFrame(frame_bb.Min, frame_bb.Max, bg_color, true, style.FrameRounding);

	const ImVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + inner_size.x, frame_bb.Min.y + inner_size.y);
	ImVec2 draw_pos = frame_bb.Min + style.FramePadding;
	ImVec2 text_size(0.0f, 0.0f);
	
	const int buf_display_max_length = 2 * 1024 * 1024;
	const char* buf_display = buf.c_str();
	const char* buf_display_end = NULL;
	if (is_displaying_hint)
	{
		buf_display = hint;
		buf_display_end = hint + strlen(hint);
	}
	
	if ((buf_display_end - buf_display) < buf_display_max_length)
	{
		ImU32 col = GetColorU32(is_displaying_hint ? ImGuiCol_TextDisabled : ImGuiCol_Text);
		draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, col,buf_display, buf_display_end, 0.0f, &clip_rect);
	}

	if (g.LogEnabled)
		LogSetNextTextDecoration("[", "]");
	
	// Automatically close popups
	//if (user_clicked && !(flags & ImGuiButtonFlags_DontClosePopups) && (wnd->Flags & ImGuiWindowFlags_Popup))
	//	CloseCurrentPopup();
	
	if(label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
}

bool ImGui::ColoredButton(const char* label, const ImVec2& size_arg, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	ImGuiButtonFlags flags = ImGuiButtonFlags_None;
	if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const bool is_gradient = bg_color_1 != bg_color_2;
	if (held || hovered)
	{
		// Modify colors (ultimately this can be prebaked in the style)
		float h_increase = (held && hovered) ? 0.02f : 0.02f;
		float v_increase = (held && hovered) ? 0.20f : 0.07f;

		ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
		ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
		bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
		bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
		ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
		bg_color_1 = GetColorU32(bg1f);
		if (is_gradient)
		{
			ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
			ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
			bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
			bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
			ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
			bg_color_2 = GetColorU32(bg2f);
		}
		else
		{
			bg_color_2 = bg_color_1;
		}
	}
	RenderNavHighlight(bb, id);

#if 0
	// V1 : faster but prevents rounding
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
	if (g.Style.FrameBorderSize > 0.0f)
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#endif
	// V2
	int vert_start_idx = window->DrawList->VtxBuffer.Size;
	window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color_1, g.Style.FrameRounding);
	int vert_end_idx = window->DrawList->VtxBuffer.Size;
	if (is_gradient)
		ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
	if (g.Style.FrameBorderSize > 0.0f)
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);

	if (g.LogEnabled)
		LogSetNextTextDecoration("[", "]");
	PushStyleColor(ImGuiCol_Text, text_color);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	PopStyleColor();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	return pressed;
}
#pragma endregion