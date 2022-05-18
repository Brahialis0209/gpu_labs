#include <memory>

#include "Manager.h"

Manager::Manager()
{
	static bool raw_input_initialized = false;
	if (!raw_input_initialized)
	{
		RAWINPUTDEVICE rid;

		rid.usUsagePage = 0x01; // this means mouse
		rid.usUsage = 0x02; // this means mouse
		rid.dwFlags = 0;
		rid.hwndTarget = NULL;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			utils::Loger::Log(GetLastError(), "Failed register raw input device.");
			exit(-1);
		}
	}
	raw_input_initialized = true;
}

LRESULT Manager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// Keyboard messages
	case WM_CHAR:
	{
		uint8_t ch = static_cast<uint8_t>(wParam);
		if (keyboard.IsCharsAutoRepeat())
		{
			keyboard.OnChar(ch);
		}
		else
		{
			bool const was_pressed = lParam & 0x40000000;
			if (!was_pressed)
			{
				keyboard.OnChar(ch);
			}
		}
		return 0;
	}
	case WM_KEYDOWN:
	{
		uint8_t keycode = static_cast<uint8_t>(wParam);
		if (keyboard.IsKeysAutoRepeat())
		{
			keyboard.OnKeyPressed(keycode);
		}
		else
		{
			bool const was_pressed = lParam & 0x40000000;
			if (!was_pressed)
			{
				keyboard.OnKeyPressed(keycode);
			}
		}
		return 0;
	}
	case WM_KEYUP:
	{
		uint8_t keycode = static_cast<uint8_t>(wParam);
		keyboard.OnKeyReleased(keycode);
		return 0;
	}
	// Mouse messages
	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.MouseMove(x, y);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.LeftPressed(x, y);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.LeftReleased(x, y);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.RightPressed(x, y);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.RightReleased(x, y);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.MiddlePressed(x, y);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.MiddleReleased(x, y);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			mouse.WHEEL_UP(x, y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			mouse.WHEEL_DOWN(x, y);
		}
		return 0;
	}
	case WM_INPUT:
	{
		UINT data_size = 0;
		HRAWINPUT raw_input = reinterpret_cast<HRAWINPUT>(lParam);
		GetRawInputData(raw_input, RID_INPUT, NULL, &data_size, sizeof(RAWINPUTHEADER));

		if (data_size > 0)
		{
			unique_ptr<BYTE[]> rawdata = make_unique<BYTE[]>(data_size);
			if (GetRawInputData(raw_input, RID_INPUT, rawdata.get(), &data_size, sizeof(RAWINPUTHEADER)) == data_size)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					mouse.MouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}


		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	// Resize Window
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		if (!ResizeWnd(width, height))
		{
			PostQuitMessage(1);
		}
		return 0;
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}



bool RenderWindow::Init(Manager* window_container_ptr,
	HINSTANCE hInstance, string wndTitle,
	string wndName, int width, int height)
{
	hInst = hInstance;
	wndWidth = width;
	wndHeight = height;
	titleWnd = wndTitle;
	wideTitle = wstring(wndTitle.begin(), wndTitle.end());
	nameWndClass = wndName;
	wideName = wstring(wndName.begin(), wndName.end());

	if (!InitWndClass()) {
		utils::Loger::Log("Failed register window class.");
		return false;
	}

	RECT wr; // Window Rectangle
	wr.left = 50;
	wr.top = 50;
	wr.right = wr.left + width;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(0,
		wideName.c_str(),
		wideTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		wr.left,
		wr.top,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInst,
		window_container_ptr);

	if (hwnd == NULL) {
		utils::Loger::Log(GetLastError(), "CreateWindowEX failed for window: " + titleWnd);
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);

	return true;
}

bool RenderWindow::InitWndClass() {
	WNDCLASSEX wndClass;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = HandleMessageSetup;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInst;
	wndClass.hIcon = NULL;
	wndClass.hIconSm = NULL;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = NULL;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = wideName.c_str();
	wndClass.cbSize = sizeof(WNDCLASSEX);
	return RegisterClassEx(&wndClass);
}

bool RenderWindow::GetNewMessages() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL) {
		if (!IsWindow(hwnd)) {
			hwnd = NULL;
			UnregisterClass(wideName.c_str(), hInst);
			return false;
		}
	}

	return true;
}

HWND RenderWindow::GetHwnd() const
{
	return hwnd;
}

RenderWindow::~RenderWindow()
{
	if (hwnd != NULL)
	{
		UnregisterClass(wideName.c_str(), hInst);
		DestroyWindow(hwnd);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		return 0;
	}
	default:
		// retrieve window container ptr
		Manager* window_container_ptr = reinterpret_cast<Manager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		// forward message to window container handler
		return window_container_ptr->WndProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		CREATESTRUCTW const* pointer_create = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Manager* window_container_ptr = reinterpret_cast<Manager*>(pointer_create->lpCreateParams);
		if (!window_container_ptr)
		{
			utils::Loger::Log("Critical Error: Pointer of Manager is null during WM_NCCREATE");
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_container_ptr));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return window_container_ptr->WndProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}