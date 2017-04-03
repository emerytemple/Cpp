#include <Windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

struct DisplayBuffer {
	BITMAPINFO info;
	void *memory;
	int width, height;
};

struct SoundBuffer {
	int samples_per_second;
	int frequency; // tone hz
	int amplitude; // volume
	int running_sample_index;
	int samples_per_cycle;
	int bytes_per_sample;
	int secondary_buffer_size;
	float tsine;
	// int latency_sample_count;
};

static DisplayBuffer back_buffer;
static LPDIRECTSOUNDBUFFER secondary_buffer;

LRESULT CALLBACK main_callback(HWND wndw, UINT msg, WPARAM wp, LPARAM lp);
void init_dib(DisplayBuffer *buf, int width, int height);
void fill_display_buffer(DisplayBuffer *buf, int xoffset, int yoffset);
void init_dsound(HWND hwnd, int samples_per_sec, int buf_sz);
void fill_sound_buffer(SoundBuffer *buf, DWORD byte_to_lock, DWORD bytes_to_write);

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_line, int show_code)
{
	HWND hwnd;
	MSG msg;

	DWORD controller_index;
	XINPUT_STATE controller_state;
	XINPUT_GAMEPAD *pad;
	WORD up, down, left, right;
	WORD start, back;
	WORD Lthumb, Rthumb, Lshoulder, Rshoulder;
	WORD Abutton, Bbutton, Xbutton, Ybutton;
	SHORT xstick, ystick;
	int width, height;
	HDC hdc;
	RECT client_rect;
	int xoffset, yoffset;

	SoundBuffer sound_output;
	DWORD play_cursor;
	DWORD write_cursos;
	DWORD bytes_to_write;
	DWORD byte_to_lock;
	DWORD target_cursor;

	char buf[256];
	LARGE_INTEGER start_time, end_time, elapsed_time;
	LARGE_INTEGER count_frequency;
	double mspf, fps;
	unsigned int start_cycle_count, end_cycle_count, elapsed_cycles;

	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		(WNDPROC)main_callback,
		0, 0,
		inst,
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(BLACK_BRUSH),
		0,
		"ClassName"
	};

	RegisterClass(&wc);

	hwnd = CreateWindow(wc.lpszClassName, "WindowName",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, inst, NULL);

	if (!hwnd)
		return 0;

	init_dib(&back_buffer, 1280, 720);

	ShowWindow(hwnd, show_code);
	UpdateWindow(hwnd);

	xoffset = 0;
	yoffset = 0;

	sound_output.samples_per_second = 48000;
	sound_output.frequency = 256; // cycles/second
	sound_output.amplitude = 3000;
	sound_output.running_sample_index = 0;
	sound_output.samples_per_cycle = sound_output.samples_per_second / sound_output.frequency;
	sound_output.bytes_per_sample = sizeof(INT16) * 2;
	sound_output.secondary_buffer_size = sound_output.samples_per_second*sound_output.bytes_per_sample;
	sound_output.tsine = 0;
	// sound_output.latency_sample_count = sound_output.samples_per_second / 15;

	init_dsound(hwnd, sound_output.samples_per_second, sound_output.secondary_buffer_size);
	fill_sound_buffer(&sound_output, 0, sound_output.secondary_buffer_size); // sound_output.latency_sample_count*sound_output.bytes_per_sample
	secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

	QueryPerformanceFrequency(&count_frequency);

	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {
		QueryPerformanceCounter(&start_time);
		start_cycle_count = __rdtsc();

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for (controller_index = 0; controller_index < XUSER_MAX_COUNT; ++controller_index) {
			if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
				pad = &controller_state.Gamepad;

				up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
				down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
				left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
				right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
				start = (pad->wButtons & XINPUT_GAMEPAD_START);
				back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
				Lthumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
				Rthumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
				Lshoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				Rshoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				Abutton = (pad->wButtons & XINPUT_GAMEPAD_A);
				Bbutton = (pad->wButtons & XINPUT_GAMEPAD_B);
				Xbutton = (pad->wButtons & XINPUT_GAMEPAD_X);
				Ybutton = (pad->wButtons & XINPUT_GAMEPAD_Y);

				xstick = pad->sThumbLX;
				ystick = pad->sThumbLY;

				// we will do deadzone handling later
				xoffset -= xstick / 4096;
				yoffset += ystick / 4096;

				if (Abutton)
					yoffset--;
				if (Bbutton)
					xoffset--;
				if (Xbutton)
					xoffset++;
				if (Ybutton)
					yoffset++;

				sound_output.frequency = 512 + (int)(256.0*((float)ystick / 30000.0));
				sound_output.samples_per_cycle = sound_output.samples_per_second / sound_output.frequency;
			}
		}

		secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursos);
		byte_to_lock = ((sound_output.running_sample_index*sound_output.bytes_per_sample) % sound_output.secondary_buffer_size);
		target_cursor = ((play_cursor + sound_output.secondary_buffer_size) % sound_output.secondary_buffer_size); // sound_output.latency_sample_count*sound_output.bytes_per_sample for first one
		if (byte_to_lock > target_cursor) {
			bytes_to_write = (sound_output.secondary_buffer_size - byte_to_lock);
			bytes_to_write += target_cursor;
		}
		else {
			bytes_to_write = target_cursor - byte_to_lock;
		}
		fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write);

		fill_display_buffer(&back_buffer, xoffset, yoffset);
		hdc = GetDC(hwnd);
		GetClientRect(hwnd, &client_rect);
		width = client_rect.right - client_rect.left;
		height = client_rect.bottom - client_rect.top;
		StretchDIBits(hdc,
			0, 0, width, height,
			0, 0, back_buffer.width, back_buffer.height,
			back_buffer.memory,
			&back_buffer.info,
			DIB_RGB_COLORS,
			SRCCOPY);
		ReleaseDC(hwnd, hdc);

		end_cycle_count = __rdtsc();
		QueryPerformanceCounter(&end_time);
		elapsed_time.QuadPart = end_time.QuadPart - start_time.QuadPart;

		fps = (double)count_frequency.QuadPart / (double)elapsed_time.QuadPart;

		elapsed_time.QuadPart *= 1000; // convert to milliseconds
		mspf = (double)elapsed_time.QuadPart / (double)count_frequency.QuadPart; // convert to milliseconds per frame

		elapsed_cycles = end_cycle_count - start_cycle_count;
		double MCPF = (double)elapsed_cycles / (1000.0 * 1000.0);

		// fps * MCPF is the GHz for the processor

		sprintf_s(buf, "%.02f ms/f,  %.02f f/s,  %.02f mc/f\n", mspf, fps, MCPF);
		OutputDebugStringA(buf);

		start_time = end_time;
		start_cycle_count = end_cycle_count;
	}

	return msg.wParam;
}

