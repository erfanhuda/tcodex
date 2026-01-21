#include <windows.h>
#include <stdio.h>
#include <time.h>

#define MAX_EVENTS 10000
#define MAX_INTERFACES 10

typedef enum { EVENT_MOUSE, EVENT_KEY } EventType;

typedef struct {
    EventType type;
    int x;           // mouse x (relative)
    int y;           // mouse y (relative)
    DWORD vkCode;    // for key events
    char keyName[64];
    time_t timestamp;
} LogEvent;

typedef struct {
    LogEvent events[MAX_EVENTS];
    int eventCount;

    NetInterfaceState interfaces[MAX_INTERFACES];
    int interfaceCount;
} ProgramLog;
