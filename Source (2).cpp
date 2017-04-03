#include <Windows.h>
#include <stdint.h>

struct OffscreenBuffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
};

static OffscreenBuffer global_back_buffer;

void get_window_dimension(HWND wndw, LONG &width, LONG &height)
{
	RECT client_rect;
	GetClientRect(wndw, &client_rect);
	width = client_rect.right - client_rect.left;
	height = client_rect.bottom - client_rect.top;
}

LRESULT CALLBACK main_callback(HWND wndw, UINT msg, WPARAM wp, LPARAM lp);

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_line, int show_code)
{
	WNDCLASSEX wc = {};
	HWND wndw;
	HDC dc;
	OffscreenBuffer back_buffer = {};
	int bytes_per_pixel;
	int bitmap_memory_sz;
	int width, height;

	// resize dib section
	if (back_buffer.memory)
		VirtualFree(back_buffer.memory, 0, MEM_RELEASE);

	back_buffer.width = 1280;
	back_buffer.height = 720;

	back_buffer.info.bmiHeader.biSize = sizeof(back_buffer.info.bmiHeader);
	back_buffer.info.bmiHeader.biWidth = back_buffer.width;
	back_buffer.info.bmiHeader.biHeight = -back_buffer.height; // negative to make top down bitmap
	back_buffer.info.bmiHeader.biPlanes = 1;
	back_buffer.info.bmiHeader.biBitCount = 32;
	back_buffer.info.bmiHeader.biCompression = BI_RGB;

	bytes_per_pixel = 4;
	bitmap_memory_sz = back_buffer.width*back_buffer.height*bytes_per_pixel;
	back_buffer.memory = VirtualAlloc(0, bitmap_memory_sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// probably clear this to black

	back_buffer.pitch = back_buffer.width*bytes_per_pixel;
	// end resize dib section

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = main_callback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = inst;
	// wc.hIcon = NULL; // change icon later
	// wc.hCursor = NULL;
	// wc.hbrBackground = NULL;
	// wc.lpszMenuName = NULL;
	wc.lpszClassName = "ClassName";
	// wc.hIconSm = NULL;

	RegisterClassExA(&wc);

	wndw = CreateWindowExA(
		0,
		wc.lpszClassName,
		"WindowName",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		inst,
		0);

	if (!wndw)
		return 0;

	dc = GetDC(wndw);

	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// render wierd gradient

	RECT client_rect;
	GetClientRect(wndw, &client_rect);
	width = client_rect.right - client_rect.left;
	height = client_rect.bottom - client_rect.top;

	StretchDIBits(dc,
		0, 0, width, height,
		0, 0, back_buffer.width, back_buffer.height,
		back_buffer.memory,
		&back_buffer.info,
		DIB_RGB_COLORS,
		SRCCOPY);

	return msg.wParam;
}

LRESULT CALLBACK main_callback(HWND wndw, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT result = 0;

	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT paint;
		HDC dc = BeginPaint(wndw, &paint);

		RECT client_rect;
		GetClientRect(wndw, &client_rect);
		int width = client_rect.right - client_rect.left;
		int height = client_rect.bottom - client_rect.top;

		StretchDIBits(dc,
			0, 0, width, height,
			0, 0, back_buffer.width, back_buffer.height,
			back_buffer.memory,
			&back_buffer.info,
			DIB_RGB_COLORS,
			SRCCOPY);
		EndPaint(wndw, &paint);
	} break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		switch (wp) {
		case 'W':
			break;
		case 'A':
			break;
		case 'S':
			break;
		case 'D':
			break;
		}
		break;
	default:
		result = DefWindowProc(wndw, msg, wp, lp);
	}

	return result;
}