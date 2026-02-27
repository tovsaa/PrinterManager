#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <winspool.h>
#include <vector>
#include <string>
#include <math.h>

#pragma comment(lib, "winspool.lib")


std::vector<std::wstring> GetPrinterNames() {
    std::vector<std::wstring> printerNames;
    DWORD needed = 0, returned = 0;

    if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                      nullptr, 2, nullptr, 0, &needed, &returned))
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return printerNames;

    std::vector<BYTE> buffer(needed);

    if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                      nullptr, 2, buffer.data(), needed, &needed, &returned))
        return printerNames;

    PRINTER_INFO_2* printers = reinterpret_cast<PRINTER_INFO_2*>(buffer.data());

    for (DWORD i = 0; i < returned; ++i)
        if (printers[i].pPrinterName != nullptr)
            printerNames.emplace_back(printers[i].pPrinterName);

    return printerNames;
}

std::vector<std::pair<std::wstring, WORD>> GetPaperFormatsWithCodes(const wchar_t* printerName) {
    std::vector<std::pair<std::wstring, WORD>> result;

    DWORD paperCount = DeviceCapabilities(printerName, nullptr, DC_PAPERNAMES, nullptr, nullptr);
    if (paperCount <= 0)
        return result;

    const DWORD NAME_SIZE = 64;
    std::vector<wchar_t> namesBuffer(paperCount * NAME_SIZE);
    DWORD retNames = DeviceCapabilities(printerName, nullptr, DC_PAPERNAMES,
                                        reinterpret_cast<LPTSTR>(namesBuffer.data()), nullptr);
    if (retNames <= 0)
        return result;

    std::vector<WORD> codes(paperCount);
    DWORD retCodes = DeviceCapabilities(printerName, nullptr, DC_PAPERS,
                                        reinterpret_cast<LPTSTR>(codes.data()), nullptr);
    if (retCodes <= 0)
        return result;

    DWORD count = fmin(retNames, retCodes);
    result.reserve(count);
    for (DWORD i = 0; i < count; ++i) {
        const wchar_t* name = namesBuffer.data() + i * NAME_SIZE;
        WORD code = codes[i];
        result.emplace_back(name, code);
    }

    return result;
}

std::vector<std::pair<std::wstring, WORD>> GetColorModesWithCodes(const wchar_t* printerName) {
    std::vector<std::pair<std::wstring, WORD>> result;

    DWORD isColor = DeviceCapabilities(printerName, nullptr, DC_COLORDEVICE, nullptr, nullptr);
    if (isColor == (DWORD)-1)
        return result;

    if (isColor) {
        result.emplace_back(L"COLOR", DMCOLOR_COLOR);
        result.emplace_back(L"MONOCHROME", DMCOLOR_MONOCHROME);
    } else {
        result.emplace_back(L"MONOCHROME", DMCOLOR_MONOCHROME);
    }

    return result;
}
