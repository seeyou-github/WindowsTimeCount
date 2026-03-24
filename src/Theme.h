#pragma once

#include <windows.h>
#include <string>

namespace Theme {

HFONT CreateUiFont(int height, int weight = FW_NORMAL, const wchar_t* faceName = L"Microsoft YaHei");
HFONT CreateMonoFont(int height, int weight = FW_BOLD, const wchar_t* faceName = L"Consolas");
void FillSolidRect(HDC dc, const RECT& rect, COLORREF color);
void DrawCenteredText(HDC dc, const RECT& rect, const std::wstring& text, COLORREF color, HFONT font);

}  // namespace Theme
