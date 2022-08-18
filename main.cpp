#include "Header Files/window.h"

int _stdcall wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow)
{
	CVGUI gui;
	std::string cascadefile = "D:\\opencv\\build\\install\\etc\\haarcascades\\haarcascade_frontalface_alt.xml";
	std::string folder_save = " ";

	gui.CreateHWindow();
	if (!gui.CreateDevice()) {
		gui.DestroyDevice();
		UnregisterClass(gui.GetWindowClass().lpszClassName, gui.GetWindowClass().hInstance);
		return 1;
	}
	ShowWindow(gui.GetWindow(), SW_SHOWDEFAULT);
	UpdateWindow(gui.GetWindow());
	gui.CreateImGui();

	MENU::Theme();

	while (gui.Begin())
	{
		gui.BeginRender();
		gui.Render();
		gui.EndRender();
	}

	gui.DestroyImGui();
	gui.DestroyDevice();
	gui.DestroyHWindow();

	return EXIT_SUCCESS;
}