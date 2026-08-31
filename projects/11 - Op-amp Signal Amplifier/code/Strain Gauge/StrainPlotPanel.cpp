/// @file StrainPlotPanel.cpp

#include "StrainPlotPanel.h"

namespace {
  const COLORREF kCurveTension = RGB(50, 200, 120);
  const COLORREF kCurveComp = RGB(255, 100, 70);
  const COLORREF kAxis = RGB(100, 100, 100);
  const COLORREF kText = RGB(140, 140, 140);
  const COLORREF kZero = RGB(90, 90, 110);
}

void StrainPlotPanel::Init(int iX, int iY, int iW, int iH) {
  rect_bounds = { iX, iY, iX + iW, iY + iH };
  iPadL = 60; iPadT = 20; iPadR = 16; iPadB = 36;
  Clear();
}

void StrainPlotPanel::Clear() {
  iCount = 0;
  ZeroMemory(arrTime_ms, sizeof(arrTime_ms));
  ZeroMemory(arrStrain, sizeof(arrStrain));
}

void StrainPlotPanel::AddSample(float fTime_ms, float fStrain_ue) {
  if(iCount >= MAX_SAMPLES) {
    memmove(arrTime_ms, arrTime_ms + 1, (MAX_SAMPLES - 1) * sizeof(float));
    memmove(arrStrain, arrStrain + 1, (MAX_SAMPLES - 1) * sizeof(float));
    iCount = MAX_SAMPLES - 1;
  }

  arrTime_ms[iCount] = fTime_ms;
  arrStrain[iCount] = fStrain_ue;
  iCount++;
}

float StrainPlotPanel::GetYMax() const {
  float fMax = 500.0f; // Minimum +/- 500 microstrain scale
  for(int i = 0; i < iCount; i++) {
    float fAbs = (arrStrain[i] < 0) ? -arrStrain[i] : arrStrain[i];
    if(fAbs > fMax) {
      fMax = fAbs * 1.2f;
    }
  }
  return fMax;
}

int StrainPlotPanel::MapX(int iIdx) const {
  int iW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
  if(iCount <= 1) {
    return rect_bounds.left + iPadL;
  }
  return rect_bounds.left + iPadL + (int)((float)iIdx / (iCount - 1) * iW);
}

int StrainPlotPanel::MapY(float fVal, float fYmin, float fYmax) const {
  int iH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
  float fRange = fYmax - fYmin;
  if(fRange < 0.001f) {
    fRange = 0.001f;
  }
  return rect_bounds.top + iPadT + iH - (int)(((fVal - fYmin) / fRange) * iH);
}

void StrainPlotPanel::Draw(HDC hdc) const {
  int iL = rect_bounds.left;
  int iT = rect_bounds.top;
  int iR = rect_bounds.right;
  int iB = rect_bounds.bottom;
  int iPL = iL + iPadL;
  int iPT = iT + iPadT;
  int iPR = iR - iPadR;
  int iPB = iB - iPadB;

  // Background
  HBRUSH hbr = CreateSolidBrush(RGB(20, 20, 20));
  RECT rf = rect_bounds;
  FillRect(hdc, &rf, hbr);
  DeleteObject(hbr);

  float fYmax = GetYMax();
  float fYmin = -fYmax;

  // Axes 
  HPEN hpen_ax = CreatePen(PS_SOLID, 1, kAxis);
  HPEN hpen_old = (HPEN)SelectObject(hdc, hpen_ax);
  MoveToEx(hdc, iPL, iPT, NULL);
  LineTo(hdc, iPL, iPB);
  MoveToEx(hdc, iPL, iPB, NULL);
  LineTo(hdc, iPR, iPB);
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_ax);

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, kText);

  // Y axis labels (µε)
  wchar_t arrBuf[24];
  wsprintf(arrBuf, L"+%dµε", (int)fYmax);
  TextOut(hdc, iL + 2, MapY(fYmax, fYmin, fYmax) - 6, arrBuf, lstrlen(arrBuf));
  wsprintf(arrBuf, L"-%dµε", (int)fYmax);
  TextOut(hdc, iL + 2, MapY(-fYmax, fYmin, fYmax) - 6, arrBuf, lstrlen(arrBuf));
  TextOut(hdc, iL + 2, MapY(0.0f, fYmin, fYmax) - 6, L"0", 1);

  // X axis time span
  if(iCount > 1) {
    int iSpan_s = (int)((arrTime_ms[iCount - 1] - arrTime_ms[0]) / 1000.0f);
    wsprintf(arrBuf, L"%ds", iSpan_s);
    TextOut(hdc, iPR - 10, iPB + 6, arrBuf, lstrlen(arrBuf));
  }
  TextOut(hdc, iPL - 2, iPB + 6, L"0", 1);

  // Zero line 
  int iY0 = MapY(0.0f, fYmin, fYmax);
  HPEN hpen_zero = CreatePen(PS_DOT, 1, kZero);
  SelectObject(hdc, hpen_zero);
  MoveToEx(hdc, iPL, iY0, NULL);
  LineTo(hdc, iPR, iY0);
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_zero);

  SetTextColor(hdc, kZero);
  TextOut(hdc, iPL + 4, iY0 - 14, L"TENSION ->", 9);
  TextOut(hdc, iPL + 4, iY0 + 4, L"COMPRESSION ->", 13);

  if(iCount < 2) {
    return;
  }

  // Waveform. Changes color above/below zero
  for(int i = 1; i < iCount; i++) {
    // Choose color based on whether this segment is tension or compression
    bool bTension = (arrStrain[i] >= 0.0f);
    HPEN hpen_seg = CreatePen(PS_SOLID, 2, bTension ? kCurveTension : kCurveComp);
    SelectObject(hdc, hpen_seg);
    MoveToEx(hdc, MapX(i - 1), MapY(arrStrain[i - 1], fYmin, fYmax), NULL);
    LineTo(hdc, MapX(i), MapY(arrStrain[i], fYmin, fYmax));
    SelectObject(hdc, hpen_old);
    DeleteObject(hpen_seg);
  }
}