@echo off

echo === Rebuilding precompiled header ===
cd ..\..\
call build_pch.bat
cd Lessons\5ControllerVisualizer

echo === Building Controller Visualizer App ===
g++ -mwindows -DUNICODE -D_UNICODE ^
-o ControllerVisualizer.exe ^
main.cpp ^
ControllerVisualizerApp.cpp ^
StickPanel.cpp ^
TriggerPanel.cpp ^
..\..\wrappers\xinputWrappers\XInputController.cpp ^
..\..\wrappers\win32Wrappers\Window.cpp ^
..\..\wrappers\serialPortWrappers\SerialPort.cpp ^
-lxinput -lcomctl32

if %errorlevel% == 0 (
  echo Build Successful.
  echo Running...
  echo.
  ControllerVisualizer.exe
) else (
  echo Build failed!
)

echo.
PAUSE