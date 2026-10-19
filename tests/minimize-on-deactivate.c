/* A screen-covering window that minimizes itself whenever the application
 * loses the activation, as Source games do. It logs every activation change
 * and size change to minimize-on-deactivate.log in the current directory,
 * so a run shows whether Wine let it minimize and whether it was brought
 * back. Esc or 60 seconds end it. */

#include <windows.h>
#include <stdio.h>

static FILE *log_file;

static void log_event(HWND hwnd, const char *what)
{
    SYSTEMTIME t;

    GetLocalTime(&t);
    fprintf(log_file, "%02u:%02u:%02u.%03u %s iconic=%d foreground=%d\n", t.wHour, t.wMinute, t.wSecond,
            t.wMilliseconds, what, IsIconic(hwnd), GetForegroundWindow() == hwnd);
    fflush(log_file);
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_ACTIVATEAPP:
        log_event(hwnd, wparam ? "activated" : "deactivated");
        if (!wparam)
            ShowWindow(hwnd, SW_MINIMIZE);
        return 0;
    case WM_SIZE:
        log_event(hwnd, wparam == SIZE_MINIMIZED ? "size: minimized" : "size: restored");
        break;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
            DestroyWindow(hwnd);
        break;
    case WM_TIMER:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int show)
{
    WNDCLASSA wc = {0};
    HWND hwnd;
    MSG msg;

    if (!(log_file = fopen("minimize-on-deactivate.log", "a")))
        return 1;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(32, 48, 64));
    wc.lpszClassName = "minimize-on-deactivate";
    RegisterClassA(&wc);
    hwnd = CreateWindowExA(0, wc.lpszClassName, "minimize-on-deactivate", WS_POPUP | WS_VISIBLE, 0, 0,
                           GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, instance, NULL);
    log_event(hwnd, "created");
    SetTimer(hwnd, 1, 60000, NULL);
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    log_event(hwnd, "exit");
    fclose(log_file);
    return 0;
}
