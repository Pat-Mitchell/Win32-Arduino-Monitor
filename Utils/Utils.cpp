/// @file Utiles.cpp
/// @brief Implementation of project-wide utility functions.

#include "Utils.h"
#include <math.h> // atan2f, cosf, sinf, sqrtf

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

// Private helper
/// @brief Draws a filled arrowhead triangle at (fTipX, fTipY) pointing
///   in the direction fAngleRad. Called by both DrawArrow and DrawArcArrow
static void DrawArrowHead(HDC hdc, float fTipX, float fTipY, float fAngleRad, int iHeadSize, COLORREF clr) {
  // The two barb points are offset +/- 30 degrees from the stem direction
  // meaasured from the tip back along the stem
  // The base angle points away from the tip (stem direction + 180 degrees)
  const float fBarb = 30.0f * DEG_TO_RAD;
  float fBaseAng = fAngleRad + M_PI; // Direction back along stem

  POINT pts[3];
  pts[0].x = (int)fTipX;
  pts[0].y = (int)fTipY;
  pts[1].x = (int)(fTipX + cosf(fBaseAng - fBarb) * iHeadSize);
  pts[1].y = (int)(fTipY + sinf(fBaseAng - fBarb) * iHeadSize);
  pts[2].x = (int)(fTipX + cosf(fBaseAng + fBarb) * iHeadSize);
  pts[2].y = (int)(fTipY + sinf(fBaseAng + fBarb) * iHeadSize);

  HBRUSH hBrush = CreateSolidBrush(clr);
  HPEN hPen = CreatePen(PS_SOLID, 1, clr);
  HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

  Polygon(hdc, pts, 3);

  SelectObject(hdc, hOldBrush);
  SelectObject(hdc, hOldPen);
  DeleteObject(hBrush);
  DeleteObject(hPen);
}

// public draw arrows
void DrawArrow(HDC hdc, int iX1, int iY1, int iX2, int iY2, int iHeadSize, COLORREF clr, int iWeight = 1) {
  // Draw the stem line
  HPEN hPen = CreatePen(PS_SOLID, iWeight, clr);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

  MoveToEx(hdc, iX1, iY1, NULL);
  LineTo(hdc, iX2, iY2);

  SelectObject(hdc, hOldPen);
  DeleteObject(hPen);

  // Compute the angle the stem points towards the tip
  float fAngle = atan2f((float)(iY2 - iY1), (float)(iX2 - iX1));

  DrawArrowHead(hdc, (float)iX2, (float)iY2, fAngle, iHeadSize, clr);
}

void DrawArcArrow(HDC hdc, int iCX, int iCY, int iRadius, float fStartAngle, float fSweepAngle, int iHeadSize, COLORREF clr) {
  // Win32 ARC() uses a bounding rect and takes start/end points on the ellipse
  // rather than angles. Convert angles to bounding-rect points.
  // Angles follow GDI convention: 0 = right, clockwise positive (Y axis down)
  float fStartRad = fStartAngle * DEG_TO_RAD;
  float fEndAngle = fStartAngle + fSweepAngle;
  float fEndRad = fEndAngle * DEG_TO_RAD;

  // Bounding rect of the circle
  int iLeft = iCX - iRadius;
  int iTop = iCY - iRadius;
  int iRight = iCX + iRadius;
  int iBottom = iCY + iRadius;

  // Start and end points on the circle circumference
  int iStartX = iCX + (int)(cosf(fStartRad) * iRadius);
  int iStartY = iCY + (int)(sinf(fStartRad) * iRadius);
  int iEndX = iCX + (int)(cosf(fEndRad) * iRadius);
  int iEndY = iCY + (int)(sinf(fEndRad) * iRadius);

  HPEN hPen = CreatePen(PS_SOLID, 1, clr);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

  // Arc() draws counter-clockwise by default in GDI (Y-down coordinate space)
  // For clockwise sweep, use AngleArc or swap start/end and use Arc.
  // Swapping start/end with Arc() give clockwise arc.
  if(fSweepAngle >= 0.0f) {
    // Clockwise: swap start/end for GDI Arc
    Arc(hdc, iLeft, iTop, iRight, iBottom, iStartX, iStartY, iEndX, iEndY);
  } else {
    // Counter-clockwise: Natural GDI direction
    Arc(hdc, iLeft, iTop, iRight, iBottom, iEndX, iEndY, iStartX, iStartY);
  }

  SelectObject(hdc, hOldPen);
  DeleteObject(hPen);

  // Arrowhead at the end of the arc
  // The tangent direction at the endpoint is perpendicular to the radius,
  // rotated 90 degrees in the sweep direction.
  float fTangentAngle;
  if(fSweepAngle >= 0.0f) {
    fTangentAngle = fEndRad + M_PI / 2.0f; // Clockwise Tangent
  } else {
    fTangentAngle = fEndRad - M_PI / 2.0f; // Counter-clockwise tangent
  }

  DrawArrowHead(hdc, (float)iEndX, (float)iEndY, fTangentAngle, iHeadSize, clr);
}

void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}