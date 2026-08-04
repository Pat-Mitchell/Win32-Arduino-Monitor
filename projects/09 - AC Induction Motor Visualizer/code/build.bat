@echo off

set WRAPPERS=../../../wrappers

echo === Rebuilding precompiled header ===
cd ../../..
call build_pch.bat
cd projects/09 - AC Induction Motor Visualizer/code

echo === Building AC Induction Motor Visualizer ===
g++ main.cpp ACMotorApp.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  -o ACInductionMotor.exe TorqueSpeedPanel.cpp RotatingFieldPanel.cpp ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    ACInductionMotor.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE