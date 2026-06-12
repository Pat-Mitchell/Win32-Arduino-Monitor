#pragma once

#include "FreqPlotPanel.h"
#include <cwchar>

namespace {
  const COLORREF kCurve = RGB(80, 180, 255);
  const COLORREF kPeak = RGB(255, 160, 40);
  const COLORREF kRefLine = RGB(60, 140, 60);
  const COLORREF kAxis = RGB(100, 100, 100);
  const COLORREF kText = RGB(140, 140, 140);
}

void FreqPlotPanel::Init(int iX, int iY, int iW, int iH) {
  rect_bounds = {iX, iY, iX + iW, iY + iH};
  iPadL = 60;
  iPadT = 20;
  iPadR = 20;
  iPadB = 44;
  Clear();
}

void FreqPlotPanel::Clear() {
  iCount = iPeakIdx = 0;
  ZeroMemory(arrFreq, sizeof(arrFreq));
  ZeroMemory(arrV, sizeof(arrV));
}

void FreqPlotPanel::AddSample(float fFreq, float fV) {
  if(iCount >= MAX_SAMPLES) return;
  arrFreq[iCount] = fFreq;
  arrV[iCount] = fV;
  if(iCount == 0 || fV > arrV[iPeakIdx]) iPeakIdx = iCount;
  iCount++;
}

float FreqPlotPanel::GetPeakFreq() const {
  return (iCount > 0) ? arrFreq[iPeakIdx] : -1.0f;
}

float FreqPlotPanel::GetPeakV() const {
  return (iCount > 0) ? arrV[iPeakIdx] : -1.0f;
}

/// @brief Maps sample index to pixel X.
///   Since the Arduino sweeps logarithmically, equal index spacing
///   approximates a log-frequency zxis without explicit log math.
int FreqPlotPanel::MapXByIdx(int i) const {
  int iW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
  if(iCount <= 1) {
    return rect_bounds.left + iPadL;
  }
  return rect_bounds.left + iPadL + (int)((float) i / (iCount - 1) * iW);
}

int FreqPlotPanel::MapY(float fV, float fVmax) const {
  int iH = (rect_bounds.bottom - rect_bounds.top) -iPadT - iPadB;
  if(fVmax < 0.001f) fVmax = 5.0f;
  return rect_bounds.top + iPadT + iH - (int)((fV / fVmax) * iH);
}

/// @brief Renders the frequency response plot
/// @param hdc Device context
/// @param fVpin Pin voltage. Sets Y scale and draws theoretical peak line.
void FreqPlotPanel::Draw(HDC hdc, float fVpin) const {
  int iL = rect_bounds.left;
  int iT = rect_bounds.top;
  int iR = rect_bounds.right;
  int iB = rect_bounds.bottom;

  // Background
  HBRUSH hbr = CreateSolidBrush(RGB(20, 20, 20));
  RECT rf = rect_bounds;
  FillRect(hdc, &rf, hbr);
  DeleteObject(hbr);

  float fVmax = (fVpin > 0.1f) ? fVpin * 1.1f : 5.0f;

  // Axes
  HPEN hpen_ax = CreatePen(PS_SOLID, 1, kAxis);
  SelectObject(hdc, hpen_ax);
  MoveToEx(hdc, iL + iPadL, iT + iPadT, NULL);
  LineTo(hdc, iL + iPadL, iB - iPadB);
  MoveToEx(hdc, iL + iPadL, iB - iPadB, NULL);
  LineTo(hdc, iR - iPadR, iB - iPadB);
  DeleteObject(hpen_ax);

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, kText);

  // Y axis labels
  wchar_t arrBuf[32];
  TextOut(hdc, iL + 4, MapY(fVmax, fVmax) - 6, L"V_pin", 5);
  TextOut(hdc, iL + 4, MapY(0.0f, fVmax) - 6, L"0V", 2);

  // Theorectical V_pin reference line (dashed green)
  if(fVpin > 0.1f) {
    HPEN hpen_ref = CreatePen(PS_DASH, 1, kRefLine);
    SelectObject(hdc, hpen_ref);
    int iYref = MapY(fVpin, fVmax);
    MoveToEx(hdc, iL + iPadL, iYref, NULL);
    LineTo(hdc, iR - iPadR, iYref);
    DeleteObject(hpen_ref);
    SetTextColor(hdc, kRefLine);
    TextOut(hdc, iL + iPadL + 4, iYref - 14, L"V_R at resonance", 16);
  }

  // X axis labels. Start and end frequency in kHz
  if(iCount > 0) {
    SetTextColor(hdc, kText);

    swprintf(arrBuf, L"%.0fk", arrFreq[0] / 1000.0f);
    TextOut(hdc, iL + iPadL, iB - iPadB + 8, arrBuf, lstrlen(arrBuf));
    swprintf(arrBuf, L"%.0fk", arrFreq[iCount - 1] / 1000.0f);
    TextOut(hdc, iR - iPadR - 28, iB - iPadB + 8, arrBuf, lstrlen(arrBuf));

    // Hz unit label centered under X axis
    TextOut(hdc, iL + iPadL + (iR - iPadR - iL - iPadL) / 2 - 12, iB - iPadB + 26, L"kHz", 3);
  }

  // Resonant peak marker. Dashed vertical amber line
  if(iCount > 1) {
    HPEN hpen_peak = CreatePen(PS_DASH, 1, kPeak);
    SelectObject(hdc, hpen_peak);
    int iXpeak = MapXByIdx(iPeakIdx);
    MoveToEx(hdc, iXpeak, iT + iPadT, NULL);
    LineTo(hdc, iXpeak, iB - iPadB);
    DeleteObject(hpen_peak);

    SetTextColor(hdc, kPeak);
    swprintf(arrBuf, L"%.1fk", arrFreq[iPeakIdx] / 1000.0f);
    TextOut(hdc, iXpeak + 3, iT + iPadT + 4, arrBuf, lstrlen(arrBuf));
  }

  // Data curve
  if(iCount < 2) return;

  HPEN hpen_c = CreatePen(PS_SOLID, 2, kCurve);
  SelectObject(hdc, hpen_c);
  MoveToEx(hdc, MapXByIdx(0), MapY(arrV[0], fVmax), NULL);
  for(int i = 1; i < iCount; i++) {
    LineTo(hdc, MapXByIdx(i), MapY(arrV[i], fVmax));
  }
  DeleteObject(hpen_c);
}