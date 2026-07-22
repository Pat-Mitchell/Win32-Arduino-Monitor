/// @file main.cpp
/// @brief Entry point for the Controller Input Visualizer

#include "ControllerVisualizerApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  ControllerVisualizerApp app;
  app.Create(hInstance, L"Controller Input Visualizer", 800, 500);
  app.Show(nCmdShow);
  return app.Run();
}