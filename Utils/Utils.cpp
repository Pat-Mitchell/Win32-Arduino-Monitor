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

void FormatReadout(wchar_t* arrBuf, int iBufLen, const wchar_t* szLabel, float fValue, int iDecimalPlaces, const wchar_t* szUnit) {
  // Clamp decimal places to a sensible range
  if(iDecimalPlaces < 0) iDecimalPlaces = 0;
  if(iDecimalPlaces > 6) iDecimalPlaces = 6;

  // Compute the power of 10 multiplier for the fractional part
  int iMultiplier = 1;
  for (int i = 0; i < iDecimalPlaces; i++) iMultiplier *= 10;

  // Handle negative values
  BOOL bNeg = (fValue < 0.0f);
  if(bNeg) fValue = -fValue;

  int iInteger = (int)fValue;
  int iFrac = (int)((fValue - (float)iInteger) * iMultiplier + 0.5f);

  // Handle carry
  if(iFrac >= iMultiplier) {
    iFrac -= iMultiplier;
    iInteger++;
  }

  if(iDecimalPlaces > 0) {
    // Build format string for fraction part with leading zeros
    wchar_t arrFmtFrac[8];
    wsprintf(arrFmtFrac, L"%%0%dd", iDecimalPlaces);

    wchar_t arrFrac[16];
    wsprintf(arrFrac, arrFmtFrac, iFrac);

    wsprintf(arrBuf, L"%s: %s%d.%s %s", szLabel, bNeg ? L"-" : L"", iInteger, arrFrac, szUnit);
  } else {
    wsprintf(arrBuf, L"%s: %s%d %s", szLabel, bNeg ? L"-" : L"", iInteger, szUnit);
  }

  // Ensure null termination within buffer bounds
  arrBuf[iBufLen - 1] = L'\0';
}

float MapFloat(float fVal, float fInMin, float fInMax, float fOutMin, float fOutMax) {
  // Guard against divide by zero if the input range is degenerate
  if(fInMax == fInMin) return fOutMin;

  return (fVal - fInMin) / (fInMax - fInMin) * (fOutMax - fOutMin) + fOutMin;
}

float ClampFloat(float fVal, float fMin, float fMax) {
  if(fVal < fMin) return fMin;
  if(fVal > fMax) return fMax;
  return fVal;
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