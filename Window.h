#pragma once

#include <windows.h>
#include <tchar.h>

typedef struct {
	int window_w;
	int window_h;
	int window_exit;
	int window_keys[512];	// 当前键盘按下状态
	HWND window_handle;		// 主窗口 HWND
	HDC window_dc;			// 配套的 HDC
	HBITMAP window_hb;		// DIB
	HBITMAP window_ob;		// 老的 BITMAP
	unsigned char *window_fb;		// frame buffer
	long window_pitch;
} window_t;

//关闭窗口
inline int window_close(window_t* window) {
	if (window->window_dc) {
		if (window->window_ob) {
			SelectObject(window->window_dc, window->window_ob);
			window->window_ob = NULL;
		}
		DeleteDC(window->window_dc);
		window->window_dc = NULL;
	}
	if (window->window_hb) {
		DeleteObject(window->window_hb);
		window->window_hb = NULL;
	}
	if (window->window_handle) {
		CloseWindow(window->window_handle);
		window->window_handle = NULL;
	}
	return 0;
}

//检查窗口是否运行中
inline int window_running(window_t* window) {
	return window->window_exit == 0 && window->window_keys[VK_ESCAPE] == 0;
}

//分发窗口消息
inline void window_dispatch(window_t* window) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

// 初始化窗口并设置标题
inline int window_init(window_t* window, int w, int h, const TCHAR *title, WNDPROC window_events) {
	window->window_w = 0;
	window->window_h = 0;
	window->window_exit = 0;
	window->window_handle = NULL;// 主窗口 HWND
	window->window_dc = NULL;	// 配套的 HDC
	window->window_hb = NULL;	// DIB
	window->window_ob = NULL;	// 老的 BITMAP
	window->window_fb = NULL;	// frame buffer
	window->window_pitch = 0;

	WNDCLASS wc = { CS_BYTEALIGNCLIENT, window_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("window3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	window_close(window);

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	window->window_handle = CreateWindow(_T("window3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (window->window_handle == NULL) return -2;

	window->window_exit = 0;
	hDC = GetDC(window->window_handle);
	window->window_dc = CreateCompatibleDC(hDC);
	ReleaseDC(window->window_handle, hDC);

	window->window_hb = CreateDIBSection(window->window_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (window->window_hb == NULL) return -3;

	window->window_ob = (HBITMAP)SelectObject(window->window_dc, window->window_hb);
	window->window_fb = (unsigned char*)ptr;
	window->window_w = w;
	window->window_h = h;
	window->window_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(window->window_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(window->window_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(window->window_handle);

	ShowWindow(window->window_handle, SW_NORMAL);
	window_dispatch(window);

	memset(window->window_keys, 0, sizeof(int) * 512);
	memset(window->window_fb, 0, w * h * 4);

	return 0;
}

//更新窗口的图像数据
inline void window_update(window_t* window) {
	HDC hDC = GetDC(window->window_handle);
	BitBlt(hDC, 0, 0, window->window_w, window->window_h, window->window_dc, 0, 0, SRCCOPY);
	ReleaseDC(window->window_handle, hDC);
	window_dispatch(window);
}

//检查窗口按键
inline int window_check(window_t*window, int key) {
	return window->window_keys[key];
}