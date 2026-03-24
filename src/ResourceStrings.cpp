#include "ResourceStrings.h"

std::wstring LoadStringResource(HINSTANCE instance, int stringId) {
    const wchar_t* rawText = nullptr;
    const int length = LoadStringW(instance, stringId, reinterpret_cast<LPWSTR>(&rawText), 0);
    if (length <= 0 || rawText == nullptr) {
        return L"";
    }

    return std::wstring(rawText, rawText + length);
}
