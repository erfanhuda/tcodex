#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

typedef struct _NetInterfaceState {
    char name[256];          // Interface name
    char description[256];   // Description
    DWORD type;              // IF_TYPE (Ethernet, Wi-Fi, etc.)
    DWORD operationalStatus; // 1=up, 2=down, etc.
    char ipv4[16];           // IPv4 address as string
    char ipv6[64];           // IPv6 address as string
} NetInterfaceState;