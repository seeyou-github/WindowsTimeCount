#pragma once

#include <windows.h>
#include <string>

class AlertWindow {
public:
    explicit AlertWindow(HINSTANCE instance);
    ~AlertWindow();

    void Show(HWND owner, const std::wstring& title, const std::wstring& message, const std::wstring& buttonText);

private:
    static constexpr const wchar_t* kClassName = L"WindowsTimeCountAlertWindow";
    static constexpr int kButtonId = 4001;

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void RegisterWindowClass();
    void Paint(HDC dc);
    void Layout();

    HINSTANCE instance_;
    HWND hwnd_;
    HWND button_;
    std::wstring title_;
    std::wstring message_;
    std::wstring buttonText_;
};
