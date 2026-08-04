/// @file main.cpp

#include "..\..\..\pch.h"
#include "ACMotorApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  ACMotorApp app;
  app.Create(hInstance, L"AC induction Motor Visualizer", 1000, 680);
  app.Show(nCmdShow);
  return app.Run();
}