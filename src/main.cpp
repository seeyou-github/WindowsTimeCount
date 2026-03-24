#include <windows.h>

#include "MainWindow.h"
#include "ResourceStrings.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    MainWindow mainWindow(instance);
    if (!mainWindow.Create()) {
        const std::wstring message = LoadStringResource(instance, 1112);
        MessageBoxW(nullptr, message.c_str(), L"WindowsTimeCount", MB_ICONERROR | MB_OK);
        return 1;
    }

    return mainWindow.Run();
}
