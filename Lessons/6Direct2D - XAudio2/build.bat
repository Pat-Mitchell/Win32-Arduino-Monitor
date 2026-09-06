@echo off

set WRAPPERS=../../wrappers
set UTILS=../../Utils

echo === Rebuilding precompiled header ===
cd ../../
call build_pch.bat
cd Lessons/6Direct2D - XAudio2

echo === Building XAudio2 Wave Generator ===
g++ main.cpp XAudio2Synth.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/win32Wrappers/D2DWindow.cpp ^
  %UTILS%/Utils.cpp ^
  -o XAudio2Synth.exe ^
  -mwindows -DUNICODE -D_UNICODE ^
  -lole32 -ld2d1 -ldwrite -lxaudio2_9
  if %errorlevel% == 0 (
    echo Build Successful. 
    echo Running...
    echo.
    XAudio2Synth.exe
  ) else (
    echo Build failed!
  )

  echo.
  PAUSE