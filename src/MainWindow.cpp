#include "MainWindow.h"

#include <commctrl.h>
#include <cstring>

#include "AppConfig.h"
#include "resource.h"
#include "ResourceStrings.h"
#include "Theme.h"

namespace {

constexpr int kResetButtonIdValue = 1001;
constexpr int kLoopCheckBoxIdValue = 1002;
constexpr int kStartButtonIdValue = 1004;

COLORREF ResolveButtonColor(int controlId, bool isSelected) {
    if (controlId == kResetButtonIdValue) {
        return isSelected ? AppConfig::kButtonPressedBackground : AppConfig::kResetButtonBackground;
    }
    if (controlId == kStartButtonIdValue) {
        return isSelected ? AppConfig::kButtonPressedBackground : AppConfig::kAccentColor;
    }
    if (controlId == kLoopCheckBoxIdValue) {
        return RGB(0x2f, 0x2f, 0x2f);
    }
    return isSelected ? AppConfig::kButtonPressedBackground : AppConfig::kButtonBackground;
}

}  // namespace

MainWindow::MainWindow(HINSTANCE instance)
    : instance_(instance),
      hwnd_(nullptr),
      totalLabel_(nullptr),
      totalValueLabel_(nullptr),
      groupBox_(nullptr),
      resetButton_(nullptr),
      loopCheckBox_(nullptr),
      stopButton_(nullptr),
      startButton_(nullptr),
      uiFont_(nullptr),
      titleFont_(nullptr),
      totalFont_(nullptr),
      largeIcon_(nullptr),
      smallIcon_(nullptr),
      trayIconAdded_(false),
      totalSeconds_(0),
      remainingSeconds_(0),
      isRunning_(false),
      isPaused_(false),
      loopMode_(false),
      overlay_(instance),
      alert_(instance) {}

MainWindow::~MainWindow() {
    RemoveTrayIcon();
    if (largeIcon_ != nullptr) {
        DestroyIcon(largeIcon_);
    }
    if (smallIcon_ != nullptr) {
        DestroyIcon(smallIcon_);
    }
    DestroyFonts();
}

bool MainWindow::Create() {
    InitCommonControls();
    RegisterWindowClass();
    CreateFonts();

    hwnd_ = CreateWindowExW(
        0,
        kClassName,
        LoadText(1100).c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        AppConfig::kMainWidth,
        AppConfig::kMainHeight,
        nullptr,
        nullptr,
        instance_,
        this);

    if (hwnd_ != nullptr) {
        ApplyWindowIcons();
        ApplyDarkTitleBar();
    }

    return hwnd_ != nullptr;
}

int MainWindow::Run() {
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void MainWindow::RegisterWindowClass() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(AppConfig::kMainBackground);
    wc.hIcon = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
    wc.hIconSm = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    wc.lpszClassName = kClassName;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassExW(&wc);
}

void MainWindow::CreateFonts() {
    uiFont_ = Theme::CreateUiFont(-22);
    titleFont_ = Theme::CreateUiFont(-20);
    totalFont_ = Theme::CreateMonoFont(-60, FW_BOLD);
}

void MainWindow::DestroyFonts() {
    if (uiFont_ != nullptr) {
        DeleteObject(uiFont_);
    }
    if (titleFont_ != nullptr) {
        DeleteObject(titleFont_);
    }
    if (totalFont_ != nullptr) {
        DeleteObject(totalFont_);
    }
}

LRESULT CALLBACK MainWindow::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* self = nullptr;

    if (message == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<MainWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self != nullptr) {
        return self->WindowProc(message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT MainWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls();
        LayoutControls();
        RefreshTotalLabel();
        return 0;
    case WM_CLOSE:
        if (isRunning_) {
            MinimizeToTray();
            return 0;
        }
        ExitApplication();
        return 0;
    case WM_SIZE:
        LayoutControls();
        return 0;
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, AppConfig::kTextColor);
        SetBkColor(dc, AppConfig::kMainBackground);
        static HBRUSH backgroundBrush = CreateSolidBrush(AppConfig::kMainBackground);
        return reinterpret_cast<INT_PTR>(backgroundBrush);
    }
    case WM_COMMAND: {
        const int controlId = LOWORD(wParam);
        if (controlId == kResetButtonId) {
            ResetTime();
        } else if (controlId == kLoopCheckBoxId) {
            loopMode_ = !loopMode_;
            InvalidateRect(loopCheckBox_, nullptr, TRUE);
        } else if (controlId == kStopButtonId) {
            StopTimer();
        } else if (controlId == kStartButtonId) {
            ToggleTimer();
        } else if (controlId >= kTimeButtonBaseId && controlId < kTimeButtonBaseId + 100) {
            for (const auto& button : timeButtons_) {
                if (GetDlgCtrlID(button.hwnd) == controlId) {
                    AddTime(button.seconds);
                    break;
                }
            }
        }
        return 0;
    }
    case WM_TIMER:
        if (wParam == kTimerId) {
            RunCountdownTick();
        }
        return 0;
    case WM_DRAWITEM:
        DrawOwnerButton(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
        return TRUE;
    case kTrayMessage:
        if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            RestoreFromTray();
        } else if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            ShowTrayMenu();
        }
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd_, kTimerId);
        RemoveTrayIcon();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd_, message, wParam, lParam);
}

