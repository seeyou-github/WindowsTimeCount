#include "AlertWindow.h"

#include "AppConfig.h"
#include "Theme.h"

AlertWindow::AlertWindow(HINSTANCE instance)
    : instance_(instance), hwnd_(nullptr), button_(nullptr) {}

AlertWindow::~AlertWindow() {
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
    }
}

void AlertWindow::RegisterWindowClass() {
    static bool registered = false;
    if (registered) {
        return;
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(AppConfig::kAlertBackground);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);
    registered = true;
}

void AlertWindow::Show(HWND owner, const std::wstring& title, const std::wstring& message, const std::wstring& buttonText) {
    title_ = title;
    message_ = message;
    buttonText_ = buttonText;

    if (hwnd_ == nullptr) {
        // 提醒窗口只创建一次，后续重复使用，避免每次到时重复注册和销毁窗口类。
        RegisterWindowClass();
        const int width = 800;
        const int height = 500;
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        hwnd_ = CreateWindowExW(
            WS_EX_TOPMOST,
            kClassName,
            title_.c_str(),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            (screenWidth - width) / 2,
            (screenHeight - height) / 2,
            width,
            height,
            owner,
            nullptr,
            instance_,
            this);

        button_ = CreateWindowExW(
            0, L"BUTTON", buttonText_.c_str(),
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
            0, 0, 0, 0,
            hwnd_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(kButtonId)),
            instance_,
            nullptr);
    } else {
        SetWindowTextW(hwnd_, title_.c_str());
        SetWindowTextW(button_, buttonText_.c_str());
    }

    Layout();
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
}

LRESULT CALLBACK AlertWindow::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    AlertWindow* self = nullptr;

    if (message == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<AlertWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<AlertWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self != nullptr) {
        return self->WindowProc(message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT AlertWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        Layout();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kButtonId) {
            ShowWindow(hwnd_, SW_HIDE);
        }
        return 0;
    case WM_DRAWITEM: {
        auto* drawItem = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (drawItem->CtlID == kButtonId) {
            Theme::FillSolidRect(drawItem->hDC, drawItem->rcItem, RGB(0x2c, 0x2c, 0x2c));
            HBRUSH borderBrush = CreateSolidBrush(RGB(0x99, 0x99, 0x99));
            FrameRect(drawItem->hDC, &drawItem->rcItem, borderBrush);
            DeleteObject(borderBrush);

            HFONT font = Theme::CreateUiFont(-22);
            Theme::DrawCenteredText(drawItem->hDC, drawItem->rcItem, buttonText_, RGB(0xff, 0xff, 0xff), font);
            DeleteObject(font);
            return TRUE;
        }
        break;
    }
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

void AlertWindow::Paint(HDC dc) {
    RECT client{};
    GetClientRect(hwnd_, &client);
    // 大红色弹窗用于在非循环模式下提供明显到时提醒。
    Theme::FillSolidRect(dc, client, AppConfig::kAlertBackground);

    RECT iconRect{ 0, 35, client.right, 130 };
    RECT textRect{ 40, 150, client.right - 40, 290 };

    HFONT iconFont = Theme::CreateUiFont(-60, FW_BOLD, L"Segoe UI Emoji");
    Theme::DrawCenteredText(dc, iconRect, L"⏰", RGB(0xff, 0xff, 0xff), iconFont);
    DeleteObject(iconFont);

    HFONT textFont = Theme::CreateUiFont(-50, FW_BOLD);
    Theme::DrawCenteredText(dc, textRect, message_, RGB(0xff, 0xff, 0xff), textFont);
    DeleteObject(textFont);
}

void AlertWindow::Layout() {
    if (hwnd_ == nullptr || button_ == nullptr) {
        return;
    }

    RECT client{};
    GetClientRect(hwnd_, &client);
    MoveWindow(button_, (client.right - 220) / 2, client.bottom - 110, 220, 52, TRUE);
}
