#pragma once

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

namespace AppConfig {

struct TimeButtonConfig {
    int stringId;
    int seconds;
};

enum class OverlayPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    TopCenter,
    BottomCenter
};

inline constexpr COLORREF kMainBackground = RGB(0x25, 0x25, 0x25);
inline constexpr COLORREF kButtonBackground = RGB(0x33, 0x33, 0x33);
inline constexpr COLORREF kButtonHoverBackground = RGB(0x40, 0x40, 0x40);
inline constexpr COLORREF kButtonPressedBackground = RGB(0x73, 0x35, 0x35);
inline constexpr COLORREF kResetButtonBackground = RGB(0x55, 0x55, 0x55);
inline constexpr COLORREF kTextColor = RGB(0xbb, 0xbb, 0xbb);
inline constexpr COLORREF kAccentColor = RGB(0x4c, 0xaf, 0x50);
inline constexpr COLORREF kBorderColor = RGB(0x44, 0x44, 0x44);
inline constexpr COLORREF kAlertBackground = RGB(0xb7, 0x1c, 0x1c);
inline constexpr COLORREF kOverlayTextColor = RGB(0xff, 0xff, 0xff);
inline constexpr COLORREF kOverlayProgressColor = RGB(0x3f, 0xa1, 0x56);
inline constexpr COLORREF kOverlayBorderColor = RGB(0xff, 0xff, 0xff);
inline constexpr BYTE kOverlayAlpha = 102;

inline constexpr int kMainWidth = 750;
inline constexpr int kMainHeight = 550;
inline constexpr int kOverlayMinWidth = 180;
inline constexpr int kOverlayMinHeight = 72;
inline constexpr int kOverlayPaddingX = 26;
inline constexpr int kOverlayPaddingY = 14;
inline constexpr int kOverlayMarginX = 8;
inline constexpr int kOverlayMarginY = 8;
inline constexpr int kOverlayProgressHeight = 20;
inline constexpr int kOverlayBorderWidth = 2;
inline constexpr int kOverlaySideMargin = 16;
inline constexpr bool kShowProgressBar = false;
inline constexpr OverlayPosition kOverlayPosition = OverlayPosition::BottomRight;

inline constexpr int kStartButtonWidth = 220;
inline constexpr int kControlButtonWidth = 100;
inline constexpr int kControlButtonHeight = 42;
inline constexpr int kTimeButtonWidth = 180;
inline constexpr int kTimeButtonHeight = 60;
inline constexpr int kButtonsPerRow = 3;

inline const std::vector<TimeButtonConfig> kTimeButtons = {
    {1200, 1},
    {1201, 5},
    {1202, 10},
    {1203, 60},
    {1204, 300},
    {1205, 900},
};

std::wstring FormatDuration(int totalSeconds);
APPBARDATA QueryTaskbar();
RECT CalculateOverlayRect(int width, int height);

}  // namespace AppConfig
