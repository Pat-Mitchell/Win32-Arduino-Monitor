@echo off

set WRAPPERS=../../../../wrappers
set UTILS=../../../../Utils

echo === Rebuilding precompiled header ===
cd ../../../..
call build_pch.bat
cd projects/11 - Op-amp Signal Amplifier/code/Strain Gauge

echo === Building Strain Gauge ===
g++ main.cpp StrainPlotPanel.cpp StrainGaugeWindow.cpp^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/serialPortWrappers/SerialPort.cpp ^
  %WRAPPERS%/xinputWrappers/XInputController.cpp ^
  %UTILS%/Utils.cpp ^
  -o StrainGauge.exe ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    StrainGauge.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE