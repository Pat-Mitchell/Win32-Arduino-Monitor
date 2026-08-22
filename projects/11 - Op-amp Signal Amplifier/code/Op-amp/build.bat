@echo off

set WRAPPERS=../../../../wrappers
set UTILS=../../../../Utils

echo === Rebuilding precompiled header ===
cd ../../../..
call build_pch.bat
cd projects/11 - Op-amp Signal Amplifier/code/Op-amp

echo === Building Oscilloscope ===
g++ Oscilloscope.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/serialPortWrappers/SerialPort.cpp ^
  %WRAPPERS%/xinputWrappers/XInputController.cpp ^
  %UTILS%/Utils.cpp ^
  -o Oscilloscope.exe ^
  -mwindows -DUNICODE -D_UNICODE -lxinput -lcomctl32
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    Oscilloscope.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE