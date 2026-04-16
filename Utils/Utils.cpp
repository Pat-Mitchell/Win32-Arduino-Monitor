/// @file Utiles.cpp
/// @brief Implementation of project-wide utility functions.

#include "Utils.h"

void ScanComPorts(ComboBox* cmb_port) {
  cmb_port->Clear();

  for(int i = 1; i <= 20; i++) {
    wchar_t arrPortPath[16];
    wsprintf(arrPortPath, L"\\\\.\\COM%d", i);

    HANDLE hTest = CreateFile(arrPortPath,
                              GENERIC_READ | GENERIC_WRITE,
                              0, NULL, OPEN_EXISTING, 0, NULL);

    if(hTest != INVALID_HANDLE_VALUE) {
      wchar_t arrName[8];
      wsprintf(arrName, L"COM%d", i);
      cmb_port->AddItem(arrName);
      CloseHandle(hTest);
    } else if(GetLastError() == ERROR_ACCESS_DENIED) {
      wchar_t arrName[16];
      wsprintf(arrName, L"COM%d (in use)", i);
      cmb_port->AddItem(arrName);
    }
  }
  cmb_port->SelectFirst();
}

float ParseFloat(const wchar_t* szSrc, const wchar_t* szKey) {
  const wchar_t* pFound = wcsstr(szSrc, szKey);
  if(!pFound) return -1.0f;

  pFound += wcslen(szKey);
  return wcstof(pFound, nullptr);
}

BOOL ShowSaveDialog(HWND hwnd_owner, const wchar_t* szFilter, const wchar_t* szDefExt, wchar_t* arrPathOut, int iBufLen) {
  OPENFILENAME ofn = {};
  arrPathOut[0] = L'\0';

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd_owner;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrDefExt = szDefExt;
  ofn.lpstrFile = arrPathOut;
  ofn.nMaxFile = iBufLen;
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

  return GetSaveFileName(&ofn);
}