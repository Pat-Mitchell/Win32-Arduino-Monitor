@echo off

set WRAPPERS=../../../wrappers
set UTILS=../../../Utils

echo === Rebuilding precompiled header ===
cd ../../..
call build_pch.bat
cd projects/10 - Dual Motor Controller/code

echo === Building Dual Motor Controller ===
g++ main.cpp DualMotorApp.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/serialPortWrappers/SerialPort.cpp ^
  %WRAPPERS%/xinputWrappers/XInputController.cpp ^
  %UTILS%/Utils.cpp ^
  -o DualMotorApp.exe MotorPanel.cpp ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    DualMotorApp.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE