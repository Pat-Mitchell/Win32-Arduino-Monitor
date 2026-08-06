/// @file main.cpp

#include "DualMotorApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  DualMotorApp app;
  app.Create(hInstance, L"Dual Motor Controller", 640, 520);
  app.Show(nCmdShow);
  return app.Run();
}