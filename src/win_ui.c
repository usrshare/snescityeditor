#include "win_ui.h"
#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
}

int win_ui_main(void) {

	HMODULE hInstance = GetModuleHandle(NULL);

	char filename[MAX_PATH];

	char* openFilters[] = {
		"SNES SRAM files","*.srm\0\0"};

	OPENFILENAME x = {

		.lStructSize = sizeof x,
		.hInstance = hInstance,
		.lpstrFilter = openFilters,
		.lpstrFile = filename,
		.nMaxFile = MAX_PATH,
	};
	
	int r = GetOpenFileName(&x);

	MessageBoxA(0,"Windows UI not implemented yet.","Sorry",0);
	return 0;
}
