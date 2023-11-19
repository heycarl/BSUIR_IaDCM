#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <ctime>
#include <thread>
#include <Windows.h>
#include "camera.hpp"

using namespace cv;
using namespace std;
HHOOK hHookKeyboard;
HWND hWnd;

string get_current_time() {
	time_t timetoday;
	time(&timetoday);
	string time = asctime(localtime(&timetoday));
	time.erase(time.size() - 1);
	replace(time.begin(), time.end(), ' ', '_');
	replace(time.begin(), time.end(), ':', '-');
	return time.substr();
}

LRESULT CALLBACK keyboard_hook_handle(int , WPARAM , LPARAM );

int main()
{
	hWnd = GetForegroundWindow();
	long style= GetWindowLong(hWnd, GWL_STYLE);
	style &= ~(WS_VISIBLE);    // this works - window become invisible
	SetWindowLong(hWnd, GWL_STYLE, style);
	ShowWindow(hWnd, SW_SHOW);
	main_func();
	std::cout << "ready" << endl;
	hHookKeyboard = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_hook_handle, GetModuleHandle(nullptr), 0);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

DWORD WINAPI photo_capture_thread(LPVOID lpParam)
{
	VideoCapture cap(0);
	cap.set(3, 1920);
	cap.set(3, 1080);
	if (!cap.isOpened()) {
		cerr << "Error with camera" << endl;
		return -1;
	}
	Mat frame;
	cap >> frame;
	string image_name = get_current_time() + ".jpg";
	cout << image_name << endl;
	if (!imwrite(image_name, frame)) {
		cout << "Photo does not saved" << endl;
		return -1;
	} else {
		return 0;
	}
}


LRESULT CALLBACK keyboard_hook_handle(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool window_is_hidden = false;
	auto* ks = (KBDLLHOOKSTRUCT*)lParam;
	if (ks->vkCode == 0x1B)								 // esc
	{
		TerminateProcess(GetCurrentProcess(), NO_ERROR);
	}

	if (ks->vkCode == 0x50 && (ks->flags & 0x80) == 0) // P - photo
	{
		CreateThread(nullptr, 0, photo_capture_thread, nullptr, 0, nullptr);
	}

	if (ks->vkCode == 0x48 && (ks->flags & 0x80) == 0)	// H - hide
	{
		if (window_is_hidden)
		{
			ShowWindow(hWnd, SW_SHOW);
			window_is_hidden = false;
		}
		else
		{
			ShowWindow(hWnd, SW_HIDE);
			window_is_hidden = true;
		}
		return -1;
	}

	return CallNextHookEx(hHookKeyboard, nCode, wParam, lParam);
}