LRESULT CALLBACK main_callback(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	LRESULT result = 0;

	static int wclient, hclient;
	// static int xoffset, yoffset;

	/* TCHAR buf[80];
	TEXTMETRIC tm;
	static int cxChar, cxCaps, cyChar;
	int ilen; */

	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_CREATE:
		/* hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc); */

		// xoffset = 0;
		// yoffset = 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (wp) {
		case VK_UP:
		case 'W':
			// yoffset += 10;
			// RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case VK_LEFT:
		case 'A':
			// xoffset += 10;
			// RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case VK_DOWN:
		case 'S':
			// yoffset -= 10;
			// RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case VK_RIGHT:
		case 'D':
			// xoffset -= 10;
			// RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// ilen = wsprintf(buf, "w = %i, h = %i", xoffset, yoffset);
		// TextOut(hdc, 0, 0, buf, ilen);
		// fill_display_buffer(&back_buffer, xoffset, yoffset);
		StretchDIBits(hdc,
			0, 0, wclient, hclient,
			0, 0, back_buffer.width, back_buffer.height,
			back_buffer.memory,
			&back_buffer.info,
			DIB_RGB_COLORS,
			SRCCOPY);
		EndPaint(hwnd, &ps);
		break;
	case WM_SIZE:
		wclient = LOWORD(lp);
		hclient = HIWORD(lp);
		break;
	default:
		result = DefWindowProc(hwnd, msg, wp, lp);
	}

	return result;
}

