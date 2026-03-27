#include "AppConfig.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace AppConfig {

std::wstring FormatDuration(int totalSeconds) {
    const int safeSeconds = std::max(totalSeconds, 0);
    const int hours = safeSeconds / 3600;
    const int remainder = safeSeconds % 3600;
    const int minutes = remainder / 60;
    const int seconds = remainder % 60;

    std::wstringstream builder;
    builder << std::setfill(L'0');

    if (safeSeconds < 3600) {
        builder << std::setw(2) << (hours * 60 + minutes)
                << L":" << std::setw(2) << seconds;
    } else {
        builder << std::setw(2) << hours
                << L":" << std::setw(2) << minutes
                << L":" << std::setw(2) << seconds;
    }

    return builder.str();
}

APPBARDATA QueryTaskbar() {
    APPBARDATA appBar{};
    appBar.cbSize = sizeof(appBar);
    SHAppBarMessage(ABM_GETTASKBARPOS, &appBar);
    return appBar;
}

RECT CalculateOverlayRect(int width, int height) {
    RECT workArea{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);

    APPBARDATA taskbar = QueryTaskbar();
    const int workWidth = workArea.right - workArea.left;

    int x = workArea.left;
    int y = workArea.top;

    if (taskbar.hWnd != nullptr) {
        switch (taskbar.uEdge) {
        case ABE_BOTTOM:
            x = workArea.right - width - kOverlayMarginX;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        case ABE_TOP:
            x = workArea.right - width - kOverlayMarginX;
            y = workArea.top + kOverlayMarginY;
            break;
        case ABE_LEFT:
            x = workArea.left + kOverlayMarginX;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        case ABE_RIGHT:
            x = workArea.right - width - kOverlayMarginX;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        default:
            break;
        }
    } else {
        switch (kOverlayPosition) {
        case OverlayPosition::TopLeft:
            x = workArea.left + kOverlayMarginX;
            y = workArea.top + kOverlayMarginY;
            break;
        case OverlayPosition::TopRight:
            x = workArea.right - width - kOverlayMarginX;
            y = workArea.top + kOverlayMarginY;
            break;
        case OverlayPosition::BottomLeft:
            x = workArea.left + kOverlayMarginX;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        case OverlayPosition::BottomRight:
            x = workArea.right - width - kOverlayMarginX;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        case OverlayPosition::TopCenter:
            x = workArea.left + (workWidth - width) / 2;
            y = workArea.top + kOverlayMarginY;
            break;
        case OverlayPosition::BottomCenter:
            x = workArea.left + (workWidth - width) / 2;
            y = workArea.bottom - height - kOverlayMarginY;
            break;
        }
    }

    return RECT{ x, y, x + width, y + height };
}

}  // namespace AppConfig
