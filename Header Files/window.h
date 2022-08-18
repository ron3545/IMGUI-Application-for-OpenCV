#pragma once
#include <cassert>

#include "../fonts/iconcpp.h"
#include "../fonts/font.h"
#include "d3d9.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <thread>
#include "../fonts/iconcpp.h"
#include "Menu.h"

#include "libraries.h"
#include "FaceRecognition.h"


#pragma region WINDOWS_HANDLER
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND	window,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
);

#pragma endregion

#pragma region OPENCV_GUI
class CVGUI
{
private:
	const int WIDTH;
	const int HEIGHT;
	bool continue_;
	ImFont* font;
	HWND window;

	WNDCLASSEX windowClass;

	POINTS position;

	//direct x special vars
	static  PDIRECT3D9				d3d ;
	static  LPDIRECT3DDEVICE9		device ;
	static  D3DPRESENT_PARAMETERS	presentParameters ;

	cv::Mat frames;
	cv::VideoCapture cap;
public:
	CVGUI() : WIDTH(900), HEIGHT(430), continue_(true), font(0),
		window(nullptr), windowClass({ }), position({ }),
		frames({ }), cap(0), FamInfo({}), acc({}), DB({})
	{}

	void CreateHWindow() noexcept;
	void DestroyHWindow() noexcept;

	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

	//Getter
	inline WNDCLASSEX GetWindowClass()	const	{ return windowClass;	}
	inline HWND GetWindow()				const	{ return window;		}
	inline bool Begin()					const	{ return continue_;		}
	inline int GetWidth()				const	{ return WIDTH;			}
	inline int GetHeight()				const	{ return HEIGHT;		}
	//mutable getters
	inline POINTS& GetPosition()				{ return position;		}

private:

	//independent function
	static LRESULT WINAPI  WindowProcess(	HWND window,	UINT message,
											WPARAM wParam,	LPARAM lParam	);

	LRESULT WINAPI  WindowProc(HWND window, UINT message,
		WPARAM wParam, LPARAM lParam);

	jdbc::SQLDataBase DB;
	jdbc::Account acc;
	jdbc::Family FamInfo;
};


#pragma endregion
