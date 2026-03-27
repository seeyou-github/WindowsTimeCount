#pragma once

#include <dwmapi.h>
#include <shellapi.h>
#include <windows.h>
#include <vector>

#include "AlertWindow.h"
#include "OverlayWindow.h"

class MainWindow {
public:
    explicit MainWindow(HINSTANCE instance);
    ~MainWindow();

    bool Create();
    int Run();

private:
    enum ControlId {
        kResetButtonId = 1001,
        kLoopCheckBoxId = 1002,
        kStopButtonId = 1003,
        kStartButtonId = 1004,
        kTimeButtonBaseId = 2000
    };

    struct TimeButton {
        HWND hwnd;
        int seconds;
    };

    static constexpr const wchar_t* kClassName = L"WindowsTimeCountMainWindow";
    static constexpr UINT_PTR kTimerId = 1;
    static constexpr UINT kTrayMessage = WM_APP + 1;
    static constexpr UINT kTrayIconId = 1;
    static constexpr UINT kTrayMenuShowId = 5001;
    static constexpr UINT kTrayMenuExitId = 5002;

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    void RegisterWindowClass();
    void CreateFonts();
    void DestroyFonts();
    void CreateControls();
    void LayoutControls();
    void RefreshTotalLabel();
    void UpdateOverlayDisplay();
    void AddTime(int seconds);
    void ResetTime();
    void StopTimer();
    void ToggleTimer();
    void RunCountdownTick();
    void ShowCompletionAlert();
    void UpdateStartButtonText();
    void DrawOwnerButton(const DRAWITEMSTRUCT* drawItem);
    std::wstring LoadText(int stringId) const;
    HWND FindTimeButton(int controlId) const;
    void ApplyWindowIcons();
    void ApplyDarkTitleBar();
    void AddTrayIcon();
    void RemoveTrayIcon();
    void MinimizeToTray();
    void RestoreFromTray();
    void ShowTrayMenu();
    void ExitApplication();

    HINSTANCE instance_;
    HWND hwnd_;
    HWND totalLabel_;
    HWND totalValueLabel_;
    HWND groupBox_;
    HWND resetButton_;
    HWND loopCheckBox_;
    HWND stopButton_;
    HWND startButton_;
    std::vector<TimeButton> timeButtons_;

    HFONT uiFont_;
    HFONT titleFont_;
    HFONT totalFont_;
    HBRUSH backgroundBrush_;
    HICON largeIcon_;
    HICON smallIcon_;
    bool trayIconAdded_;

    int totalSeconds_;
    int remainingSeconds_;
    bool isRunning_;
    bool isPaused_;
    bool loopMode_;

    OverlayWindow overlay_;
    AlertWindow alert_;
};
