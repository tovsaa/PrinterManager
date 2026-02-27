#include "userip.h"
#include <windows.h>
#include <iphlpapi.h>
#include <string>
#include <cstring>

#pragma comment(lib, "iphlpapi.lib")


std::string GetLocalIP() {
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    PIP_ADAPTER_INFO pAdapterInfo = nullptr;
    std::string result;

    dwRetVal = GetAdaptersInfo(nullptr, &dwSize);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(dwSize);
        if (pAdapterInfo) {
            dwRetVal = GetAdaptersInfo(pAdapterInfo, &dwSize);
            if (dwRetVal == NO_ERROR) {
                PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
                while (pAdapter) {
                    if (pAdapter->IpAddressList.IpAddress.String[0] != '0' &&
                        strcmp(pAdapter->IpAddressList.IpAddress.String, "127.0.0.1") != 0) {
                        result = pAdapter->IpAddressList.IpAddress.String;
                        break;
                    }
                    pAdapter = pAdapter->Next;
                }
            }
            free(pAdapterInfo);
        }
    }
    return result;
}
