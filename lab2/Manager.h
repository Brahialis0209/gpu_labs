#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "Loger.h"
#include "Graphics/Graphics.h"

using namespace std;

class Manager;

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class RenderWindow {
public:
	bool Init(Manager* window_container_ptr, HINSTANCE hInstance, string title, string name, int width, int height);
	bool GetNewMessages();
	HWND GetHwnd() const;
	~RenderWindow();

private:
	bool InitWndClass();
	HWND hwnd = NULL;
	HINSTANCE hInst = NULL;
	string titleWnd = "";
	wstring wideTitle = L"";
	string nameWndClass = "";
	wstring wideName = L"";
	int wndWidth = 0;
	int wndHeight = 0;
};

class Manager {
public:
	Manager();
	LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual bool ResizeWnd(size_t width, size_t height) { return true; };
	MouseButtons mouse;
	Graphics graph;
	KeyboardKeys keyboard;
	RenderWindow rendWnd;
	
private:
};

