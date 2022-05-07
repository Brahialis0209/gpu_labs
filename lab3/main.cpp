#include "Process.h"
#include "Mode.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,	_In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int argCount;

	LPWSTR* argList = CommandLineToArgvW(lpCmdLine, &argCount);
	if (argCount > 0) {
		wstring arg = argList[0];
		if (arg == L"DEBUG")
			Mode::ReturnExample().dbgMode = true;
		else
			Mode::ReturnExample().dbgMode = false;
	}
	else
		Mode::ReturnExample().dbgMode = false;

	Process prc;
	if (prc.Init(hInstance, "Square", "lab_3", 1200, 800, false)) {
		while (prc.GetNewMessages() == true) {
			prc.Refresh();
			prc.RenderFrame();
		}
	}
	return 0;
}