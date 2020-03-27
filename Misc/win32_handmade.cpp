#include <Windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>

#define M_PI 3.14159265358979323846

struct DisplayBuffer {
	BITMAPINFO info;
	void *memory;
	int width, height, pitch;
};

struct SoundBuffer {
	int samples_per_second;
	int tone_hz;
	int tone_volume;
	int running_sample_index;
	int wave_period;
	int bytes_per_sample;
	int secondary_buffer_size;
	double tsine;
	int latency_sample_count;
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
	RECT ClientRect;

	DWORD PlayCursor;
	DWORD WriteCursor;
	DWORD bytes_to_write;
	DWORD byte_to_lock;
	DWORD TargetCursor;

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

	int xoffset = 0;
	int yoffset = 0;

	SoundBuffer sound_output;
	sound_output.samples_per_second = 48000;
	sound_output.tone_hz = 256;
	sound_output.tone_volume = 3000;
	sound_output.wave_period = sound_output.samples_per_second/sound_output.tone_hz;
	sound_output.bytes_per_sample = sizeof(INT16)*2;
	sound_output.secondary_buffer_size = sound_output.samples_per_second*sound_output.bytes_per_sample;
	sound_output.latency_sample_count = sound_output.samples_per_second/15;

	init_dsound(hwnd, sound_output.samples_per_second, sound_output.secondary_buffer_size);
	fill_sound_buffer(&sound_output, 0, sound_output.latency_sample_count*sound_output.bytes_per_sample);
	secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {
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

				if(Abutton)
					yoffset--;
				if(Bbutton)
					xoffset--;
				if(Xbutton)
					xoffset++;
				if(Ybutton)
					yoffset++;

				sound_output.tone_hz = 512 + (int)(256.0*((float)ystick / 30000.0));
				sound_output.wave_period = sound_output.samples_per_second/sound_output.tone_hz;
			}
		}

		secondary_buffer->GetCurrentPosition(&PlayCursor, &WriteCursor);
		byte_to_lock = ((sound_output.running_sample_index*sound_output.bytes_per_sample) % sound_output.secondary_buffer_size);
		TargetCursor = ((PlayCursor + (sound_output.latency_sample_count*sound_output.bytes_per_sample)) % sound_output.secondary_buffer_size);
		if(byte_to_lock > TargetCursor) {
			bytes_to_write = (sound_output.secondary_buffer_size - byte_to_lock);
			bytes_to_write += TargetCursor;
		} else {
			bytes_to_write = TargetCursor - byte_to_lock;
		}
		fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write);

		fill_display_buffer(&back_buffer, xoffset, yoffset);
		hdc = GetDC(hwnd);
		GetClientRect(hwnd, &ClientRect);
		width = ClientRect.right - ClientRect.left;
		height = ClientRect.bottom - ClientRect.top;
		StretchDIBits(hdc,
			0, 0, width, height,
			0, 0, back_buffer.width, back_buffer.height,
			back_buffer.memory,
			&back_buffer.info,
			DIB_RGB_COLORS,
			SRCCOPY);
		ReleaseDC(hwnd, hdc);
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
	buf->pitch = width*bytes_per_pixel;
}

void fill_display_buffer(DisplayBuffer *back_buffer, int xoffset, int yoffset)
{
	int x, y;
	BYTE *row;
	UINT32 *pixel;
	BYTE blue, green;

	row = (BYTE*)back_buffer->memory;
	for (y = 0; y < back_buffer->height; ++y) {
		pixel = (UINT32*)row;
		for (x = 0; x < back_buffer->width; ++x) {
			blue = x + xoffset;
			green = y + yoffset;
			*pixel++ = ((green << 8) | blue);
		}
		row += back_buffer->pitch;
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
	DWORD region1_sample_count, region2_sample_count;
	INT16 *sample_out;
	DWORD sample_ind, sample_val;
	float sin_val;

	secondary_buffer->Lock(byte_to_lock, bytes_to_write,
						&region1, &region1sz,
						&region2, &region2sz,
						0);

	region1_sample_count = region1sz/buf->bytes_per_sample;
	sample_out = (INT16 *)region1;

	for(sample_ind = 0; sample_ind < region1_sample_count; ++sample_ind) {
		sin_val = sinf(buf->tsine);
		sample_val = (INT16)(sin_val * buf->tone_volume);
		*sample_out++ = sample_val;
		*sample_out++ = sample_val;

		buf->tsine += 2.0*M_PI*1.0/(float)buf->wave_period;
		++buf->running_sample_index;
	}

	region2_sample_count = region2sz/buf->bytes_per_sample;
	sample_out = (INT16 *)region2;

	for(sample_ind = 0; sample_ind < region1_sample_count; ++sample_ind) {
		sin_val = sinf(buf->tsine);
		sample_val = (INT16)(sin_val * buf->tone_volume);
		*sample_out++ = sample_val;
		*sample_out++ = sample_val;

		buf->tsine += 2.0*M_PI*1.0/(float)buf->wave_period;
		++buf->running_sample_index;
	}

	secondary_buffer->Unlock(region1, region1sz, region2, region2sz);
}