void MainWindow::CreateControls() {
    // 顶部区域负责展示当前累积出的总倒计时时长。
    totalLabel_ = CreateWindowExW(
        0, L"STATIC", LoadText(1101).c_str(),
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        hwnd_, nullptr, instance_, nullptr);

    totalValueLabel_ = CreateWindowExW(
        0, L"STATIC", L"00:00",
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        hwnd_, nullptr, instance_, nullptr);

    groupBox_ = CreateWindowExW(
        0, L"BUTTON", LoadText(1102).c_str(),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0, 0, 0, 0,
        hwnd_, nullptr, instance_, nullptr);

    resetButton_ = CreateWindowExW(
        0, L"BUTTON", LoadText(1103).c_str(),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kResetButtonId)), instance_, nullptr);

    loopCheckBox_ = CreateWindowExW(
        0, L"BUTTON", LoadText(1104).c_str(),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLoopCheckBoxId)), instance_, nullptr);

    stopButton_ = CreateWindowExW(
        0, L"BUTTON", LoadText(1105).c_str(),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStopButtonId)), instance_, nullptr);

    startButton_ = CreateWindowExW(
        0, L"BUTTON", LoadText(1106).c_str(),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStartButtonId)), instance_, nullptr);

    SendMessageW(totalLabel_, WM_SETFONT, reinterpret_cast<WPARAM>(titleFont_), TRUE);
    SendMessageW(totalValueLabel_, WM_SETFONT, reinterpret_cast<WPARAM>(totalFont_), TRUE);
    SendMessageW(groupBox_, WM_SETFONT, reinterpret_cast<WPARAM>(titleFont_), TRUE);
    SendMessageW(resetButton_, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), TRUE);
    SendMessageW(loopCheckBox_, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), TRUE);
    SendMessageW(stopButton_, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), TRUE);
    SendMessageW(startButton_, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), TRUE);

    // 预设时长按钮完全来自配置表，后续要扩展按钮只改配置即可。
    timeButtons_.clear();
    for (size_t i = 0; i < AppConfig::kTimeButtons.size(); ++i) {
        const auto& config = AppConfig::kTimeButtons[i];
        HWND button = CreateWindowExW(
            0, L"BUTTON", LoadText(config.stringId).c_str(),
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
            0, 0, 0, 0,
            hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTimeButtonBaseId + static_cast<int>(i))),
            instance_, nullptr);
        SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), TRUE);
        timeButtons_.push_back({button, config.seconds});
    }
}

void MainWindow::LayoutControls() {
    if (hwnd_ == nullptr) {
        return;
    }

    MoveWindow(totalLabel_, 30, 32, 180, 30, TRUE);
    MoveWindow(totalValueLabel_, 220, 10, 320, 86, TRUE);
    MoveWindow(groupBox_, 20, 110, 690, 245, TRUE);

    const int baseX = 42;
    const int baseY = 152;
    const int gapX = 18;
    const int gapY = 16;
    for (size_t index = 0; index < timeButtons_.size(); ++index) {
        const int row = static_cast<int>(index) / AppConfig::kButtonsPerRow;
        const int col = static_cast<int>(index) % AppConfig::kButtonsPerRow;
        MoveWindow(
            timeButtons_[index].hwnd,
            baseX + col * (AppConfig::kTimeButtonWidth + gapX),
            baseY + row * (AppConfig::kTimeButtonHeight + gapY),
            AppConfig::kTimeButtonWidth,
            AppConfig::kTimeButtonHeight,
            TRUE);
    }

    const int bottomY = 400;
    MoveWindow(resetButton_, 40, bottomY, 110, AppConfig::kControlButtonHeight, TRUE);
    MoveWindow(loopCheckBox_, 170, bottomY, 150, AppConfig::kControlButtonHeight, TRUE);
    MoveWindow(stopButton_, 340, bottomY, AppConfig::kControlButtonWidth, AppConfig::kControlButtonHeight, TRUE);
    MoveWindow(startButton_, 460, bottomY, AppConfig::kStartButtonWidth, AppConfig::kControlButtonHeight, TRUE);
}

void MainWindow::RefreshTotalLabel() {
    // 该标签始终显示“总时长”，而不是剩余时间，这与原 Python 逻辑保持一致。
    const std::wstring totalText = AppConfig::FormatDuration(totalSeconds_);
    SetWindowTextW(totalValueLabel_, totalText.c_str());
}

