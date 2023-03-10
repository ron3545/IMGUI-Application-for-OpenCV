#pragma once
#pragma warning(disable : 4996)

#include "d3d9.h"
#include "imgui.h"

#include <thread>
#include "../fonts/icons.h"
#include "FaceRecognition.h"
#include <shlobj.h>
#include <iostream>
#include <tchar.h> 
#include "PrivateHeader.h"
#include "DataBase.h"

#include <ctime>
#include <stack>
#include <string>
#include <thread>
#include <vector>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <algorithm> 

#include <oleidl.h>

#include <GL/gl3w.h>            

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define IFD_DIALOG_FILE			0
#define IFD_DIALOG_DIRECTORY	1
#define IFD_DIALOG_SAVE			2

//NDEBUG;_CONSOLE;%(PreprocessorDefinitions) - processor definition

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

std::string charTostring(char* characters);

namespace System
{
	class SystemMenu
	{
	private:
		int optimal_image_width_height, max_image_width, image_width, image_height, radio_v;
		GLuint original_texture_id, modified_texture_id;

		std::vector<std::filesystem::path> image_folder_path;
		std::filesystem::path import_file, export_file;

		ImGuiWindowFlags window_flags;

		const int required_number_images = 5;
		const ImVec4 pressed = to_vec4(119, 136, 153, 255);
		const ImVec4 not_pressed = to_vec4(112, 128, 144, 255);

		cv::VideoCapture capture;
		static cv::Mat frames;  //main frame
		cv::Mat m_image; // for rendering image selected during registry

		jdbc::SQLDataBase DB;
		FaceRecognition FR;

		enum Selector{ Face_Recognition, Object_Detection, Pose_Recognition };
		std::string project_name;

		const std::string tbl_face, tbl_family_info, tbl_unknown_person;
		//for displaying the information about the person that was detected. if NULL, it will display "stranger" on the Information Child window
		std::string Selected_Name; 

		jdbc::RegistryInfo reg_info;
	public:
		SystemMenu(const std::string& project_name);
		~SystemMenu();
		void Menu() noexcept;
		
	private:
		SystemMenu(const SystemMenu&);
		SystemMenu& operator=(const SystemMenu&);

		void Theme();
		bool loginform();

		void UpdateTexture(  cv::Mat images, bool first = false);
		void UpdateTexture(cv::Mat images, GLuint& texture_id, GLuint& modified,int height, int width, bool first = false);

		void get_data_login(jdbc::Account& UD);
		void Login(bool& Connected, bool& continue_, bool& is_pressed, jdbc::Account& UD);
	};
	
	bool ImGui_MessageBox(const char* label, std::string message);
}

namespace ImGui 
{
	void HelpMarker(const char* desc);

	IMGUI_API bool Callendar(const char* label, const char* hint,std::string& buff, int buf_size, ImGuiButtonFlags flags,
		const ImVec2& size = ImVec2(0,0)) noexcept;

	bool ColoredButton(const char* label, const ImVec2& size, ImU32 text_color, 
		ImU32 bg_color_1, ImU32 bg_color_2);
}

namespace ifd {
	class FileDialog {
	public:
		static inline FileDialog& Instance()
		{
			static FileDialog ret;
			return ret;
		}

		FileDialog();
		~FileDialog();

		bool Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir = "");

		bool Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect = false, const std::string& startingDir = "");

		bool IsDone(const std::string& key);

		inline bool HasResult() { return m_result.size(); }
		inline const std::filesystem::path& GetResult() { return m_result[0]; }
		inline const std::vector<std::filesystem::path>& GetResults() { return m_result; }

		void Close();

		void RemoveFavorite(const std::string& path);
		void AddFavorite(const std::string& path);
		inline const std::vector<std::string>& GetFavorites() { return m_favorites; }

		inline void SetZoom(float z) {
			m_zoom = std::min<float>(25.0f, std::max<float>(1.0f, z));
			m_refreshIconPreview();
		}
		inline float GetZoom() { return m_zoom; }

		std::function<void* (uint8_t*, int, int, char)> CreateTexture; // char -> fmt -> { 0 = BGRA, 1 = RGBA }
		std::function<void(void*)> DeleteTexture;

		class FileTreeNode {
		public:
#ifdef _WIN32
			FileTreeNode(const std::wstring& path) {
				Path = std::filesystem::path(path);
				Read = false;
			}
#endif

			FileTreeNode(const std::string& path) {
				Path = std::filesystem::u8path(path);
				Read = false;
			}

			std::filesystem::path Path;
			bool Read;
			std::vector<FileTreeNode*> Children;
		};
		class FileData {
		public:
			FileData(const std::filesystem::path& path);

			std::filesystem::path Path;
			bool IsDirectory;
			size_t Size;
			time_t DateModified;

			bool HasIconPreview;
			void* IconPreview;
			uint8_t* IconPreviewData;
			int IconPreviewWidth, IconPreviewHeight;
		};

	private:
		std::string m_currentKey;
		std::string m_currentTitle;
		std::filesystem::path m_currentDirectory;
		bool m_isMultiselect;
		bool m_isOpen;
		uint8_t m_type;
		char m_inputTextbox[1024];
		char m_pathBuffer[1024];
		char m_newEntryBuffer[1024];
		char m_searchBuffer[128];
		std::vector<std::string> m_favorites;
		bool m_calledOpenPopup;
		std::stack<std::filesystem::path> m_backHistory, m_forwardHistory;
		float m_zoom;

		std::vector<std::filesystem::path> m_selections;
		int m_selectedFileItem;
		void m_select(const std::filesystem::path& path, bool isCtrlDown = false);

		std::vector<std::filesystem::path> m_result;
		bool m_finalize(const std::string& filename = "");

		std::string m_filter;
		std::vector<std::vector<std::string>> m_filterExtensions;
		size_t m_filterSelection;
		void m_parseFilter(const std::string& filter);

		std::vector<int> m_iconIndices;
		std::vector<std::string> m_iconFilepaths; // m_iconIndices[x] <-> m_iconFilepaths[x]
		std::unordered_map<std::string, void*> m_icons;
		void* m_getIcon(const std::filesystem::path& path);
		void m_clearIcons();
		void m_refreshIconPreview();
		void m_clearIconPreview();

		std::thread* m_previewLoader;
		bool m_previewLoaderRunning;
		void m_stopPreviewLoader();
		void m_loadPreview();

		std::vector<FileTreeNode*> m_treeCache;
		void m_clearTree(FileTreeNode* node);
		void m_renderTree(FileTreeNode* node);

		unsigned int m_sortColumn;
		unsigned int m_sortDirection;
		std::vector<FileData> m_content;
		void m_setDirectory(const std::filesystem::path& p, bool addHistory = true);
		void m_sortContent(unsigned int column, unsigned int sortDirection);
		void m_renderContent();

		void m_renderPopups();
		void m_renderFileDialog();
	};
}