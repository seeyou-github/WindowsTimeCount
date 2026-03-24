#pragma once

#include <windows.h>
#include <string>

class OverlayWindow {
public:
    explicit OverlayWindow(HINSTANCE instance);
    ~OverlayWindow();

    bool Create(HWND owner);
    void Show();
    void Hide();
    void UpdateTime(const std::wstring& text, int remainingSeconds, int totalSeconds);

private:
    static constexpr const wchar_t* kClassName = L"WindowsTimeCountOverlayWindow";

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void Paint(HDC dc);
    void RegisterWindowClass();
    void UpdateLayout();

    HINSTANCE instance_;
    HWND hwnd_;
    std::wstring timeText_;
    int remainingSeconds_;
    int totalSeconds_;
};