void init_dib(DisplayBuffer *buf, int width, int height)
{
	int bytes_per_pixel = 4;

	if (buf->memory)
		VirtualFree(buf->memory, 0, MEM_RELEASE);

	buf->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	buf->info.bmiHeader.biWidth = width;
	buf->info.bmiHeader.biHeight = -height; // negative for top-down
	buf->info.bmiHeader.biPlanes = 1;
	buf->info.bmiHeader.biBitCount = 32; // 32/8 = 4, 4 bytes/pixel
	buf->info.bmiHeader.biCompression = BI_RGB;
	buf->info.bmiHeader.biSizeImage = 0;
	buf->info.bmiHeader.biXPelsPerMeter = 0;
	buf->info.bmiHeader.biYPelsPerMeter = 0;
	buf->info.bmiHeader.biClrUsed = 0;
	buf->info.bmiHeader.biClrImportant = 0;

	buf->memory = VirtualAlloc(0, width*height * bytes_per_pixel, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	buf->width = width;
	buf->height = height;
}

void fill_display_buffer(DisplayBuffer *back_buffer, int xoffset, int yoffset)
{
	int x, y;
	BYTE *row;
	UINT32 *pixel;
	BYTE blue, green;
	int bytes_per_pixel = 4;

	row = (BYTE*)back_buffer->memory;
	for (y = 0; y < back_buffer->height; ++y) {
		pixel = (UINT32*)row;
		for (x = 0; x < back_buffer->width; ++x) {
			blue = x + xoffset;
			green = y + yoffset;
			*pixel++ = ((green << 8) | blue);
		}
		row += back_buffer->width*bytes_per_pixel; // row += pitch;
	}
}

void init_dsound(HWND hwnd, int samples_per_second, int buf_sz)
{
	WAVEFORMATEX wfx;
	DSBUFFERDESC dsbdesc;
	LPDIRECTSOUND direct_sound;
	LPDIRECTSOUNDBUFFER primary_buffer;

	// set up wave format structure
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = samples_per_second;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.nChannels*wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nBlockAlign;
	wfx.cbSize = 0;

	// set up DSBUFFERDESC structure
	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.dwReserved = 0;
	dsbdesc.lpwfxFormat = NULL;
	dsbdesc.guid3DAlgorithm = GUID_NULL;

	DirectSoundCreate(0, &direct_sound, 0);

	direct_sound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	direct_sound->CreateSoundBuffer(&dsbdesc, &primary_buffer, 0);
	primary_buffer->SetFormat(&wfx);

	dsbdesc.dwFlags = 0;
	dsbdesc.dwBufferBytes = buf_sz;
	dsbdesc.lpwfxFormat = &wfx;

	direct_sound->CreateSoundBuffer(&dsbdesc, &secondary_buffer, 0);
}

void fill_sound_buffer(SoundBuffer *buf, DWORD byte_to_lock, DWORD bytes_to_write)
{
	VOID *region1, *region2;
	DWORD region1sz, region2sz;
	INT16 *sample_out;
	DWORD sample_ind, sample_val;

	secondary_buffer->Lock(byte_to_lock, bytes_to_write,
		&region1, &region1sz,
		&region2, &region2sz,
		0);

	sample_out = (INT16 *)region1;
	for (sample_ind = 0; sample_ind < region1sz / buf->bytes_per_sample; ++sample_ind) { // max is region1_sample_count
		sample_val = (INT16)(sinf(buf->tsine) * buf->amplitude);
		*sample_out++ = sample_val;
		*sample_out++ = sample_val;

		buf->tsine += 2.0*M_PI*1.0 / (float)buf->samples_per_cycle;
		++buf->running_sample_index;
	}

	sample_out = (INT16 *)region2;
	for (sample_ind = 0; sample_ind < region2sz / buf->bytes_per_sample; ++sample_ind) {
		sample_val = (INT16)(sinf(buf->tsine) * buf->amplitude);
		*sample_out++ = sample_val;
		*sample_out++ = sample_val;

		buf->tsine += 2.0*M_PI*1.0 / (float)buf->samples_per_cycle;
		++buf->running_sample_index;
	}

	secondary_buffer->Unlock(region1, region1sz, region2, region2sz);
}
