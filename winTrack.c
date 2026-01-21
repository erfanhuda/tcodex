#include "win.h"

static POINT startPos;

void PrintKeyName(WPARAM vk, LPARAM lParam)
{
    char keyName[64] = {0};
    
    UINT scanCode = (lParam >> 16) & 0xFF;
    LONG lparam = (scanCode << 16);
    
    if (GetKeyNameTextA(lparam, keyName, sizeof(keyName))) {

        // LowLevellpts(lParam);
        printf("KEY         %d (VK=0x%02X)\n", keyName, (unsigned int) vk);
    }
    
    else {
        printf("KEY         VK=0x%02X\n", (unsigned int) vk);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Get mouse position at program start
        GetCursorPos(&startPos);
        printf("Start position set to (0,0)\n");
        break;
    }

    case WM_MOUSEMOVE:
    {
        POINT current;
        GetCursorPos(&current);

        int x = current.x - startPos.x;
        int y = current.y - startPos.y;

        printf("%d,%d\n", x, y);
        break;
    }

    case WM_KEYDOWN:
        PrintKeyName(wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


int main(void)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "MouseKeyboardTracker";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        "Mouse + Keyboard Tracker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 300,
        NULL, NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    SetFocus(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}