/// @file main.cpp


#include "LRCResonance.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  INITCOMMONCONTROLSEX icc = {};
  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&icc);

  LRCWindow win_main;
  win_main.Create(hInstance, L"LRC Resonance Detector", 640, 580);
  win_main.Show(nCmdShow);
  return win_main.Run();
}