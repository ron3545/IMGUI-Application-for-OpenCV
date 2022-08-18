#include "..\Header Files\window.h"

using namespace std;
using namespace cv;

PDIRECT3D9 CVGUI::d3d = NULL;
LPDIRECT3DDEVICE9 CVGUI::device = NULL;
D3DPRESENT_PARAMETERS CVGUI::presentParameters = { };

#pragma region WND_PROC
//https://devblogs.microsoft.com/oldnewthing/20140203-00/?p=1893
LRESULT WINAPI CVGUI::WindowProcess(HWND window, UINT message, 
	WPARAM wParam, LPARAM lParam)
{
	CVGUI* pthis; //"this" pointer will go here
	if (message == WM_NCCREATE) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pthis = static_cast<CVGUI*>(lpcs->lpCreateParams);
		// Put the value in a safe place for future use
		SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pthis));
	}
	else {
		pthis = reinterpret_cast<CVGUI*>(GetWindowLongPtr(window, GWLP_USERDATA));
	}
	if (pthis)
		return pthis->WindowProc(window, message, wParam, lParam);
	return DefWindowProc(window, message, wParam, lParam);
}

LRESULT WINAPI CVGUI::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_SIZE:
		if (device != NULL && wParam != SIZE_MINIMIZED)
		{
			presentParameters.BackBufferWidth = LOWORD(lParam);
			presentParameters.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN: {
		GetPosition() = MAKEPOINTS(lParam);
	}return 0;

	case WM_MOUSEMOVE: {
		if (wParam == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(lParam);
			auto rect = ::RECT{ };

			GetWindowRect(window, &rect);

			rect.left += points.x - GetPosition().x;
			rect.top += points.y - GetPosition().y;

			if (GetPosition().x >= 0 && GetPosition().x <= GetWidth() && GetPosition().y >= 0 && GetPosition().y <= 19)
				SetWindowPos(window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
		}
	}return 0;

	}

	return DefWindowProc(window, message, wParam, lParam);
}
#pragma endregion

#pragma region OPENCV_GUI-DEFINITION
#pragma managed(push, off)
void CVGUI::CreateHWindow() noexcept 
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = CVGUI::WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = _T("Thesis");
	windowClass.hIconSm = 0;

	::RegisterClassEx(&windowClass);

	window = CreateWindow(_T("Thesis"), _T("Threath Detection"), WS_OVERLAPPEDWINDOW, 100, 100,
		WIDTH, HEIGHT, NULL, NULL, windowClass.hInstance, this);

}

bool CVGUI::CreateDevice() noexcept 
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == NULL)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferWidth = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0) return false;

	return true;
}

void CVGUI::ResetDevice() noexcept 
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT result = device->Reset(&presentParameters);
	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

void CVGUI::CreateImGui() noexcept 
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;

	}

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	//load font
	static const ImWchar icon_range[] = { 0xf000,0xf3ff,0 };
	ImFontConfig config_icons, Custom_Font;
	Custom_Font.FontDataOwnedByAtlas = false;

	config_icons.MergeMode = true;
	config_icons.PixelSnapH = true;
	config_icons.OversampleH = 2;
	config_icons.OversampleV = 2;

	io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 21.f, &Custom_Font);
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.0f, &config_icons, icon_range);
	font = io.Fonts->AddFontFromFileTTF("D:\\Imgui application\\MyApplication\\misc\\fonts\\Cousine-Regular.ttf", 17);
	io.Fonts->AddFontDefault();
	IM_ASSERT(font != NULL);
}

void CVGUI::BeginRender() noexcept 
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
		if (message.message == WM_QUIT)
			continue_ = false;
	}
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	device->Present(NULL, NULL, NULL, NULL);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CVGUI::Render() noexcept 
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoSavedSettings;

	
	std::string name;


		//optimize full viewport 
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		//main window
		static bool is_connected = false;
		if (is_connected) {
			ImGui::Begin("Threath Detection", &continue_, window_flags);
			{
				//ImGui::ShowDemoWindow();
				System::Menu(FamInfo);
			}
			ImGui::End();
		}
		else
			is_connected = System::loginform(DB);
}

void CVGUI::EndRender() noexcept 
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

void CVGUI::DestroyImGui() noexcept 
{
	ImGui::DestroyPlatformWindows();
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void CVGUI::DestroyDevice() noexcept
{
	if (device) {
		device->Release();
		device = nullptr;
	}

	if (d3d) {
		d3d->Release();
		d3d = nullptr;
	}
}

void CVGUI::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}
#pragma endregion