/// @file main.cpp

#include "XAudio2Synth.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // D2D factory must exist before any D2DWindow is constructed
  if(FAILED(D2DWindow::InitD2D())) {
    MessageBox(NULL, L"Direct2D initialisation failed.", L"Fatal", MB_OK | MB_ICONERROR);
    return -1;
  }

  XAudio2Window win;
  win.Create(hInstance, L"XAudio2 Wave Generator", 660, 480);
  win.Show(nCmdShow);
  return win.Run();
}