void MainWindow::UpdateOverlayDisplay() {
    // 悬浮窗只负责剩余时间显示，因此单独走 remainingSeconds_。
    const std::wstring text = AppConfig::FormatDuration(remainingSeconds_);
    if (overlay_.Create(hwnd_)) {
        overlay_.UpdateTime(text, remainingSeconds_, totalSeconds_);
    }
}

void MainWindow::AddTime(int seconds) {
    // 运行中追加时长时，需要同时扩展总时长与剩余时长。
    if (isRunning_) {
        totalSeconds_ += seconds;
        remainingSeconds_ += seconds;
    } else {
        totalSeconds_ += seconds;
        remainingSeconds_ = totalSeconds_;
    }

    RefreshTotalLabel();
    UpdateOverlayDisplay();
}

void MainWindow::ResetTime() {
    StopTimer();
    totalSeconds_ = 0;
    remainingSeconds_ = 0;
    RefreshTotalLabel();
    UpdateOverlayDisplay();
}

void MainWindow::StopTimer() {
    // 停止行为与 Python 版本一致：恢复剩余时间到总时长，并隐藏悬浮窗。
    isRunning_ = false;
    isPaused_ = false;
    KillTimer(hwnd_, kTimerId);
    remainingSeconds_ = totalSeconds_;
    UpdateStartButtonText();
    overlay_.Hide();
    UpdateOverlayDisplay();
}

void MainWindow::ToggleTimer() {
    if (totalSeconds_ <= 0) {
        return;
    }

    if (!isRunning_) {
        // 首次启动时立即刷新一次显示，然后进入 1 秒周期定时器。
        isRunning_ = true;
        isPaused_ = false;
        UpdateStartButtonText();
        overlay_.Create(hwnd_);
        overlay_.Show();
        RunCountdownTick();
        return;
    }

    if (!isPaused_) {
        // 暂停只停止节拍，不销毁悬浮窗，让用户保留当前剩余时间视觉反馈。
        isPaused_ = true;
        KillTimer(hwnd_, kTimerId);
        UpdateStartButtonText();
        return;
    }

    isPaused_ = false;
    UpdateStartButtonText();
    SetTimer(hwnd_, kTimerId, 1000, nullptr);
}

void MainWindow::RunCountdownTick() {
    if (remainingSeconds_ > 0) {
        // 先显示当前秒，再减 1，保持和 tkinter after 方案一致的观感。
        UpdateOverlayDisplay();
        --remainingSeconds_;
        SetTimer(hwnd_, kTimerId, 1000, nullptr);
        return;
    }

    // 归零时先显示 00:00，再发声并根据循环模式决定后续行为。
    UpdateOverlayDisplay();
    MessageBeep(MB_OK);
    if (loopMode_) {
        remainingSeconds_ = totalSeconds_;
        SetTimer(hwnd_, kTimerId, 1000, nullptr);
    } else {
        overlay_.Hide();
        ShowCompletionAlert();
    }
}

void MainWindow::ShowCompletionAlert() {
    StopTimer();
    alert_.Show(hwnd_, LoadText(1107), LoadText(1108), LoadText(1109));
}

void MainWindow::UpdateStartButtonText() {
    int textId = 1106;
    if (isRunning_ && !isPaused_) {
        textId = 1110;
    } else if (isPaused_) {
        textId = 1111;
    }

    SetWindowTextW(startButton_, LoadText(textId).c_str());
    InvalidateRect(startButton_, nullptr, TRUE);
}

