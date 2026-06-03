/// @file main.cpp
#include "ACPhasor.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  ACPhasorApp app;
  app.Create(hInstance, L"AC Phasor Diagram", 960, 680);
  app.Show(nCmdShow);
  return app.Run();
}