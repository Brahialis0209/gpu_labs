#pragma once
#include "Manager.h"
#include "Keyboard.h"
#include "Mouse.h"

class Process : Manager {
public:
	bool Init(HINSTANCE hInstance, string wndTitle, string wndName, int width, int height, bool dbgMode = false);
	void Refresh();
	bool GetNewMessages();
	void RenderFrame();

private:
	virtual bool ResizeWnd(size_t width, size_t height) final;

	KeyboardProcessor keyProc;
	Mouse mouseProc;
};