void MainWindow::DrawOwnerButton(const DRAWITEMSTRUCT* drawItem) {
    if (drawItem == nullptr) {
        return;
    }

    wchar_t caption[128]{};
    GetWindowTextW(drawItem->hwndItem, caption, static_cast<int>(std::size(caption)));

    const int controlId = static_cast<int>(drawItem->CtlID);
    const bool isSelected = (drawItem->itemState & ODS_SELECTED) != 0;
    const bool isLoopBox = controlId == kLoopCheckBoxId;

    Theme::FillSolidRect(drawItem->hDC, drawItem->rcItem, ResolveButtonColor(controlId, isSelected));

    HBRUSH borderBrush = CreateSolidBrush(AppConfig::kBorderColor);
    FrameRect(drawItem->hDC, &drawItem->rcItem, borderBrush);
    DeleteObject(borderBrush);

    if (isLoopBox) {
        // 循环模式使用自绘复选框，避免系统默认浅色主题破坏整体暗色界面。
        RECT checkRect{
            drawItem->rcItem.left + 12,
            drawItem->rcItem.top + 10,
            drawItem->rcItem.left + 34,
            drawItem->rcItem.top + 32
        };
        Theme::FillSolidRect(drawItem->hDC, checkRect, RGB(0x1a, 0x1a, 0x1a));
        HBRUSH checkBorder = CreateSolidBrush(RGB(0x88, 0x88, 0x88));
        FrameRect(drawItem->hDC, &checkRect, checkBorder);
        DeleteObject(checkBorder);

        if (loopMode_) {
            HPEN pen = CreatePen(PS_SOLID, 3, AppConfig::kAccentColor);
            HGDIOBJ oldPen = SelectObject(drawItem->hDC, pen);
            MoveToEx(drawItem->hDC, checkRect.left + 4, checkRect.top + 11, nullptr);
            LineTo(drawItem->hDC, checkRect.left + 9, checkRect.top + 16);
            LineTo(drawItem->hDC, checkRect.right - 4, checkRect.top + 5);
            SelectObject(drawItem->hDC, oldPen);
            DeleteObject(pen);
        }

        RECT textRect = drawItem->rcItem;
        textRect.left += 42;
        HFONT font = Theme::CreateUiFont(-20);
        Theme::DrawCenteredText(drawItem->hDC, textRect, caption, AppConfig::kTextColor, font);
        DeleteObject(font);
    } else {
        HFONT font = Theme::CreateUiFont(-20);
        Theme::DrawCenteredText(drawItem->hDC, drawItem->rcItem, caption, RGB(0xee, 0xee, 0xee), font);
        DeleteObject(font);
    }
}

std::wstring MainWindow::LoadText(int stringId) const {
    return LoadStringResource(instance_, stringId);
}

HWND MainWindow::FindTimeButton(int controlId) const {
    for (const auto& button : timeButtons_) {
        if (GetDlgCtrlID(button.hwnd) == controlId) {
            return button.hwnd;
        }
    }
    return nullptr;
}

void MainWindow::ApplyWindowIcons() {
    largeIcon_ = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
    smallIcon_ = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));

    if (largeIcon_ != nullptr) {
        SendMessageW(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(largeIcon_));
    }
    if (smallIcon_ != nullptr) {
        SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon_));
    }
}

void MainWindow::ApplyDarkTitleBar() {
    const BOOL enabled = TRUE;
    DwmSetWindowAttribute(hwnd_, 20, &enabled, sizeof(enabled));
    DwmSetWindowAttribute(hwnd_, 19, &enabled, sizeof(enabled));
    const COLORREF captionColor = RGB(0x10, 0x10, 0x10);
    const COLORREF textColor = RGB(0xf0, 0xf0, 0xf0);
    DwmSetWindowAttribute(hwnd_, 35, &captionColor, sizeof(captionColor));
    DwmSetWindowAttribute(hwnd_, 36, &textColor, sizeof(textColor));
}

void MainWindow::AddTrayIcon() {
    if (trayIconAdded_) {
        return;
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = kTrayIconId;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = kTrayMessage;
    nid.hIcon = smallIcon_ != nullptr ? smallIcon_ : largeIcon_;
    std::wstring tip = LoadText(IDS_APP_TITLE);
    wcsncpy_s(nid.szTip, tip.c_str(), _TRUNCATE);

    trayIconAdded_ = Shell_NotifyIconW(NIM_ADD, &nid) == TRUE;
}

void MainWindow::RemoveTrayIcon() {
    if (!trayIconAdded_) {
        return;
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = kTrayIconId;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    trayIconAdded_ = false;
}

void MainWindow::MinimizeToTray() {
    AddTrayIcon();
    ShowWindow(hwnd_, SW_HIDE);
}

void MainWindow::RestoreFromTray() {
    RemoveTrayIcon();
    ShowWindow(hwnd_, SW_SHOW);
    ShowWindow(hwnd_, SW_RESTORE);
    SetForegroundWindow(hwnd_);
}

void MainWindow::ShowTrayMenu() {
    HMENU menu = CreatePopupMenu();
    if (menu == nullptr) {
        return;
    }

    const std::wstring showText = LoadText(IDS_TRAY_SHOW);
    const std::wstring exitText = LoadText(IDS_TRAY_EXIT);
    AppendMenuW(menu, MF_STRING, kTrayMenuShowId, showText.c_str());
    AppendMenuW(menu, MF_STRING, kTrayMenuExitId, exitText.c_str());

    POINT cursor{};
    GetCursorPos(&cursor);
    SetForegroundWindow(hwnd_);
    const UINT command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY,
        cursor.x, cursor.y, 0, hwnd_, nullptr);
    DestroyMenu(menu);

    if (command == kTrayMenuShowId) {
        RestoreFromTray();
    } else if (command == kTrayMenuExitId) {
        ExitApplication();
    }
}

void MainWindow::ExitApplication() {
    RemoveTrayIcon();
    DestroyWindow(hwnd_);
}
