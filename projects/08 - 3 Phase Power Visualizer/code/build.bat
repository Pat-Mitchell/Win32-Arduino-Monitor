@echo off

set WRAPPERS=../../../wrappers

echo === Rebuilding precompiled header ===
cd ../../..
call build_pch.bat
cd projects/08 - 3 Phase Power Visualizer/code

echo === Building H-Bridge Motor ===
g++ main.cpp ThreePhaseApp.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/serialPortWrappers/SerialPort.cpp ^
  -o ThreePhasePower.exe ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    HBridgeMotor.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE