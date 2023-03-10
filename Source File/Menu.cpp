#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "..\Header Files\Menu.h"

#include "..\Fonts\IconsFontAwesome5.h"
#include "..\imgui\imgui_stdlib.h"
#include <string>
#include <shlobj.h>
#include <iostream>
#include <sstream>
#include "..\Header Files\Log.h"
#include <array>
#include <map>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "../Fonts/icons.h"

#define  STB_IMAGE_IMPLEMENTATION
#include "../includes/stb_image.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#pragma comment(lib, "Shell32.lib")
#else
#include <unistd.h>
#include <pwd.h>
#endif

#define ICON_SIZE			ImGui::GetFont()->FontSize + 3
#define GUI_ELEMENT_SIZE	std::max(GImGui->FontSize + 10.f, 24.f)
#define DEFAULT_ICON_SIZE	32

const long float PI = 3.141592f;

extern ImFont* font;
using namespace std;
using namespace ImGui;
using namespace CPlusPlusLogging;

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct Node
	{
		int delay;
		ImTextureID data;
		struct Node* next;
	}Node;

	STBIDEF unsigned char* LoadGIF(const char* file_path, int* x, int* y, int* frames)
	{
		FILE* gif_path;
		stbi__context gif;
		if (!(gif_path = stbi__fopen(file_path, "r")))
			return stbi__errpuc("can't open", "unable to open file");

		ImTextureID result;

		stbi__start_file(&gif, gif_path);
		if (stbi__gif_test(&gif))
		{
			int comp = 0;
			stbi__gif m_gif;

			Node head;
			Node* prev_node = 0, * gr = &head;
			*frames = 0;
			
			memset(&m_gif, 0, sizeof(m_gif));
			memset(&head, 0, sizeof(head));

			
		}
	}
}


void rtd::Centered_Text(std::string text)
{
	float windows_width = ImGui::GetWindowSize().x;
	float Text_width = ImGui::CalcTextSize(text.c_str()).x;
	float Text_indentation = (windows_width - Text_width) * 0.5f;

	if (Text_indentation <= minimum_indentation)
		Text_indentation = minimum_indentation;

	ImGui::SameLine(Text_indentation);
	ImGui::PushTextWrapPos(windows_width - Text_indentation);
	ImGui::TextWrapped(text.c_str());
	ImGui::PopTextWrapPos();
}

void System::SystemMenu::Theme()
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

std::string charTostring(char* characters)
{
	int char_size = strlen(characters);
	std::string str;
	for (int i = 0; i < char_size; i++)
		str += characters[i];
	return str;
}

#pragma region MENU

cv::Mat System::SystemMenu::frames;

System::SystemMenu::SystemMenu(const std::string& project_name)
	: optimal_image_width_height(412), max_image_width(0), image_width(0), image_height(0), radio_v(0), original_texture_id(-1),
	modified_texture_id(-1), window_flags(0), tbl_face("tbl_face"), tbl_family_info("tbl_family_info"), tbl_unknown_person("tbl_unknown_person")
{
	if (gl3wInit() != 0)
		Logger::getInstance()->error("failed to initialize OpenGL loader!");

	capture.open(0, cv::CAP_ANY);
	this->project_name = project_name;
	
	ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		return (void*)tex;
	};
	ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
		GLuint texID = (GLuint)((uintptr_t)tex);
		glDeleteTextures(1, &texID);
	};
}

System::SystemMenu::~SystemMenu()
{
	capture.release();
}

void System::SystemMenu::UpdateTexture( cv::Mat images, bool first)
{
	unsigned char* data;
	if (first)
	{
		data = images.ptr();
		if (original_texture_id == -1)
			glGenTextures(1, &original_texture_id);
		glBindTexture(GL_TEXTURE_2D, original_texture_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, (images.step & 3) ? 1 : 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

		if (modified_texture_id == -1)
			glGenTextures(1, &modified_texture_id);
		glBindTexture(GL_TEXTURE_2D, modified_texture_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, (images.step & 3) ? 1 : 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		data = images.ptr();
		glBindTexture(GL_TEXTURE_2D, modified_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	}
}

void System::SystemMenu::UpdateTexture(cv::Mat images, GLuint& texture_id, GLuint& modified, int height, int width, bool first)
{
	unsigned char* data;
	if (first)
	{
		data = images.ptr();
		if (texture_id == -1)
			glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, (images.step & 3) ? 1 : 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

		if (texture_id == -1)
			glGenTextures(1, &modified);
		glBindTexture(GL_TEXTURE_2D, modified);
		glPixelStorei(GL_UNPACK_ALIGNMENT, (images.step & 3) ? 1 : 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		data = images.ptr();
		glBindTexture(GL_TEXTURE_2D, modified);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	}
}

void System::SystemMenu::Menu() noexcept
{
	static bool is_connected = false;
	if (is_connected)
	{
		window_flags =	ImGuiWindowFlags_NoMove				|
						ImGuiWindowFlags_NoTitleBar			|
						ImGuiWindowFlags_NoSavedSettings	|
						ImGuiWindowFlags_NoResize			|
						ImGuiWindowFlags_AlwaysAutoResize	|
						ImGuiWindowFlags_NoScrollbar		|
						ImGuiWindowFlags_NoScrollWithMouse;

		//optimize full viewport 
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		if (ImGui::Begin("Threath Detection", NULL, window_flags))
		{
			static std::string	FirstName,
								MiddleName,
								LastName,
								Relation,
								Sex,
								Birthday; //YYYY-MM-DD

			static bool m_register		= false;
			static bool m_export		= false;
			static bool m_import		= false;
			static bool register_mode	= false;

			bool entered = false;

			if (register_mode != true && capture.isOpened())
					capture >> frames;
			
			{
				ImGuiWindowFlags flag = ImGuiWindowFlags_NoTitleBar		|
										ImGuiWindowFlags_NoMove			|
										ImGuiWindowFlags_NoScrollbar	|
										ImGuiWindowFlags_NoCollapse		|
										ImGuiWindowFlags_NoResize;

				if (m_register /* || DB.Is_TableEmpty("tbl_face") || DB.Is_TableEmpty("tbl_face_info")*/) {
					ImGui::OpenPopup("Register");
					register_mode = true;
				}
			
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.01f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 5.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

				if (ImGui::BeginPopupModal("Register", NULL, flag))
				{
					static std::vector<cv::Mat> images;
					
					HelpMarker("This section allows you to register your family identity, \nwhich includes the full name of the person, their face picture, \nand their part or connection on the family.");

					ImGui::Columns(2, 0, false);
					ImGui::SetColumnOffset(1, 250);
					{
						ImGui::Text("First Name:");
						ImGui::InputTextWithHint("##First", "First Name", &FirstName);
						ImGui::Spacing();

						ImGui::Text("Middle Name:");
						ImGui::InputTextWithHint("##Middle", "Middle Name", &MiddleName);
						ImGui::Spacing();

						ImGui::Text("Last Name:");
						ImGui::InputTextWithHint("##Last", "Last Name", &LastName);
						ImGui::NewLine();
						ImGui::Spacing();
					}
					ImGui::NextColumn();
					//image rendering and face capturing 
					{
						const int width = 150, height = 150;
				
						ImGui::SetColumnOffset(1, 250);
						ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
						ImGui::BeginChild("Face Capture", ImVec2(150.0f, 150.0f), true, flag);
						{
							for (const auto& image: image_folder_path)
							{
								m_image = cv::imread(image.string());
								images.push_back(m_image);
							}
							image_folder_path.clear();

							try {
								
								static GLuint texture_id, modified;
								//display the last image saved
								if (!m_image.empty())
								{
									cv::Mat image;

									float resized_height;
									const int optimal_size = 211;
									float ratio = m_image.size().width / m_image.size().height;
									if (ratio * optimal_size > width)
										resized_height = width / m_image.size().width * m_image.size().height;
									else
										resized_height = optimal_size;
									double scale = resized_height / m_image.size().height;

									cv::resize(m_image, image, cv::Size(width, height),scale,scale,cv::INTER_AREA);
									
									UpdateTexture(image,texture_id, modified, height, width, true);

									SetCursorPos((GetWindowSize() - ImVec2(width, height)) * 0.5f);
									ImGui::Image((void*)(intptr_t)texture_id, ImVec2((float)width + 12	, (float)height));
								}
							}
							catch (cv::Exception& ex)
							{
								std::ostringstream os;
								os << "error code: " << ex.code << "\n" << "error description: " << ex.what() << "\n";
								os << "File: " << ex.file << "\n" << "Function Name: " << ex.func << "\n";
								os << "line number: " << ex.line << " \n " << "message: " << ex.msg;

								Logger::getInstance()->always("==========================================================================");
								Logger::getInstance()->error(os);
								Logger::getInstance()->always("==========================================================================");
							}
						}
						ImGui::EndChild();
						ImGui::Spacing();
						
						//refer to this: https://docs.microsoft.com/en-us/windows/win32/api/commdlg/nf-commdlg-getopenfilenamea
						
						if (ImGui::Button("Select Image", ImVec2(150.0f, 28.0f)))
						{
							ImGui::Begin("select", NULL, ImGuiWindowFlags_NoResize);
							ifd::FileDialog::Instance().Open("IMAGES", "Open a Images", "Image file (*.png;*.jpg;*.jpeg){.png,.jpg,.jpeg},.*", true, "C:\\");
							ImGui::End();
						}
						// file dialogs
						if (ifd::FileDialog::Instance().IsDone("IMAGES")) {
							if (ifd::FileDialog::Instance().HasResult()) {
								if (image_folder_path.empty())
									image_folder_path = ifd::FileDialog::Instance().GetResults();
								else image_folder_path.clear();
							}
							ifd::FileDialog::Instance().Close();
						}

						ImGui::PopStyleVar(2);
					}
					ImGui::EndColumns();

					static int CurrentItem = 0;
					static const char* Sexes[] = { "male", "female" };

					static int CurrentItem_s = 0;
					static const char* Family_relationship[] = { "Father", "Mother", "Son","Daugther","Cousin", "Family Relative" };

					ImGui::Text("Sex");
					ImGui::Combo("##Sex", &CurrentItem, Sexes, IM_ARRAYSIZE(Sexes));
					Sex = Sexes[CurrentItem];

					ImGui::Text("Family Relationship");
					ImGui::Combo("##Family Relationship", &CurrentItem_s, Family_relationship, IM_ARRAYSIZE(Family_relationship));
					Relation = Family_relationship[CurrentItem_s];

					ImGui::Text("Date of Birth");
					ImGui::Callendar("##Date of Birth", "Date of Birth", Birthday, Birthday.capacity(), ImGuiButtonFlags_None);

					int num_defective_image = 0;
					ImGui::PushStyleColor(ImGuiCol_Button, entered == true ? pressed : not_pressed);
					if (ImGui::Button("Register", ImVec2(100, 30)))
					{
						if (FirstName.empty() || MiddleName.empty() || LastName.empty() || images.empty())
						{
							ImGui::OpenPopup("Warning");
						}
						else
						{
							entered = true;
							std::vector<cv::Mat> face_images;
							bool stop_insert_proc = true;

							for (const cv::Mat& img : images) {
								if (FR.Face_Count(img)) {
									face_images.push_back(img);
								}
								else
									++num_defective_image;
							}
							
							if (num_defective_image > 0) {
								ImGui::OpenPopup("image register error");
								face_images.clear();
								stop_insert_proc = true;
							}
							else if (images.size() == face_images.size())
								stop_insert_proc = false;
							
							if (!stop_insert_proc) {
								try
								{
									if (DB.InsertFamily(face_images, FirstName, MiddleName, LastName, Sex, Relation, Birthday))
										Logger::getInstance()->info("insert successful!");
									else Logger::getInstance()->error("insert is not successful!");
								}
								catch (cv::Exception& ex)
								{
									std::ostringstream os;
									os << "error croping the image" << endl;
									os << "error code: " << ex.code << "\n" << "error description: " << ex.what() << "\n";
									os << "File: " << ex.file << "\n" << "Function Name: " << ex.func << "\n";
									os << "line number: " << ex.line << " \n " << "message: " << ex.msg;

									Logger::getInstance()->always("==========================================================================");
									Logger::getInstance()->error(os);
									Logger::getInstance()->always("==========================================================================");
								}
								register_mode = false;
								m_register = false;

								ImGui::CloseCurrentPopup();
							}
						}
					}

					if (ImGui::BeginPopupModal("image register error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
					{
						std::string text = "There are total of " + std::to_string(num_defective_image) + " image(s) rejected due to multiple images selected. \nTo submit this to the database, you must re-enter " + "\nyour pictures and make sure that there is only one face visible.";
						ImGui::Text(text.c_str());

						m_image.release();

						if (ImGui::Button("Ok"))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}

					if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
					{
						ImGui::Text("Fill all the blanks");

						if (ImGui::Button("Ok"))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}

					ImGui::SameLine();

					ImGui::PushStyleColor(ImGuiCol_Button, entered == true ? pressed : not_pressed);
					if (ImGui::Button("Cancel", ImVec2(100, 30)))
					{
						m_register = false;
						register_mode = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();// register popup modal
				}
				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar(3);
				ImGui::NewLine();
			}
			
			ImGui::Columns(2, 0, false);
			ImGui::SetColumnOffset(1, 265);

			{
				ImGuiWindowFlags child_flag =	ImGuiWindowFlags_None			|
												ImGuiWindowFlags_NoResize		|
												ImGuiWindowFlags_NoMove			|
												ImGuiWindowFlags_NoScrollbar	|
												ImGuiWindowFlags_NoCollapse		|
												ImGuiWindowFlags_NoTitleBar;

				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 20.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
				ImGui::BeginChild("Buttons", ImVec2((ImGui::GetContentRegionAvail().x * 1.002f) - 16, ImGui::GetContentRegionAvail().y * 1.002), true, child_flag);
				{
					//Left side button groups
					ImGui::Spacing();
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, m_register == true ? pressed : not_pressed);
					if (ImGui::Button("Register", ImVec2(215, 41))) 
					{
						m_register	= true;
						m_export	= m_import =false;
					}

					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, m_import ? pressed : not_pressed);
					if (ImGui::Button("import DataBase", ImVec2(215, 41)))
					{
						m_import	= true;
						m_register	= m_export = false;

						register_mode = true;
						ImGui::Begin("import", NULL, ImGuiWindowFlags_NoResize);
						ifd::FileDialog::Instance().Open("IMPORT", "Open a Images", "Image file (*.png;*.jpg;*.jpeg){.png,.jpg,.jpeg},.*", true, "C:\\");
						ImGui::End();
					}
					// file dialogs
					if (ifd::FileDialog::Instance().IsDone("IMPORT")) 
					{
						if (ifd::FileDialog::Instance().HasResult()) 
							import_file = ifd::FileDialog::Instance().GetResult();
						
						ifd::FileDialog::Instance().Close();
					}

					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, m_export? pressed : not_pressed);
					if (ImGui::Button("Export DataBase", ImVec2(215, 41)))
					{
						m_export	= true;
						m_register	= m_import = false;

						register_mode = true;
						ImGui::Begin("export", NULL, ImGuiWindowFlags_NoResize);
						ifd::FileDialog::Instance().Open("EXPORT", "Open a Images", "Image file (*.png;*.jpg;*.jpeg){.png,.jpg,.jpeg},.*", false, "C:\\");
						ImGui::End();
					}
					// file dialogs
					if (ifd::FileDialog::Instance().IsDone("EXPORT")) 
					{
						if (ifd::FileDialog::Instance().HasResult()) 
							export_file = ifd::FileDialog::Instance().GetResult();
						
						ifd::FileDialog::Instance().Close();
					}

					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 20.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
					ImGui::BeginChild("Information", ImVec2((ImGui::GetContentRegionAvail().x * 1.002f) , ImGui::GetContentRegionAvail().y * 1.002), true, child_flag);
					{

					}
					ImGui::EndChild();
					ImGui::PopStyleVar(4);
					ImGui::PopStyleColor(3);
					ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 30);
				}
				ImGui::EndChild();
				ImGui::PopStyleVar(2);
			}
			ImGui::NextColumn();

			{
				ImGuiWindowFlags child_flag =	ImGuiWindowFlags_None			|
												ImGuiWindowFlags_NoResize		|
												ImGuiWindowFlags_NoMove			|
												ImGuiWindowFlags_NoScrollbar	|
												ImGuiWindowFlags_NoCollapse		|
												ImGuiWindowFlags_NoTitleBar;

				const float width = (ImGui::GetContentRegionAvail().x * 1.002f) - 10;
				const float height = (ImGui::GetContentRegionAvail().y * 1.002) - 60;

				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
				ImGui::BeginChild("Video", ImVec2(width, height), true, child_flag);
				{
					std::map <std::string, jdbc::RegistryImages> person_images;
					try
					{	
						static cv::Mat prev_frame, derived_frame;

						if (!register_mode)
						{
							prev_frame = frames.clone();
							derived_frame = frames.clone();
						}
						else  derived_frame = prev_frame.clone();

						if (!derived_frame.empty())
						{
							image_width = derived_frame.size().width;
							image_height = derived_frame.size().height;
							
							if (!DB.Is_TableEmpty(tbl_face.c_str()) && DB.Is_TableEmpty(tbl_family_info.c_str())){
							}
							//if(radio_v == Face_Recognition && !register_mode)
								//FR.Recognize(Selected_Name, derived_frame, person_images,capture.get(cv::CAP_PROP_FRAME_WIDTH), int(capture.get(cv::CAP_PROP_FRAME_HEIGHT)));

							UpdateTexture(derived_frame, true);
							ImGui::Image((void*)(intptr_t)original_texture_id, ImVec2((float)image_width + 15, (float)image_height));
						}
					}
					catch (const cv::Exception& ex)
					{
						std::ostringstream os;
						os << "error code: " << ex.code << "\n" << "error description: " << ex.what() << "\n";
						os << "File: " << ex.file << "\n" << "Function Name: " << ex.func << "\n";
						os << "line number: " << ex.line << " \n " << "message: " << ex.msg;

						Logger::getInstance()->always("=====================================OPENCV START=====================================");
						Logger::getInstance()->error(os);
						Logger::getInstance()->always("=====================================OPENCV END=======================================");
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}

			{
				ImGuiWindowFlags child_flag =	ImGuiWindowFlags_None			|
												ImGuiWindowFlags_NoResize		|
												ImGuiWindowFlags_NoMove			|
												ImGuiWindowFlags_NoScrollbar	|
												ImGuiWindowFlags_NoCollapse		|
												ImGuiWindowFlags_NoTitleBar;

				const float width = (ImGui::GetContentRegionAvail().x * 1.002f) - 10;
				const float height = ImGui::GetContentRegionAvail().y * 1.002f;

				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
				ImGui::BeginChild("Selector", ImVec2(width, height), true, child_flag);
				{
					ImGui::RadioButton("Face Recognition", &radio_v, Face_Recognition);	ImGui::SameLine();
					ImGui::RadioButton("Object Detection", &radio_v, Object_Detection);	ImGui::SameLine();
					ImGui::RadioButton("Pose Recognition", &radio_v, Pose_Recognition);
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}

			ImGui::EndColumns();
			ImGui::End();
		}
	} 
	else is_connected = loginform();
}
#pragma endregion

#pragma region HELPER_FUNCTIONS

void System::SystemMenu::Login(bool& Connected, bool& continue_,bool& is_pressed, jdbc::Account& UD)
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

void System::SystemMenu::get_data_login(jdbc::Account& UD)
{
	UD.m_ptchHost = "tcp://127.0.0.1:3306";
	UD.m_ptchSchema = "tdap";

	rtd::Centered_Text("LOGIN");
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

bool System::SystemMenu::loginform()
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

	if(ImGui::Begin("Login", &continue_, window_flags))
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
				Login(Connected, continue_, is_pressed,UD);
			}
		}
		ImGui::PopStyleColor();
		ImGui::End();
	}
	return Connected;
}

#pragma region CALENDAR

/*A Function that returns the index of the day
  of the date- day/month/year
  For e.g-

  Index     Day
  0         Sunday
  1         Monday
  2         Tuesday
  3         Wednesday
  4         Thursday
  5         Friday
  6         Saturday*/
int dayNumber( int day, int month, int year )
{
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1,
					   4, 6, 2, 4 };
	year -= month < 3;
	return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

void CallendarEx(std::string& buf, std::string& date, bool user_clicked, bool& display_hint)
{
	ImGuiWindowFlags flags =	ImGuiWindowFlags_NoTitleBar			|
								ImGuiWindowFlags_NoMove				|
								ImGuiWindowFlags_NoScrollbar		|
								ImGuiWindowFlags_NoScrollWithMouse	|
								ImGuiWindowFlags_NoCollapse			|
								ImGuiWindowFlags_NoResize;

	const int starting_year = 1950;
	const int end_year = 2022;

	static const char* months[] = { "January","Febuary","March", "April", "May", "June", "July", "August", "September", "october", "November", "December" };
	const int number_of_months = sizeof(months) / sizeof(months[0]);
	static int month_index = 0;

	const std::array<std::string, 7> weeks = { {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri","Sat"} };
	std::string name_of_week_ended; // stores the last day a month iterated

	bool is_leap_year = false;
	const int leap_yr = 29, common_yr = 28;

	static std::vector<std::string> years; //calculated data with int data type will be converted to a required data type of the BeginCombo ImGui API
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

	static int day_selected = 0;

	const char* callendar_id = "callendar";
	ImGuiStyle& popup_style = ImGui::GetStyle();
	if (user_clicked)
		ImGui::OpenPopup(callendar_id);

	popup_style.Colors[ImGuiCol_PopupBg] = ImColor(0, 0, 0, 255);
	popup_style.PopupRounding = 16;

	if (ImGui::BeginPopup(callendar_id, flags))
	{
		float spacing = popup_style.ItemInnerSpacing.x;

		ImGui::SameLine(0, spacing * 6);
		if (ImGui::ArrowButton("##Left", ImGuiDir_Left))
		{
			if (month_index <= 0)
				month_index = number_of_months;
			month_index--;
		}

		//calculate combo width
		float size = ImGui::CalcItemWidth() - popup_style.ItemInnerSpacing.x * 51;

		ImGui::SameLine(0, spacing);
		ImGui::PushItemWidth(size + 22);
		ImGui::Combo("##Months", &month_index, months, IM_ARRAYSIZE(months));
		ImGui::PopItemWidth();

		ImGui::SameLine(0, spacing * 13);
		ImGui::PushItemWidth(size);
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
		ImGui::PopItemWidth();

		ImGui::SameLine(0, spacing);
		if (ImGui::ArrowButton("##Right", ImGuiDir_Right))
		{
			if (month_index >= number_of_months - 1)
				month_index = -1;
			month_index++;
		}

		ImGui::Separator();
		
		const ImGuiTableFlags flag = ImGuiTableFlags_BordersV |ImGuiTableFlags_SizingStretchSame;
		int year_selected = std::stoi(display_current_year);

		if ((year_selected % 4 == 0 && year_selected % 100 != 0) || year_selected % 400 == 0)
			is_leap_year = true;
		else
			is_leap_year = false;

		typedef  std::map<std::string, int> MAP;
		MAP number_of_days_per_month = { {months[0], 31}, {months[1], is_leap_year ? leap_yr : common_yr},
			{months[2], 31}, {months[3], 30}, {months[4], 31}, {months[5], 30}, {months[6], 31}, {months[7],31},
			{months[8], 30}, {months[9],31}, {months[10], 30}, {months[11], 31} };

		const MAP months_id = { {months[0], 1}, {months[1], 2},
			{months[2], 3}, {months[3], 4}, {months[4], 5}, {months[5], 6}, {months[6], 7}, {months[7],8},
			{months[8], 9}, {months[9],10}, {months[10], 11}, {months[11], 12} };
		int month_id = 0;
		int max_days = 0;

		const ImVec2 cell_padding(10.0f, 0.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
		if (ImGui::BeginTable("days getter", weeks.size()), flag)
		{
			MAP::const_iterator pos1 = number_of_days_per_month.find(months[month_index]);
			if (pos1 == number_of_days_per_month.end())
			{
				std::ostringstream msg;
				msg << "cannot find: " << months[month_index];
				Logger::getInstance()->error(msg);
			}
			else max_days = pos1->second;

			MAP::const_iterator pos2 = months_id.find(months[month_index]);
			if (pos2 == months_id.end())
			{
				std::ostringstream msg;
				msg << "cannot find: " << months[month_index];
				Logger::getInstance()->error(msg);
			}
			else
				month_id = pos2->second;

			for (const auto& day : weeks)
			{
				ImGui::TableSetupColumn(day.c_str());
			}
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();

#pragma region gregorian
			boost::gregorian::greg_year m_year(1400);
			boost::gregorian::greg_month m_month(1);

			std::string day;
			try
			{
				m_year = boost::gregorian::greg_year(year_selected);
				m_month = boost::gregorian::greg_month(month_id);
			}
			catch (boost::gregorian::bad_year by) { Logger::getInstance()->error(by.what()); }
			catch (boost::gregorian::bad_month bm) { Logger::getInstance()->error(bm.what()); }

			boost::gregorian::date d(m_year, m_month, 1);
			d = (m_year, m_month, d.end_of_month());
			boost::gregorian::date_duration dd(1);
			d += dd;
			day = d.day_of_week();
			// for spacing
			int current = dayNumber(d.day(), month_id, year_selected);
#pragma endregion

			int k;
			for (k = 0; k < current; k++)
				ImGui::TableSetColumnIndex(k);

			for (int d = 1; d <= max_days; d++)
			{
				ImGui::TableSetColumnIndex(k);
				std::string day = to_string(d);
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f, 8.0f, 9.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(50.0f, 8.0f, 12.0f));
				if (ImGui::Button(day.c_str())) {
					day_selected = std::stoi(day);

					std::ostringstream date_format1, date_format2;
					std::string day_str = to_string(day_selected);
					std::string month_str = to_string(month_id);

					date_format1 << day + "," << months[month_index] << " " << year_selected;
					buf = date_format1.str(); //passed to mysql as a varchar
					
					date_format2 << (month_str.length() > 1 ? month_str : "0" + month_str) << "/" << (day_str.length() > 1? day_str : "0" + day_str) << "/" << year_selected;
					date = date_format2.str(); //displayed 

					display_hint = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopStyleColor(3);
				if (++k > 6)
				{
					k = 0;
					ImGui::TableNextRow();
				}
			}
			if (k) ImGui::TableNextRow();
			current = k;

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::EndPopup();
	}
}

void ImGui::HelpMarker(const char* desc)
{
	
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

IMGUI_API bool ImGui::Callendar(const char* label, const char* hint, std::string& buf, int buf_size, ImGuiButtonFlags flags,const ImVec2& size) noexcept
{
	ImGuiWindow* wnd = GetCurrentWindow();
	if (wnd->SkipItems)
		return false;
	
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

	static bool display_hint = true;
	const bool is_displaying_hint = hint != NULL && (buf.empty() || g.ActiveId == id); //is_displaying_hint? ImGuiCol_TextDisabled : ImGuiCol_Text
	const bool hovered = ItemHoverable(frame_bb, id);
	const bool  user_clicked = hovered && io.MouseClicked[0];
	//change the cursor when hovering on the bounding box
	if (hovered)
		g.MouseCursor = ImGuiMouseCursor_TextInput;

#pragma region Popup Callendar
	static std::string display_date;
	CallendarEx(buf, display_date, user_clicked, display_hint);
#pragma endregion

	//Render Frame
	const ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg, 0.67);
	RenderNavHighlight(frame_bb, id);
	RenderFrame(frame_bb.Min, frame_bb.Max, bg_color, true, style.FrameRounding);

	const ImVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + inner_size.x, frame_bb.Min.y + inner_size.y);
	ImVec2 draw_pos = frame_bb.Min + style.FramePadding;
	ImVec2 text_size(0.0f, 0.0f);
	
	const int buf_display_max_length = 2 * 1024 * 1024;
	const char* buf_display = display_date.c_str();
	const char* buf_display_end = NULL;
	if (is_displaying_hint && display_hint)
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

bool System::ImGui_MessageBox(const char* label, std::string message)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(label, NULL, flags))
	{
		ImGui::Text(message.c_str());
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup();
			return true;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			return false;
		}
		ImGui::EndPopup();
	}
}


#pragma region FILE_DIALOGUE
namespace ifd {
	static const char* GetDefaultFolderIcon();
	static const char* GetDefaultFileIcon();

	/* UI CONTROLS */
	bool FolderNode(const char* label, ImTextureID icon, bool& clicked)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		clicked = false;

		ImU32 id = window->GetID(label);
		int opened = window->StateStorage.GetInt(id, 0);
		ImVec2 pos = window->DC.CursorPos;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= pos.x && g.IO.MousePos.x < pos.x + g.FontSize);
		if (ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2)))
		{
			if (is_mouse_x_over_arrow) {
				int* p_opened = window->StateStorage.GetIntRef(id, 0);
				opened = *p_opened = !*p_opened;
			}
			else {
				clicked = true;
			}
		}
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		if (doubleClick && hovered) {
			int* p_opened = window->StateStorage.GetIntRef(id, 0);
			opened = *p_opened = !*p_opened;
			clicked = false;
		}
		if (hovered || active)
			window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

		// Icon, text
		float icon_posX = pos.x + g.FontSize + g.Style.FramePadding.y;
		float text_posX = icon_posX + g.Style.FramePadding.y + ICON_SIZE;
		ImGui::RenderArrow(window->DrawList, ImVec2(pos.x, pos.y + g.Style.FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[((hovered && is_mouse_x_over_arrow) || opened) ? ImGuiCol_Text : ImGuiCol_TextDisabled]), opened ? ImGuiDir_Down : ImGuiDir_Right);
		window->DrawList->AddImage(icon, ImVec2(icon_posX, pos.y), ImVec2(icon_posX + ICON_SIZE, pos.y + ICON_SIZE));
		ImGui::RenderText(ImVec2(text_posX, pos.y + g.Style.FramePadding.y), label);
		if (opened)
			ImGui::TreePush(label);
		return opened != 0;
	}
	bool FileNode(const char* label, ImTextureID icon) {
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		//ImU32 id = window->GetID(label);
		ImVec2 pos = window->DC.CursorPos;
		bool ret = ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2));

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		if (hovered || active)
			window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

		// Icon, text
		window->DrawList->AddImage(icon, ImVec2(pos.x, pos.y), ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE));
		ImGui::RenderText(ImVec2(pos.x + g.Style.FramePadding.y + ICON_SIZE, pos.y + g.Style.FramePadding.y), label);

		return ret;
	}
	bool PathBox(const char* label, std::filesystem::path& path, char* pathBuffer, ImVec2 size_arg) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		bool ret = false;
		const ImGuiID id = window->GetID(label);
		int* state = window->StateStorage.GetIntRef(id, 0);

		ImGui::SameLine();

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 uiPos = ImGui::GetCursorPos();
		ImVec2 size = ImGui::CalcItemSize(size_arg, 200, GUI_ELEMENT_SIZE);
		const ImRect bb(pos, pos + size);

		// buttons
		if (!(*state & 0b001)) {
			ImGui::PushClipRect(bb.Min, bb.Max, false);

			// background
			bool hovered = g.IO.MousePos.x >= bb.Min.x && g.IO.MousePos.x <= bb.Max.x &&
				g.IO.MousePos.y >= bb.Min.y && g.IO.MousePos.y <= bb.Max.y;
			bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
			bool anyOtherHC = false; // are any other items hovered or clicked?
			window->DrawList->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[(*state & 0b10) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));

			// fetch the buttons (so that we can throw some away if needed)
			std::vector<std::string> btnList;
			float totalWidth = 0.0f;
			for (auto comp : path) {
				std::string section = comp.u8string();
				if (section.size() == 1 && (section[0] == '\\' || section[0] == '/'))
					continue;

				totalWidth += ImGui::CalcTextSize(section.c_str()).x + style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
				btnList.push_back(section);
			}
			totalWidth -= GUI_ELEMENT_SIZE;

			// UI buttons
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			bool isFirstElement = true;
			for (size_t i = 0; i < btnList.size(); i++) {
				if (totalWidth > size.x - 30 && i != btnList.size() - 1) { // trim some buttons if there's not enough space
					float elSize = ImGui::CalcTextSize(btnList[i].c_str()).x + style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
					totalWidth -= elSize;
					continue;
				}

				ImGui::PushID(static_cast<int>(i));
				if (!isFirstElement) {
					ImGui::ArrowButtonEx("##dir_dropdown", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));
					anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
					ImGui::SameLine();
				}
				if (ImGui::Button(btnList[i].c_str(), ImVec2(0, GUI_ELEMENT_SIZE))) {
#ifdef _WIN32
					std::string newPath = "";
#else
					std::string newPath = "/";
#endif
					for (size_t j = 0; j <= i; j++) {
						newPath += btnList[j];
#ifdef _WIN32
						if (j != i)
							newPath += "\\";
#else
						if (j != i)
							newPath += "/";
#endif
					}
					path = std::filesystem::u8path(newPath);
					ret = true;
				}
				anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
				ImGui::SameLine();
				ImGui::PopID();

				isFirstElement = false;
			}
			ImGui::PopStyleVar(2);


			// click state
			if (!anyOtherHC && clicked) {
				strcpy(pathBuffer, path.u8string().c_str());
				*state |= 0b001;
				*state &= 0b011; // remove SetKeyboardFocus flag
			}
			else
				*state &= 0b110;

			// hover state
			if (!anyOtherHC && hovered && !clicked)
				*state |= 0b010;
			else
				*state &= 0b101;

			ImGui::PopClipRect();

			// allocate space
			ImGui::SetCursorPos(uiPos);
			ImGui::ItemSize(size);
		}
		// input box
		else {
			bool skipActiveCheck = false;
			if (!(*state & 0b100)) {
				skipActiveCheck = true;
				ImGui::SetKeyboardFocusHere();
				if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					*state |= 0b100;
			}
			if (ImGui::InputTextEx("##pathbox_input", "", pathBuffer, 1024, size_arg, ImGuiInputTextFlags_EnterReturnsTrue)) {
				std::string tempStr(pathBuffer);
				if (std::filesystem::exists(tempStr))
					path = std::filesystem::u8path(tempStr);
				ret = true;
			}
			if (!skipActiveCheck && !ImGui::IsItemActive())
				*state &= 0b010;
		}

		return ret;
	}
	bool FavoriteButton(const char* label, bool isFavorite)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		ImVec2 pos = window->DC.CursorPos;
		bool ret = ImGui::InvisibleButton(label, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();

		float size = g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x;

		int numPoints = 5;
		float innerRadius = size / 4;
		float outerRadius = size / 2;
		float angle = PI / numPoints;
		ImVec2 center = ImVec2(pos.x + size / 2, pos.y + size / 2);

		// fill
		if (isFavorite || hovered || active) {
			ImU32 fillColor = 0xff00ffff;// ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
			if (hovered || active)
				fillColor = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]);

			// since there is no PathFillConcave, fill first the inner part, then the triangles
			// inner
			window->DrawList->PathClear();
			for (int i = 1; i < numPoints * 2; i += 2)
				window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin(i * angle), center.y - innerRadius * cos(i * angle)));
			window->DrawList->PathFillConvex(fillColor);

			// triangles
			for (int i = 0; i < numPoints; i++) {
				window->DrawList->PathClear();

				int pIndex = i * 2;
				window->DrawList->PathLineTo(ImVec2(center.x + outerRadius * sin(pIndex * angle), center.y - outerRadius * cos(pIndex * angle)));
				window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin((pIndex + 1) * angle), center.y - innerRadius * cos((pIndex + 1) * angle)));
				window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin((pIndex - 1) * angle), center.y - innerRadius * cos((pIndex - 1) * angle)));

				window->DrawList->PathFillConvex(fillColor);
			}
		}

		// outline
		window->DrawList->PathClear();
		for (int i = 0; i < numPoints * 2; i++) {
			float radius = i & 1 ? innerRadius : outerRadius;
			window->DrawList->PathLineTo(ImVec2(center.x + radius * sin(i * angle), center.y - radius * cos(i * angle)));
		}
		window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), true, 2.0f);

		return ret;
	}
	bool FileIcon(const char* label, bool isSelected, ImTextureID icon, ImVec2 size, bool hasPreview, int previewWidth, int previewHeight)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		float windowSpace = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		ImVec2 pos = window->DC.CursorPos;
		bool ret = false;

		if (ImGui::InvisibleButton(label, size))
			ret = true;

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		if (doubleClick && hovered)
			ret = true;


		float iconSize = size.y - g.FontSize * 2;
		float iconPosX = pos.x + (size.x - iconSize) / 2.0f;
		ImVec2 textSize = ImGui::CalcTextSize(label, 0, true, size.x);


		if (hovered || active || isSelected)
			window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : (isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)]));

		if (hasPreview) {
			ImVec2 availSize = ImVec2(size.x, iconSize);

			float scale = std::min<float>(availSize.x / previewWidth, availSize.y / previewHeight);
			availSize.x = previewWidth * scale;
			availSize.y = previewHeight * scale;

			float previewPosX = pos.x + (size.x - availSize.x) / 2.0f;
			float previewPosY = pos.y + (iconSize - availSize.y) / 2.0f;

			window->DrawList->AddImage(icon, ImVec2(previewPosX, previewPosY), ImVec2(previewPosX + availSize.x, previewPosY + availSize.y));
		}
		else
			window->DrawList->AddImage(icon, ImVec2(iconPosX, pos.y), ImVec2(iconPosX + iconSize, pos.y + iconSize));

		window->DrawList->AddText(g.Font, g.FontSize, ImVec2(pos.x + (size.x - textSize.x) / 2.0f, pos.y + iconSize), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label, 0, size.x);


		float lastButtomPos = ImGui::GetItemRectMax().x;
		float thisButtonPos = lastButtomPos + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
		if (thisButtonPos < windowSpace)
			ImGui::SameLine();

		return ret;
	}

	FileDialog::FileData::FileData(const std::filesystem::path& path) {
		std::error_code ec;
		Path = path;
		IsDirectory = std::filesystem::is_directory(path, ec);
		Size = std::filesystem::file_size(path, ec);

		struct stat attr;
		stat(path.u8string().c_str(), &attr);
		DateModified = attr.st_ctime;

		HasIconPreview = false;
		IconPreview = nullptr;
		IconPreviewData = nullptr;
		IconPreviewHeight = 0;
		IconPreviewWidth = 0;
	}

	FileDialog::FileDialog() {
		m_isOpen = false;
		m_type = 0;
		m_calledOpenPopup = false;
		m_sortColumn = 0;
		m_sortDirection = ImGuiSortDirection_Ascending;
		m_filterSelection = 0;
		m_inputTextbox[0] = 0;
		m_pathBuffer[0] = 0;
		m_searchBuffer[0] = 0;
		m_newEntryBuffer[0] = 0;
		m_selectedFileItem = -1;
		m_zoom = 1.0f;

		m_previewLoader = nullptr;
		m_previewLoaderRunning = false;

		m_setDirectory(std::filesystem::current_path(), false);

		// favorites are available on every OS
		FileTreeNode* quickAccess = new FileTreeNode("Quick Access");
		quickAccess->Read = true;
		m_treeCache.push_back(quickAccess);

#ifdef _WIN32
		wchar_t username[UNLEN + 1] = { 0 };
		DWORD username_len = UNLEN + 1;
		GetUserNameW(username, &username_len);

		std::wstring userPath = L"C:\\Users\\" + std::wstring(username) + L"\\";

		// Quick Access / Bookmarks
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Documents"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Pictures"));

		// OneDrive
		FileTreeNode* oneDrive = new FileTreeNode(userPath + L"OneDrive");
		m_treeCache.push_back(oneDrive);

		// This PC
		FileTreeNode* thisPC = new FileTreeNode("This PC");
		thisPC->Read = true;
		if (std::filesystem::exists(userPath + L"3D Objects"))
			thisPC->Children.push_back(new FileTreeNode(userPath + L"3D Objects"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Documents"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Music"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Pictures"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Videos"));
		DWORD d = GetLogicalDrives();
		for (int i = 0; i < 26; i++)
			if (d & (1 << i))
				thisPC->Children.push_back(new FileTreeNode(std::string(1, 'A' + i) + ":"));
		m_treeCache.push_back(thisPC);
#else
		std::error_code ec;

		// Quick Access
		struct passwd* pw;
		uid_t uid;
		uid = geteuid();
		pw = getpwuid(uid);
		if (pw) {
			std::string homePath = "/home/" + std::string(pw->pw_name);

			if (std::filesystem::exists(homePath, ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath));
			if (std::filesystem::exists(homePath + "/Desktop", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Desktop"));
			if (std::filesystem::exists(homePath + "/Documents", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Documents"));
			if (std::filesystem::exists(homePath + "/Downloads", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Downloads"));
			if (std::filesystem::exists(homePath + "/Pictures", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Pictures"));
		}

		// This PC
		FileTreeNode* thisPC = new FileTreeNode("This PC");
		thisPC->Read = true;
		for (const auto& entry : std::filesystem::directory_iterator("/", ec)) {
			if (std::filesystem::is_directory(entry, ec))
				thisPC->Children.push_back(new FileTreeNode(entry.path().u8string()));
		}
		m_treeCache.push_back(thisPC);
#endif
	}
	FileDialog::~FileDialog() {
		m_clearIconPreview();
		m_clearIcons();

		for (auto fn : m_treeCache)
			m_clearTree(fn);
		m_treeCache.clear();
	}
	bool FileDialog::Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir)
	{
		if (!m_currentKey.empty())
			return false;

		m_currentKey = key;
		m_currentTitle = title + "###" + key;
		m_isOpen = true;
		m_calledOpenPopup = false;
		m_result.clear();
		m_inputTextbox[0] = 0;
		m_selections.clear();
		m_selectedFileItem = -1;
		m_isMultiselect = false;
		m_type = IFD_DIALOG_SAVE;

		m_parseFilter(filter);
		if (!startingDir.empty())
			m_setDirectory(std::filesystem::u8path(startingDir), false);
		else
			m_setDirectory(m_currentDirectory, false); // refresh contents

		return true;
	}

	bool FileDialog::Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir)
	{
		if (!m_currentKey.empty())
			return false;

		m_currentKey = key;
		m_currentTitle = title + "###" + key;
		m_isOpen = true;
		m_calledOpenPopup = false;
		m_result.clear();
		m_inputTextbox[0] = 0;
		m_selections.clear();
		m_selectedFileItem = -1;
		m_isMultiselect = isMultiselect;
		m_type = filter.empty() ? IFD_DIALOG_DIRECTORY : IFD_DIALOG_FILE;

		m_parseFilter(filter);
		if (!startingDir.empty())
			m_setDirectory(std::filesystem::u8path(startingDir), false);
		else
			m_setDirectory(m_currentDirectory, false); // refresh contents

		return true;
	}
	bool FileDialog::IsDone(const std::string& key)
	{
		bool isMe = m_currentKey == key;

		if (isMe && m_isOpen) {
			if (!m_calledOpenPopup) {
				ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
				ImGui::OpenPopup(m_currentTitle.c_str());
				m_calledOpenPopup = true;
			}

			if (ImGui::BeginPopupModal(m_currentTitle.c_str(), &m_isOpen, ImGuiWindowFlags_NoScrollbar)) {
				m_renderFileDialog();
				ImGui::EndPopup();
			}
			else m_isOpen = false;
		}

		return isMe && !m_isOpen;
	}
	void FileDialog::Close()
	{
		m_currentKey.clear();
		m_backHistory = std::stack<std::filesystem::path>();
		m_forwardHistory = std::stack<std::filesystem::path>();

		// clear the tree
		for (auto fn : m_treeCache) {
			for (auto item : fn->Children) {
				for (auto ch : item->Children)
					m_clearTree(ch);
				item->Children.clear();
				item->Read = false;
			}
		}

		// free icon textures
		m_clearIconPreview();
		m_clearIcons();
	}

	void FileDialog::RemoveFavorite(const std::string& path)
	{
		auto itr = std::find(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string());

		if (itr != m_favorites.end())
			m_favorites.erase(itr);

		// remove from sidebar
		for (auto& p : m_treeCache)
			if (p->Path == "Quick Access") {
				for (size_t i = 0; i < p->Children.size(); i++)
					if (p->Children[i]->Path == path) {
						p->Children.erase(p->Children.begin() + i);
						break;
					}
				break;
			}
	}
	void FileDialog::AddFavorite(const std::string& path)
	{
		if (std::count(m_favorites.begin(), m_favorites.end(), path) > 0)
			return;

		if (!std::filesystem::exists(std::filesystem::u8path(path)))
			return;

		m_favorites.push_back(path);

		// add to sidebar
		for (auto& p : m_treeCache)
			if (p->Path == "Quick Access") {
				p->Children.push_back(new FileTreeNode(path));
				break;
			}
	}

	void FileDialog::m_select(const std::filesystem::path& path, bool isCtrlDown)
	{
		bool multiselect = isCtrlDown && m_isMultiselect;

		if (!multiselect) {
			m_selections.clear();
			m_selections.push_back(path);
		}
		else {
			auto it = std::find(m_selections.begin(), m_selections.end(), path);
			if (it != m_selections.end())
				m_selections.erase(it);
			else
				m_selections.push_back(path);
		}

		if (m_selections.size() == 1) {
			std::string filename = m_selections[0].filename().u8string();
			if (filename.size() == 0)
				filename = m_selections[0].u8string(); // drive

			strcpy(m_inputTextbox, filename.c_str());
		}
		else {
			std::string textboxVal = "";
			for (const auto& sel : m_selections) {
				std::string filename = sel.filename().u8string();
				if (filename.size() == 0)
					filename = sel.u8string();

				textboxVal += "\"" + filename + "\", ";
			}
			strcpy(m_inputTextbox, textboxVal.substr(0, textboxVal.size() - 2).c_str());
		}
	}

	bool FileDialog::m_finalize(const std::string& filename)
	{
		bool hasResult = (!filename.empty() && m_type != IFD_DIALOG_DIRECTORY) || m_type == IFD_DIALOG_DIRECTORY;

		if (hasResult) {
			if (!m_isMultiselect || m_selections.size() <= 1) {
				std::filesystem::path path = std::filesystem::u8path(filename);
				if (path.is_absolute()) m_result.push_back(path);
				else m_result.push_back(m_currentDirectory / path);
				if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE) {
					if (!std::filesystem::exists(m_result.back())) {
						m_result.clear();
						return false;
					}
				}
			}
			else {
				for (const auto& sel : m_selections) {
					if (sel.is_absolute()) m_result.push_back(sel);
					else m_result.push_back(m_currentDirectory / sel);
					if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE) {
						if (!std::filesystem::exists(m_result.back())) {
							m_result.clear();
							return false;
						}
					}
				}
			}

			if (m_type == IFD_DIALOG_SAVE) {
				// add the extension
				if (m_filterSelection < m_filterExtensions.size() && m_filterExtensions[m_filterSelection].size() > 0) {
					if (!m_result.back().has_extension()) {
						std::string extAdd = m_filterExtensions[m_filterSelection][0];
						m_result.back().replace_extension(extAdd);
					}
				}
			}
		}

		m_isOpen = false;

		return true;
	}
	void FileDialog::m_parseFilter(const std::string& filter)
	{
		m_filter = "";
		m_filterExtensions.clear();
		m_filterSelection = 0;

		if (filter.empty())
			return;

		std::vector<std::string> exts;

		size_t lastSplit = 0, lastExt = 0;
		bool inExtList = false;
		for (size_t i = 0; i < filter.size(); i++) {
			if (filter[i] == ',') {
				if (!inExtList)
					lastSplit = i + 1;
				else {
					exts.push_back(filter.substr(lastExt, i - lastExt));
					lastExt = i + 1;
				}
			}
			else if (filter[i] == '{') {
				std::string filterName = filter.substr(lastSplit, i - lastSplit);
				if (filterName == ".*") {
					m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
					m_filterExtensions.push_back(std::vector<std::string>());
				}
				else
					m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
				inExtList = true;
				lastExt = i + 1;
			}
			else if (filter[i] == '}') {
				exts.push_back(filter.substr(lastExt, i - lastExt));
				m_filterExtensions.push_back(exts);
				exts.clear();

				inExtList = false;
			}
		}
		if (lastSplit != 0) {
			std::string filterName = filter.substr(lastSplit);
			if (filterName == ".*") {
				m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
				m_filterExtensions.push_back(std::vector<std::string>());
			}
			else
				m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
		}
	}

	void* FileDialog::m_getIcon(const std::filesystem::path& path)
	{
#ifdef _WIN32
		if (m_icons.count(path.u8string()) > 0)
			return m_icons[path.u8string()];

		std::string pathU8 = path.u8string();

		std::error_code ec;
		m_icons[pathU8] = nullptr;

		DWORD attrs = 0;
		UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
		if (!std::filesystem::exists(path, ec)) {
			flags |= SHGFI_USEFILEATTRIBUTES;
			attrs = FILE_ATTRIBUTE_DIRECTORY;
		}

		SHFILEINFOW fileInfo = { 0 };
		std::wstring pathW = path.wstring();
		for (int i = 0; i < pathW.size(); i++)
			if (pathW[i] == '/')
				pathW[i] = '\\';
		SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags);

		if (fileInfo.hIcon == nullptr)
			return nullptr;

		// check if icon is already loaded
		auto itr = std::find(m_iconIndices.begin(), m_iconIndices.end(), fileInfo.iIcon);
		if (itr != m_iconIndices.end()) {
			const std::string& existingIconFilepath = m_iconFilepaths[itr - m_iconIndices.begin()];
			m_icons[pathU8] = m_icons[existingIconFilepath];
			return m_icons[pathU8];
		}

		m_iconIndices.push_back(fileInfo.iIcon);
		m_iconFilepaths.push_back(pathU8);

		ICONINFO iconInfo = { 0 };
		GetIconInfo(fileInfo.hIcon, &iconInfo);

		if (iconInfo.hbmColor == nullptr)
			return nullptr;

		DIBSECTION ds;
		GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
		int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

		if (byteSize == 0)
			return nullptr;

		uint8_t* data = (uint8_t*)malloc(byteSize);
		GetBitmapBits(iconInfo.hbmColor, byteSize, data);

		m_icons[pathU8] = this->CreateTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);

		free(data);

		return m_icons[pathU8];
#else
		if (m_icons.count(path.u8string()) > 0)
			return m_icons[path.u8string()];

		std::string pathU8 = path.u8string();

		m_icons[pathU8] = nullptr;

		std::error_code ec;
		int iconID = 1;
		if (std::filesystem::is_directory(path, ec))
			iconID = 0;

		// check if icon is already loaded
		auto itr = std::find(m_iconIndices.begin(), m_iconIndices.end(), iconID);
		if (itr != m_iconIndices.end()) {
			const std::string& existingIconFilepath = m_iconFilepaths[itr - m_iconIndices.begin()];
			m_icons[pathU8] = m_icons[existingIconFilepath];
			return m_icons[pathU8];
		}

		m_iconIndices.push_back(iconID);
		m_iconFilepaths.push_back(pathU8);

		ImVec4 wndBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

		// light theme - load default icons
		if ((wndBg.x + wndBg.y + wndBg.z) / 3.0f > 0.5f) {
			uint8_t* data = (uint8_t*)ifd::GetDefaultFileIcon();
			if (iconID == 0)
				data = (uint8_t*)ifd::GetDefaultFolderIcon();
			m_icons[pathU8] = this->CreateTexture(data, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE, 0);
		}
		// dark theme - invert the colors
		else {
			uint8_t* data = (uint8_t*)ifd::GetDefaultFileIcon();
			if (iconID == 0)
				data = (uint8_t*)ifd::GetDefaultFolderIcon();

			uint8_t* invData = (uint8_t*)malloc(DEFAULT_ICON_SIZE * DEFAULT_ICON_SIZE * 4);
			for (int y = 0; y < 32; y++) {
				for (int x = 0; x < 32; x++) {
					int index = (y * DEFAULT_ICON_SIZE + x) * 4;
					invData[index + 0] = 255 - data[index + 0];
					invData[index + 1] = 255 - data[index + 1];
					invData[index + 2] = 255 - data[index + 2];
					invData[index + 3] = data[index + 3];
				}
			}
			m_icons[pathU8] = this->CreateTexture(invData, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE, 0);

			free(invData);
		}

		return m_icons[pathU8];
#endif
	}
	void FileDialog::m_clearIcons()
	{
		std::vector<unsigned int> deletedIcons;

		// delete textures
		for (auto& icon : m_icons) {
			unsigned int ptr = (unsigned int)((uintptr_t)icon.second);
			if (std::count(deletedIcons.begin(), deletedIcons.end(), ptr)) // skip duplicates
				continue;

			deletedIcons.push_back(ptr);
			DeleteTexture(icon.second);
		}
		m_iconFilepaths.clear();
		m_iconIndices.clear();
		m_icons.clear();
	}
	void FileDialog::m_refreshIconPreview()
	{
		if (m_zoom >= 5.0f) {
			if (m_previewLoader == nullptr) {
				m_previewLoaderRunning = true;
				m_previewLoader = new std::thread(&FileDialog::m_loadPreview, this);
			}
		}
		else
			m_clearIconPreview();
	}
	void FileDialog::m_clearIconPreview()
	{
		m_stopPreviewLoader();

		for (auto& data : m_content) {
			if (!data.HasIconPreview)
				continue;

			data.HasIconPreview = false;
			this->DeleteTexture(data.IconPreview);

			if (data.IconPreviewData != nullptr) {
				stbi_image_free(data.IconPreviewData);
				data.IconPreviewData = nullptr;
			}
		}
	}
	void FileDialog::m_stopPreviewLoader()
	{
		if (m_previewLoader != nullptr) {
			m_previewLoaderRunning = false;

			if (m_previewLoader && m_previewLoader->joinable())
				m_previewLoader->join();

			delete m_previewLoader;
			m_previewLoader = nullptr;
		}
	}
	void FileDialog::m_loadPreview()
	{
		for (size_t i = 0; m_previewLoaderRunning && i < m_content.size(); i++) {
			auto& data = m_content[i];

			if (data.HasIconPreview)
				continue;

			if (data.Path.has_extension()) {
				std::string ext = data.Path.extension().u8string();
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
					int width, height, nrChannels;
					unsigned char* image = stbi_load(data.Path.u8string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

					if (image == nullptr || width == 0 || height == 0)
						continue;

					data.HasIconPreview = true;
					data.IconPreviewData = image;
					data.IconPreviewWidth = width;
					data.IconPreviewHeight = height;
				}
			}
		}

		m_previewLoaderRunning = false;
	}
	void FileDialog::m_clearTree(FileTreeNode* node)
	{
		if (node == nullptr)
			return;

		for (auto n : node->Children)
			m_clearTree(n);

		delete node;
		node = nullptr;
	}
	void FileDialog::m_setDirectory(const std::filesystem::path& p, bool addHistory)
	{
		bool isSameDir = m_currentDirectory == p;

		if (addHistory && !isSameDir)
			m_backHistory.push(m_currentDirectory);

		m_currentDirectory = p;
#ifdef _WIN32
		// drives don't work well without the backslash symbol
		if (p.u8string().size() == 2 && p.u8string()[1] == ':')
			m_currentDirectory = std::filesystem::u8path(p.u8string() + "\\");
#endif

		m_clearIconPreview();
		m_content.clear(); // p == "" after this line, due to reference
		m_selectedFileItem = -1;

		if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
			m_inputTextbox[0] = 0;
		m_selections.clear();

		if (!isSameDir) {
			m_searchBuffer[0] = 0;
			m_clearIcons();
		}

		if (p.u8string() == "Quick Access") {
			for (auto& node : m_treeCache) {
				if (node->Path == p)
					for (auto& c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else if (p.u8string() == "This PC") {
			for (auto& node : m_treeCache) {
				if (node->Path == p)
					for (auto& c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else {
			std::error_code ec;
			if (std::filesystem::exists(m_currentDirectory, ec))
				for (const auto& entry : std::filesystem::directory_iterator(m_currentDirectory, ec)) {
					FileData info(entry.path());

					// skip files when IFD_DIALOG_DIRECTORY
					if (!info.IsDirectory && m_type == IFD_DIALOG_DIRECTORY)
						continue;

					// check if filename matches search query
					if (m_searchBuffer[0]) {
						std::string filename = info.Path.u8string();

						std::string filenameSearch = filename;
						std::string query(m_searchBuffer);
						std::transform(filenameSearch.begin(), filenameSearch.end(), filenameSearch.begin(), ::tolower);
						std::transform(query.begin(), query.end(), query.begin(), ::tolower);

						if (filenameSearch.find(query, 0) == std::string::npos)
							continue;
					}

					// check if extension matches
					if (!info.IsDirectory && m_type != IFD_DIALOG_DIRECTORY) {
						if (m_filterSelection < m_filterExtensions.size()) {
							const auto& exts = m_filterExtensions[m_filterSelection];
							if (exts.size() > 0) {
								std::string extension = info.Path.extension().u8string();

								// extension not found? skip
								if (std::count(exts.begin(), exts.end(), extension) == 0)
									continue;
							}
						}
					}

					m_content.push_back(info);
				}
		}

		m_sortContent(m_sortColumn, m_sortDirection);
		m_refreshIconPreview();
	}
	void FileDialog::m_sortContent(unsigned int column, unsigned int sortDirection)
	{
		// 0 -> name, 1 -> date, 2 -> size
		m_sortColumn = column;
		m_sortDirection = sortDirection;

		// split into directories and files
		std::partition(m_content.begin(), m_content.end(), [](const FileData& data) {
			return data.IsDirectory;
			});

		if (m_content.size() > 0) {
			// find where the file list starts
			size_t fileIndex = 0;
			for (; fileIndex < m_content.size(); fileIndex++)
				if (!m_content[fileIndex].IsDirectory)
					break;

			// compare function
			auto compareFn = [column, sortDirection](const FileData& left, const FileData& right) -> bool {
				// name
				if (column == 0) {
					std::string lName = left.Path.u8string();
					std::string rName = right.Path.u8string();

					std::transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
					std::transform(rName.begin(), rName.end(), rName.begin(), ::tolower);

					int comp = lName.compare(rName);

					if (sortDirection == ImGuiSortDirection_Ascending)
						return comp < 0;
					return comp > 0;
				}
				// date
				else if (column == 1) {
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.DateModified < right.DateModified;
					else
						return left.DateModified > right.DateModified;
				}
				// size
				else if (column == 2) {
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.Size < right.Size;
					else
						return left.Size > right.Size;
				}

				return false;
			};

			// sort the directories
			std::sort(m_content.begin(), m_content.begin() + fileIndex, compareFn);

			// sort the files
			std::sort(m_content.begin() + fileIndex, m_content.end(), compareFn);
		}
	}

	void FileDialog::m_renderTree(FileTreeNode* node)
	{
		// directory
		std::error_code ec;
		ImGui::PushID(node);
		bool isClicked = false;
		std::string displayName = node->Path.stem().u8string();
		if (displayName.size() == 0)
			displayName = node->Path.u8string();
		if (FolderNode(displayName.c_str(), (ImTextureID)m_getIcon(node->Path), isClicked)) {
			if (!node->Read) {
				// cache children if it's not already cached
				if (std::filesystem::exists(node->Path, ec))
					for (const auto& entry : std::filesystem::directory_iterator(node->Path, ec)) {
						if (std::filesystem::is_directory(entry, ec))
							node->Children.push_back(new FileTreeNode(entry.path().u8string()));
					}
				node->Read = true;
			}

			// display children
			for (auto c : node->Children)
				m_renderTree(c);

			ImGui::TreePop();
		}
		if (isClicked)
			m_setDirectory(node->Path);
		ImGui::PopID();
	}
	void FileDialog::m_renderContent()
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			m_selectedFileItem = -1;

		// table view
		if (m_zoom == 1.0f) {
			if (ImGui::BeginTable("##contentTable", 3, /*ImGuiTableFlags_Resizable |*/ ImGuiTableFlags_Sortable, ImVec2(0, -FLT_MIN))) {
				// header
				ImGui::TableSetupColumn("Name##filename", ImGuiTableColumnFlags_WidthStretch, 0.0f - 1.0f, 0);
				ImGui::TableSetupColumn("Date modified##filedate", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 0.0f, 1);
				ImGui::TableSetupColumn("Size##filesize", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 0.0f, 2);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				// sort
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
					if (sortSpecs->SpecsDirty) {
						sortSpecs->SpecsDirty = false;
						m_sortContent(sortSpecs->Specs->ColumnUserID, sortSpecs->Specs->SortDirection);
					}
				}

				// content
				int fileId = 0;
				for (auto& entry : m_content) {
					std::string filename = entry.Path.filename().u8string();
					if (filename.size() == 0)
						filename = entry.Path.u8string(); // drive

					bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

					ImGui::TableNextRow();

					// file name
					ImGui::TableSetColumnIndex(0);
					ImGui::Image((ImTextureID)m_getIcon(entry.Path), ImVec2(ICON_SIZE, ICON_SIZE));
					ImGui::SameLine();
					if (ImGui::Selectable(filename.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
						std::error_code ec;
						bool isDir = std::filesystem::is_directory(entry.Path, ec);

						if (ImGui::IsMouseDoubleClicked(0)) {
							if (isDir) {
								m_setDirectory(entry.Path);
								break;
							}
							else
								m_finalize(filename);
						}
						else {
							if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
								m_select(entry.Path, ImGui::GetIO().KeyCtrl);
						}
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						m_selectedFileItem = fileId;
					fileId++;

					// date
					ImGui::TableSetColumnIndex(1);
					auto tm = std::localtime(&entry.DateModified);
					if (tm != nullptr)
						ImGui::Text("%d/%d/%d %02d:%02d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year, tm->tm_hour, tm->tm_min);
					else ImGui::Text("---");

					// size
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%.3f KiB", entry.Size / 1024.0f);
				}

				ImGui::EndTable();
			}
		}
		// "icon" view
		else {
			// content
			int fileId = 0;
			for (auto& entry : m_content) {
				if (entry.HasIconPreview && entry.IconPreviewData != nullptr) {
					entry.IconPreview = this->CreateTexture(entry.IconPreviewData, entry.IconPreviewWidth, entry.IconPreviewHeight, 1);
					stbi_image_free(entry.IconPreviewData);
					entry.IconPreviewData = nullptr;
				}

				std::string filename = entry.Path.filename().u8string();
				if (filename.size() == 0)
					filename = entry.Path.u8string(); // drive

				bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

				if (FileIcon(filename.c_str(), isSelected, entry.HasIconPreview ? entry.IconPreview : (ImTextureID)m_getIcon(entry.Path), ImVec2(32 + 16 * m_zoom, 32 + 16 * m_zoom), entry.HasIconPreview, entry.IconPreviewWidth, entry.IconPreviewHeight)) {
					std::error_code ec;
					bool isDir = std::filesystem::is_directory(entry.Path, ec);

					if (ImGui::IsMouseDoubleClicked(0)) {
						if (isDir) {
							m_setDirectory(entry.Path);
							break;
						}
						else
							m_finalize(filename);
					}
					else {
						if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
							m_select(entry.Path, ImGui::GetIO().KeyCtrl);
					}
				}
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					m_selectedFileItem = fileId;
				fileId++;
			}
		}
	}
	void FileDialog::m_renderPopups()
	{
		bool openAreYouSureDlg = false, openNewFileDlg = false, openNewDirectoryDlg = false;
		if (ImGui::BeginPopupContextItem("##dir_context")) {
			if (ImGui::Selectable("New file"))
				openNewFileDlg = true;
			if (ImGui::Selectable("New directory"))
				openNewDirectoryDlg = true;
			if (m_selectedFileItem != -1 && ImGui::Selectable("Delete"))
				openAreYouSureDlg = true;
			ImGui::EndPopup();
		}
		if (openAreYouSureDlg)
			ImGui::OpenPopup("Are you sure?##delete");
		if (openNewFileDlg)
			ImGui::OpenPopup("Enter file name##newfile");
		if (openNewDirectoryDlg)
			ImGui::OpenPopup("Enter directory name##newdir");
		if (ImGui::BeginPopupModal("Are you sure?##delete")) {
			if (m_selectedFileItem >= static_cast<int>(m_content.size()) || m_content.size() == 0)
				ImGui::CloseCurrentPopup();
			else {
				const FileData& data = m_content[m_selectedFileItem];
				ImGui::TextWrapped("Are you sure you want to delete %s?", data.Path.filename().u8string().c_str());
				if (ImGui::Button("Yes")) {
					std::error_code ec;
					std::filesystem::remove_all(data.Path, ec);
					m_setDirectory(m_currentDirectory, false); // refresh
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
					ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Enter file name##newfile")) {
			ImGui::PushItemWidth(250.0f);
			ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
			ImGui::PopItemWidth();

			if (ImGui::Button("OK")) {
				std::ofstream out((m_currentDirectory / std::string(m_newEntryBuffer)).string());
				out << "";
				out.close();

				m_setDirectory(m_currentDirectory, false); // refresh
				m_newEntryBuffer[0] = 0;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				m_newEntryBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Enter directory name##newdir")) {
			ImGui::PushItemWidth(250.0f);
			ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
			ImGui::PopItemWidth();

			if (ImGui::Button("OK")) {
				std::error_code ec;
				std::filesystem::create_directory(m_currentDirectory / std::string(m_newEntryBuffer), ec);
				m_setDirectory(m_currentDirectory, false); // refresh
				m_newEntryBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				m_newEntryBuffer[0] = 0;
			}
			ImGui::EndPopup();
		}
	}
	void FileDialog::m_renderFileDialog()
	{
		/***** TOP BAR *****/
		bool noBackHistory = m_backHistory.empty(), noForwardHistory = m_forwardHistory.empty();

		ImGui::PushStyleColor(ImGuiCol_Button, 0);
		if (noBackHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		if (ImGui::ArrowButtonEx("##back", ImGuiDir_Left, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), m_backHistory.empty() * ImGuiItemFlags_Disabled)) {
			std::filesystem::path newPath = m_backHistory.top();
			m_backHistory.pop();
			m_forwardHistory.push(m_currentDirectory);

			m_setDirectory(newPath, false);
		}
		if (noBackHistory) ImGui::PopStyleVar();
		ImGui::SameLine();

		if (noForwardHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		if (ImGui::ArrowButtonEx("##forward", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), m_forwardHistory.empty() * ImGuiItemFlags_Disabled)) {
			std::filesystem::path newPath = m_forwardHistory.top();
			m_forwardHistory.pop();
			m_backHistory.push(m_currentDirectory);

			m_setDirectory(newPath, false);
		}
		if (noForwardHistory) ImGui::PopStyleVar();
		ImGui::SameLine();

		if (ImGui::ArrowButtonEx("##up", ImGuiDir_Up, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
			if (m_currentDirectory.has_parent_path())
				m_setDirectory(m_currentDirectory.parent_path());
		}

		std::filesystem::path curDirCopy = m_currentDirectory;
		if (PathBox("##pathbox", curDirCopy, m_pathBuffer, ImVec2(-250, GUI_ELEMENT_SIZE)))
			m_setDirectory(curDirCopy);
		ImGui::SameLine();

		if (FavoriteButton("##dirfav", std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string()))) {
			if (std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string()))
				RemoveFavorite(m_currentDirectory.u8string());
			else
				AddFavorite(m_currentDirectory.u8string());
		}
		ImGui::SameLine();
		ImGui::PopStyleColor();

		if (ImGui::InputTextEx("##searchTB", "Search", m_searchBuffer, 128, ImVec2(-FLT_MIN, GUI_ELEMENT_SIZE), 0)) // TODO: no hardcoded literals
			m_setDirectory(m_currentDirectory, false); // refresh



		/***** CONTENT *****/
		float bottomBarHeight = (GImGui->FontSize + ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().ItemSpacing.y * 2.0f) * 2;
		if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_Resizable, ImVec2(0, -bottomBarHeight))) {
			ImGui::TableSetupColumn("##tree", ImGuiTableColumnFlags_WidthFixed, 125.0f);
			ImGui::TableSetupColumn("##content", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableNextRow();

			// the tree on the left side
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("##treeContainer", ImVec2(0, -bottomBarHeight));
			for (auto node : m_treeCache)
				m_renderTree(node);
			ImGui::EndChild();

			// content on the right side
			ImGui::TableSetColumnIndex(1);
			ImGui::BeginChild("##contentContainer", ImVec2(0, -bottomBarHeight));
			m_renderContent();
			ImGui::EndChild();
			if (ImGui::IsItemHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
				m_zoom = std::min<float>(25.0f, std::max<float>(1.0f, m_zoom + ImGui::GetIO().MouseWheel));
				m_refreshIconPreview();
			}

			// New file, New directory and Delete popups
			m_renderPopups();

			ImGui::EndTable();
		}



		/***** BOTTOM BAR *****/
		ImGui::Text("File name:");
		ImGui::SameLine();
		if (ImGui::InputTextEx("##file_input", "Filename", m_inputTextbox, 1024, ImVec2((m_type != IFD_DIALOG_DIRECTORY) ? -250.0f : -FLT_MIN, 0), ImGuiInputTextFlags_EnterReturnsTrue)) {
			bool success = m_finalize(std::string(m_inputTextbox));
#ifdef _WIN32
			if (!success)
				MessageBeep(MB_ICONERROR);
#else
			(void)success;
#endif
		}
		if (m_type != IFD_DIALOG_DIRECTORY) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-FLT_MIN);
			int sel = static_cast<int>(m_filterSelection);
			if (ImGui::Combo("##ext_combo", &sel, m_filter.c_str())) {
				m_filterSelection = static_cast<size_t>(sel);
				m_setDirectory(m_currentDirectory, false); // refresh
			}
		}

		// buttons
		float ok_cancel_width = GUI_ELEMENT_SIZE * 7;
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ok_cancel_width);
		if (ImGui::Button(m_type == IFD_DIALOG_SAVE ? "Save" : "Open", ImVec2(ok_cancel_width / 2 - ImGui::GetStyle().ItemSpacing.x, 0.0f))) {
			std::string filename(m_inputTextbox);
			bool success = false;
			if (!filename.empty() || m_type == IFD_DIALOG_DIRECTORY)
				success = m_finalize(filename);
#ifdef _WIN32
			if (!success)
				MessageBeep(MB_ICONERROR);
#else
			(void)success;
#endif
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(-FLT_MIN, 0.0f))) {
			if (m_type == IFD_DIALOG_DIRECTORY)
				m_isOpen = false;
			else
				m_finalize();
		}

		int escapeKey = ImGui::GetIO().KeyMap[ImGuiKey_Escape];
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			escapeKey >= 0 && ImGui::IsKeyPressed(escapeKey))
			m_isOpen = false;
	}
}

static const unsigned int file_icon[] = {
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x4c000000, 0xf5000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xdd000000, 0x2d000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0xd1000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6a000000, 0xa1000000, 0xff000000, 0xff000000, 0x2e000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x54000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x46000000, 0xf5000000, 0xe0000000, 0xff000000, 0x30000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6e000000, 0xf8000000, 0x01000000, 0xc3000000, 0xff000000, 0x30000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00000000, 0x00000000, 0xd2000000, 0xff000000, 0x30000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x13000000, 0x00000000, 0x00000000, 0xd2000000, 0xff000000, 0x30000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x73000000, 0xff000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xbe000000, 0xff000000, 0x30000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x65000000, 0xff000000, 0x34000000, 0x10000000, 0x10000000, 0x03000000, 0x0a000000, 0xdb000000, 0xff000000, 0x2f000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0f000000, 0xd9000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xed000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x06000000, 0x5e000000, 0x6c000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x60000000, 0x9e000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x52000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x54000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x54000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0xd2000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0xd2000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x4c000000, 0xf5000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xf5000000, 0x4b000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
};

static const unsigned int folder_icon[] = {
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00000000, 0x00000000, 0x45000000, 0x8a000000, 0x99000000, 0x97000000, 0x97000000, 0x97000000, 0x97000000, 0x97000000, 0x98000000, 0x81000000, 0x35000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x9e000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x76000000, 0xff000000, 0xff000000, 0xf6000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xff000000, 0xff000000, 0xff000000, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0xe7000000, 0xff000000, 0xbe000000, 0x11000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0xd1000000, 0xff000000, 0xff000000, 0x75000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0xfa000000, 0xff000000, 0x5a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x06000000, 0xe0000000, 0xff000000, 0xff000000, 0x68000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0xf4000000, 0xff000000, 0x67000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x11000000, 0xe4000000, 0xff000000, 0xff000000, 0xad000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x96000000, 0x8b000000, 0x4f000000, 0x00000000, 0x00000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x17000000, 0xe8000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xaf000000, 0x00000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0e000000, 0x88000000, 0xc3000000, 0xcd000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcb000000, 0xcc000000, 0xe2000000, 0xff000000, 0xff000000, 0x81000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xb6000000, 0xff000000, 0xec000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x5b000000, 0xff000000, 0xf9000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x68000000, 0xff000000, 0xf4000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
 0xf4000000, 0xff000000, 0x68000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x68000000, 0xff000000, 0xf4000000,
 0xfa000000, 0xff000000, 0x5a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x5a000000, 0xff000000, 0xf9000000,
 0xea000000, 0xff000000, 0xb5000000, 0x05000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x05000000, 0xb5000000, 0xff000000, 0xea000000,
 0x7e000000, 0xff000000, 0xff000000, 0xeb000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xeb000000, 0xff000000, 0xff000000, 0x7f000000,
 0x00000000, 0xac000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xac000000, 0x00000000,
 0x00000000, 0x00000000, 0x53000000, 0x8f000000, 0x9a000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x9a000000, 0x8f000000, 0x53000000, 0x00000000, 0x00000000,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
};
const char* ifd::GetDefaultFolderIcon()
{
	return (const char*)&folder_icon[0];
}
const char* ifd::GetDefaultFileIcon()
{
	return (const char*)&file_icon[0];
}
#pragma endregion