#ifndef PRINTER_UTILS_H
#define PRINTER_UTILS_H

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
#include <utility>

#pragma comment(lib, "winspool.lib")


std::vector<std::wstring> GetPrinterNames();
std::vector<std::pair<std::wstring, WORD>> GetPaperFormatsWithCodes(const wchar_t* printerName);
std::vector<std::pair<std::wstring, WORD>> GetColorModesWithCodes(const wchar_t* printerName);

#endif
