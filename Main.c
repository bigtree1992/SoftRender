#include "window.h"
#include "Math.h"
#include "Device.h"

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif


static window_t g_window;
static device_t g_device;

static LRESULT window_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: g_window.window_exit = 1; break;
	case WM_KEYDOWN: g_window.window_keys[wParam & 511] = 1; break;
	case WM_KEYUP: g_window.window_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

vertex_t mesh[8] = {
	{ { 1, -1,  1, 1 },{ 0, 0 },{ 1.0f, 0.2f, 0.2f }, 1 },
	{ { -1, -1,  1, 1 },{ 0, 1 },{ 0.2f, 1.0f, 0.2f }, 1 },
	{ { -1,  1,  1, 1 },{ 1, 1 },{ 0.2f, 0.2f, 1.0f }, 1 },
	{ { 1,  1,  1, 1 },{ 1, 0 },{ 1.0f, 0.2f, 1.0f }, 1 },
	{ { 1, -1, -1, 1 },{ 0, 0 },{ 1.0f, 1.0f, 0.2f }, 1 },
	{ { -1, -1, -1, 1 },{ 0, 1 },{ 0.2f, 1.0f, 1.0f }, 1 },
	{ { -1,  1, -1, 1 },{ 1, 1 },{ 1.0f, 0.3f, 0.3f }, 1 },
	{ { 1,  1, -1, 1 },{ 1, 0 },{ 0.2f, 1.0f, 0.3f }, 1 },
};

static void draw_plane(device_t *device, int a, int b, int c, int d) {
	vertex_t p1 = mesh[a], p2 = mesh[b], p3 = mesh[c], p4 = mesh[d];
	p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
	p3.tc.u = 1, p3.tc.v = 1, p4.tc.u = 1, p4.tc.v = 0;
	device_draw_primitive(device, &p1, &p2, &p3);
	device_draw_primitive(device, &p3, &p4, &p1);
}

static void draw_box(device_t *device, float theta) {
	matrix_t m;
	matrix_set_rotate(&m, -1, -0.5, 1, theta);
	device->transform.world = m;
	transform_update(&device->transform);
	draw_plane(device, 0, 1, 2, 3);
	draw_plane(device, 4, 5, 6, 7);
	draw_plane(device, 0, 4, 5, 1);
	draw_plane(device, 1, 5, 6, 2);
	draw_plane(device, 2, 6, 7, 3);
	draw_plane(device, 3, 7, 4, 0);
}

int main(void)
{
	TCHAR *title = _T("SoftRender");
	window_t* window = &g_window;
	device_t* device = &g_device;

	float alpha = 1;
	float pos = 3.5;
	
	//初始化窗口
	if (window_init(window, 800, 600, title, (WNDPROC)window_events))
		return -1;
	//再初始化渲染设备,渲染设备只关心一个渲染的buffer,不需要关心其他细节
	device_init(device, 800, 600, window->window_fb);
	
	//开始渲染主循环
	while (window_running(window)){
		window_dispatch(window);

		device_clear(device, 1);
		device_set_camera(device, pos, 0, 0);

		if (window_check(window,VK_UP)) pos -= 0.01f;
		if (window_check(window, VK_DOWN)) pos += 0.01f;
		if (window_check(window, VK_LEFT)) alpha += 0.01f;
		if (window_check(window, VK_RIGHT)) alpha -= 0.01f;

		device_change_state(device,window_check(window, VK_SPACE));
		
		draw_box(device, alpha);

		window_update(window);
		Sleep(1);
	}

	return 0;
}