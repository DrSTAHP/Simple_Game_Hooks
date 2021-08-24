#pragma once
#include <Windows.h>

extern int window_height, window_width;

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);

HWND GetProcessWindow();