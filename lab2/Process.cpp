#include "Process.h"
#include "Mode.h"

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

bool Process::Init(HINSTANCE hInstance, string wndTitle, string wndName, int width, int height, bool debug_mode) {
	keyboard.EnableAutoRepeatKeys();
	return rendWnd.Init(this, hInstance, wndTitle, wndName, width, height) && graph.Init(rendWnd.GetHwnd(), width, height) 
		&& keyProc.Init(&keyboard, &graph) && mouseProc.Init(&mouse, &graph);
}

void Process::Refresh() {
	keyProc.Process();
	mouseProc.Process();
}

bool Process::ResizeWnd(size_t width, size_t height) {
	Mode::GetUserAnnotation().BegEvent(L"Resize window");
	bool res = graph.ResizeWnd(width, height);
	Mode::GetUserAnnotation().EndEvent();
	return res;
}

bool Process::GetNewMessages() {
	return rendWnd.GetNewMessages();
}

void Process::RenderFrame() {
	graph.RenderFrame();
}