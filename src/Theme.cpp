#include "Theme.h"

namespace Theme {

HFONT CreateUiFont(int height, int weight, const wchar_t* faceName) {
    return CreateFontW(
        height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, faceName);
}

HFONT CreateMonoFont(int height, int weight, const wchar_t* faceName) {
    return CreateFontW(
        height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH, faceName);
}

void FillSolidRect(HDC dc, const RECT& rect, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
}

void DrawCenteredText(HDC dc, const RECT& rect, const std::wstring& text, COLORREF color, HFONT font) {
    const int oldMode = SetBkMode(dc, TRANSPARENT);
    const COLORREF oldColor = SetTextColor(dc, color);
    HGDIOBJ oldFont = SelectObject(dc, font);

    DrawTextW(dc, text.c_str(), -1, const_cast<RECT*>(&rect),
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    SelectObject(dc, oldFont);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldMode);
}

}  // namespace Theme
