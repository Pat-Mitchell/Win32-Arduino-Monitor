/// @file main.cpp

#include "../../../pch.h"
#include "ThreePhaseApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { 
  ThreePhaseApp app;
  app.Create(hInstance, L"3-Phase Power Visualizer", 1000, 420);
  app.Show(nCmdShow);
  return app.Run(); 
}