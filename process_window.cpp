#include "process_window.h"

static HWND window;

extern int window_height = 1080;
extern int window_width = 1920;

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);

    if (GetCurrentProcessId() != wndProcId)
        return TRUE; // skip to next window

    window = handle;
    return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
    window = NULL;
    EnumWindows(EnumWindowsCallback, NULL);

    RECT size;
    GetWindowRect(window, &size);
    window_width = size.right - size.left;
    window_height = size.bottom - size.top;

    return window;
}