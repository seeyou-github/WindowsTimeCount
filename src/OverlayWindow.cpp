#include "OverlayWindow.h"

#include <algorithm>

#include "AppConfig.h"
#include "Theme.h"

OverlayWindow::OverlayWindow(HINSTANCE instance)
    : instance_(instance), hwnd_(nullptr), timeText_(L"00:00"), remainingSeconds_(0), totalSeconds_(0) {}

OverlayWindow::~OverlayWindow() {
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
    }
}

void OverlayWindow::RegisterWindowClass() {
    static bool registered = false;
    if (registered) {
        return;
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0x10, 0x10, 0x10));
    wc.lpszClassName = kClassName;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassExW(&wc);
    registered = true;
}

bool OverlayWindow::Create(HWND owner) {
    if (hwnd_ != nullptr) {
        return true;
    }

    RegisterWindowClass();
    const RECT rect = AppConfig::CalculateOverlayRect(AppConfig::kOverlayMinWidth, AppConfig::kOverlayMinHeight);
    hwnd_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        kClassName,
        L"",
        WS_POPUP,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        owner,
        nullptr,
        instance_,
        this);

    if (hwnd_ == nullptr) {
        return false;
    }

    SetLayeredWindowAttributes(hwnd_, 0, AppConfig::kOverlayAlpha, LWA_ALPHA);
    return true;
}

void OverlayWindow::Show() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
        UpdateWindow(hwnd_);
    }
}

void OverlayWindow::Hide() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_HIDE);
    }
}

void OverlayWindow::UpdateTime(const std::wstring& text, int remainingSeconds, int totalSeconds) {
    timeText_ = text;
    remainingSeconds_ = remainingSeconds;
    totalSeconds_ = totalSeconds;

    if (hwnd_ != nullptr) {
        UpdateLayout();
        InvalidateRect(hwnd_, nullptr, TRUE);
    }
}

void OverlayWindow::UpdateLayout() {
    if (hwnd_ == nullptr) {
        return;
    }

    HDC dc = GetDC(hwnd_);
    HFONT overlayFont = Theme::CreateMonoFont(-100, FW_BOLD);
    HGDIOBJ oldFont = SelectObject(dc, overlayFont);

    RECT textRect{0, 0, 0, 0};
    DrawTextW(dc, timeText_.c_str(), -1, &textRect, DT_CALCRECT | DT_SINGLELINE);

    SelectObject(dc, oldFont);
    DeleteObject(overlayFont);
    ReleaseDC(hwnd_, dc);

    int width = (textRect.right - textRect.left) + AppConfig::kOverlayPaddingX * 2;
    int height = (textRect.bottom - textRect.top) + AppConfig::kOverlayPaddingY * 2;

    if (AppConfig::kShowProgressBar) {
        height += AppConfig::kOverlayProgressHeight + 16;
        width = std::max(width, 220);
    }

    width = std::max(width, AppConfig::kOverlayMinWidth);
    height = std::max(height, AppConfig::kOverlayMinHeight);

    const RECT rect = AppConfig::CalculateOverlayRect(width, height);
    SetWindowPos(hwnd_, HWND_TOPMOST, rect.left, rect.top, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

LRESULT CALLBACK OverlayWindow::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    OverlayWindow* self = nullptr;

    if (message == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<OverlayWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self != nullptr) {
        return self->WindowProc(message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT OverlayWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd_, &ps);
        Paint(dc);
        EndPaint(hwnd_, &ps);
        return 0;
    }
    }

    return DefWindowProcW(hwnd_, message, wParam, lParam);
}

void OverlayWindow::Paint(HDC dc) {
    RECT client{};
    GetClientRect(hwnd_, &client);
    // 悬浮窗采用统一深色底并整体 alpha 半透明，视觉上尽量接近原项目效果。
    Theme::FillSolidRect(dc, client, RGB(0x10, 0x10, 0x10));

    RECT timeRect = client;
    if (AppConfig::kShowProgressBar) {
        timeRect.bottom -= 32;
    }

    HFONT overlayFont = Theme::CreateMonoFont(-100, FW_BOLD);
    Theme::DrawCenteredText(dc, timeRect, timeText_, AppConfig::kOverlayTextColor, overlayFont);
    DeleteObject(overlayFont);

    if (!AppConfig::kShowProgressBar || totalSeconds_ <= 0) {
        return;
    }

    // 进度条逻辑保留，默认配置关闭；后续只改配置即可打开。
    const int border = AppConfig::kOverlayBorderWidth;
    const int progressHeight = AppConfig::kOverlayProgressHeight;
    RECT progressRect{
        AppConfig::kOverlaySideMargin,
        client.bottom - progressHeight - 20,
        client.right - AppConfig::kOverlaySideMargin,
        client.bottom - 20
    };

    HPEN borderPen = CreatePen(PS_SOLID, border, AppConfig::kOverlayBorderColor);
    HGDIOBJ oldPen = SelectObject(dc, borderPen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, progressRect.left, progressRect.top, progressRect.right, progressRect.bottom);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(borderPen);

    const double ratio = static_cast<double>(remainingSeconds_) / static_cast<double>(totalSeconds_);
    RECT fillRect = progressRect;
    fillRect.left += border;
    fillRect.top += border;
    fillRect.bottom -= border;
    fillRect.right = fillRect.left + static_cast<int>((progressRect.right - progressRect.left - border * 2) * ratio);
    if (fillRect.right > fillRect.left) {
        Theme::FillSolidRect(dc, fillRect, AppConfig::kOverlayProgressColor);
    }
}
