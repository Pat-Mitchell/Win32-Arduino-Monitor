@echo off

set WRAPPERS=../../../wrappers
set UTILS=../../../Utils

echo === Rebuilding precompiled header ===
cd ../../..
call build_pch.bat
cd projects/07 - H-Bridge DC Motor Controller/code

echo === Building H-Bridge Motor ===
g++ HBridgeMotor.cpp ^
  %WRAPPERS%/win32Wrappers/Window.cpp ^
  %WRAPPERS%/serialPortWrappers/SerialPort.cpp ^
  %UTILS%/Utils.cpp ^
  -o HBridgeMotor.exe ^
  -mwindows -DUNICODE -D_UNICODE -lcomctl32
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