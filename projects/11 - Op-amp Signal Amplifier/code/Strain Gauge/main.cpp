/// @file main.h

#include "../../../../pch.h"
#include "../../../../Utils/Utils.h"
#include "StrainGaugeWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  StrainGaugeWindow win_main;
  win_main.Create(hInstance, L"Strain Gauge", 750, 600);
  win_main.Show(nCmdShow);
  return win_main.Run();
}