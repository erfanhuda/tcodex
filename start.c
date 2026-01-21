#pragma once
#define WIN32_LEAN_AND_MEAN
#include "win.h"
#include "network.h"
#include "log.h"

static POINT startPos;
static int initialized = 0;

HHOOK mouseHook = NULL;
HHOOK keyboardHook = NULL;

ProgramLog gLog = {0};

FILE* logFile = NULL;

void AddMouseEvent(int x, int y)
{
    if (!logFile) return;

    time_t t = time(NULL);
    char timeStr[32];
    struct tm* tm_info = localtime(&t);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logFile, "[%s] MOUSE x=%d y=%d\n", timeStr, x, y);
    fflush(logFile); // write immediately
}

void AddKeyEvent(DWORD vkCode, const char* keyName)
{
    if (!logFile) return;

    time_t t = time(NULL);
    char timeStr[32];
    struct tm* tm_info = localtime(&t);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logFile, "[%s] KEY %s (VK=0x%02X)\n", timeStr, keyName, vkCode);
    fflush(logFile);
}


void SaveNetworkInterfaces(NetInterfaceState* interfaces, int count)
{
    gLog.interfaceCount = (count > MAX_INTERFACES) ? MAX_INTERFACES : count;
    for (int i = 0; i < gLog.interfaceCount; i++)
    {
        gLog.interfaces[i] = interfaces[i]; // struct copy
    }
}

void WriteLogFile(const char* filename)
{
    FILE* f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "=== Mouse & Keyboard Events ===\n");
    for (int i = 0; i < gLog.eventCount; i++)
    {
        LogEvent* e = &gLog.events[i];
        char timeStr[32];
        struct tm* tm_info = localtime(&e->timestamp);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

        if (e->type == EVENT_MOUSE)
            fprintf(f, "[%s] MOUSE x=%d y=%d\n", timeStr, e->x, e->y);
        else
            fprintf(f, "[%s] KEY %s (VK=0x%02X)\n", timeStr, e->keyName, e->vkCode);
    }

    fprintf(f, "\n=== Network Interfaces ===\n");
    for (int i = 0; i < gLog.interfaceCount; i++)
    {
        NetInterfaceState* n = &gLog.interfaces[i];
        fprintf(f, "Name: %s\nDesc: %s\nType: %lu\nStatus: %lu\nIPv4: %s\nIPv6: %s\n\n",
                n->name, n->description, n->type, n->operationalStatus, n->ipv4, n->ipv6);
    }

    fclose(f);
}


/* Convert integer to binary string */
void IntToBinary(int value, char* buffer, int bits)
{
    buffer[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--)
    {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

/* Convert VK code to human-readable name */
void PrintKeyBinary(DWORD vkCode, DWORD scanCode)
{
    char keyName[64] = {0};
    LONG lParamKey = (scanCode << 16);
    if (GetKeyNameTextA(lParamKey, keyName, sizeof(keyName)) == 0)
        snprintf(keyName, sizeof(keyName), "VK=0x%02X", vkCode);

    char binary[9]; // 8-bit binary for VK code
    IntToBinary(vkCode, binary, 8);

    printf("KEY   %s | BINARY=%s\n", keyName, binary);
    AddKeyEvent(vkCode, keyName);
}

/* Convert Virtual Keyboard code to human-readable name (manusia banget) */
void PrintKeyName(DWORD vkCode, DWORD scanCode)
{
    char keyName[64] = {0};
    LONG lParamKey = (scanCode << 16);
    if (GetKeyNameTextA(lParamKey, keyName, sizeof(keyName)))
    {
        printf("KEY      %s (VK=0x%02X)\n", keyName, vkCode);
    }
    else {
        printf("KEY      (VK=0x%02X)\n", keyName, vkCode);
    }

    AddKeyEvent(vkCode, keyName);
}

/* Keyboard hook callback */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        DWORD kbCode = kb->vkCode;
        DWORD scanCode = kb->scanCode;

        PrintKeyBinary(kbCode, scanCode);

        // printf("KEY VK=0x%02X\n", kb->vkCode);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

/* Mouse hook callback */
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;

        if (!initialized)
        {
            startPos.x = ms->pt.x;
            startPos.y = ms->pt.y;
            initialized = 1;
            printf("Start position set to (0,0)\n");
        }

        if (wParam == WM_MOUSEMOVE)
        {
            int x = ms->pt.x - startPos.x;
            int y = ms->pt.y - startPos.y;

            char xbin[33], ybin[33]; // 32-bit binary
            IntToBinary(x, xbin, 32);
            IntToBinary(y, ybin, 32);

            printf("MOUSE x=%d,y=%d | BINARY x=%s y=%s \n", x, y, xbin, ybin);

            AddMouseEvent(x, y);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int startTracker(void)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    printf("Global mouse and keyboard tracker started.\n");

    // Open log file in append mode
    logFile = fopen("program_log.txt", "a");
    if (!logFile)
    {
        printf("Failed to open log file.\n");
        return 1;
    }

    /* Install global hooks */
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    if (!mouseHook || !keyboardHook)
    {
        printf("Failed to install hooks.\n");
        if (logFile) fclose(logFile);
        return 1;
    }

    /* Message loop is required for hooks to work */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Unhook before exit */
    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);

    if (logFile) fclose(logFile);
    return 0;
}

int startLookInterfaces() {
    NetInterfaceState interfaces[10];
    int count = GetNetworkInterfaces(interfaces, 10);
    SaveNetworkInterfaces(interfaces, count);

    for (int i = 0; i < count; i++) {
        printf("Name: %s\n", interfaces[i].name);
        printf("Desc: %s\n", interfaces[i].description);
        printf("Type: %lu\n", interfaces[i].type);
        printf("Status: %lu\n", interfaces[i].operationalStatus);
        printf("IPv4: %s\n", interfaces[i].ipv4);
        printf("IPv6: %s\n\n", interfaces[i].ipv6);
    }

    WriteLogFile("network_log.txt");

    return 0;
}


int main()
{
    startTracker();
}