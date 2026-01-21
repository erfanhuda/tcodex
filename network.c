#include "network.h"

int GetNetworkInterfaces(NetInterfaceState* interfaces, int maxCount)
{
    DWORD size = 0;
    DWORD ret = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &size);
    if (ret != ERROR_BUFFER_OVERFLOW)
        return 0;

    IP_ADAPTER_ADDRESSES* adapter = (IP_ADAPTER_ADDRESSES*)malloc(size);
    if (!adapter) return 0;

    ret = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapter, &size);
    if (ret != NO_ERROR) {
        free(adapter);
        return 0;
    }

    int count = 0;
    IP_ADAPTER_ADDRESSES* a = adapter;
    while (a && count < maxCount) {
        strncpy_s(interfaces[count].name, sizeof(interfaces[count].name), a->AdapterName, _TRUNCATE);
        strncpy_s(interfaces[count].description, sizeof(interfaces[count].description), a->Description, _TRUNCATE);
        interfaces[count].type = a->IfType;
        interfaces[count].operationalStatus = a->OperStatus;

        /* IPv4 address */
        IP_ADAPTER_UNICAST_ADDRESS* ua = a->FirstUnicastAddress;
        interfaces[count].ipv4[0] = '\0';
        interfaces[count].ipv6[0] = '\0';
        while (ua) {
            if (ua->Address.lpSockaddr->sa_family == AF_INET) {
                struct sockaddr_in* sa = (struct sockaddr_in*)ua->Address.lpSockaddr;
                inet_ntop(AF_INET, &sa->sin_addr, interfaces[count].ipv4, sizeof(interfaces[count].ipv4));
            } else if (ua->Address.lpSockaddr->sa_family == AF_INET6) {
                struct sockaddr_in6* sa6 = (struct sockaddr_in6*)ua->Address.lpSockaddr;
                inet_ntop(AF_INET6, &sa6->sin6_addr, interfaces[count].ipv6, sizeof(interfaces[count].ipv6));
            }
            ua = ua->Next;
        }

        count++;
        a = a->Next;
    }

    free(adapter);
    return count;
}