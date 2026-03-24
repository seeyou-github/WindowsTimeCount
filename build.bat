@echo off
setlocal

echo ===== Build WindowsTimeCount =====

if not exist build mkdir build

set CXX=g++
set RC=windres
set CXXFLAGS=-std=c++17 -DUNICODE -D_UNICODE -Ires -Isrc
set LDFLAGS=-municode -mwindows -lgdi32 -lcomctl32 -lshell32 -lole32 -loleaut32 -luuid -ldwmapi -luxtheme -lwinmm

%CXX% -c src\main.cpp %CXXFLAGS% -o build\main.o
if errorlevel 1 goto error

%CXX% -c src\AppConfig.cpp %CXXFLAGS% -o build\AppConfig.o
if errorlevel 1 goto error

%CXX% -c src\ResourceStrings.cpp %CXXFLAGS% -o build\ResourceStrings.o
if errorlevel 1 goto error

%CXX% -c src\Theme.cpp %CXXFLAGS% -o build\Theme.o
if errorlevel 1 goto error

%CXX% -c src\OverlayWindow.cpp %CXXFLAGS% -o build\OverlayWindow.o
if errorlevel 1 goto error

%CXX% -c src\AlertWindow.cpp %CXXFLAGS% -o build\AlertWindow.o
if errorlevel 1 goto error

%CXX% -c src\MainWindow.cpp %CXXFLAGS% -o build\MainWindow.o
if errorlevel 1 goto error

%RC% res\resource.rc -O coff -o build\resource.o
if errorlevel 1 goto error

echo Linking...
%CXX% build\main.o build\AppConfig.o build\ResourceStrings.o build\Theme.o build\OverlayWindow.o build\AlertWindow.o build\MainWindow.o build\resource.o -o build\WindowsTimeCount.exe %LDFLAGS%
if errorlevel 1 goto error

echo ===== Build succeeded =====
echo Output: build\WindowsTimeCount.exe
goto end

:error
echo ===== Build failed =====
exit /b 1

:end
endlocal
