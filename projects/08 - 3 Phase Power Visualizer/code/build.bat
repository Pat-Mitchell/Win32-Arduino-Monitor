@echo off

set WRAPPERS=../../../wrappers

echo === Rebuilding precompiled header ===
cd ../../..
call build_pch.bat
cd projects/08 - 3 Phase Power Visualizer/code

echo === Building Phase Power Visualizer ===
g++ main.cpp ThreePhaseApp.cpp WaveformPanel.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  -o ThreePhasePower.exe ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    ThreePhasePower.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE