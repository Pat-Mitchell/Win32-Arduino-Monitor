/// @file phasorPanel.cpp
#include "PhasorPanel.h"
#include <cwchar> // wcslen
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


namespace {
  const COLORREF CLR_BG = RGB(12, 14, 26); // Panel Background
  const COLORREF CLR_AXIS = RGB(35, 38, 65); // Vertical axis
  const COLORREF CLR_AXISX = RGB(50, 55, 90); // Horizontal axis
  const COLORREF CLR_DIM = RGB(55, 58, 90); // Panel title/Error text
  const COLORREF CLR_I = RGB(160, 160, 175); // Current reference
  const COLORREF CLR_VR = RGB(72, 199, 142); // V_R = green
  const COLORREF CLR_VL = RGB(99, 172, 255); // V_L = blue
  const COLORREF CLR_VC = RGB(255, 152, 67); // V_C = amber
  const COLORREF CLR_VTOT = RGB(240, 240, 255); // V_TOTAL = near white
  const COLORREF CLR_PHI = RGB(230, 210, 80); // Phase arc and label
}

void PhasorPanel::Draw(HDC hdc, const RECT& rc, const PhasorState& s) {
  // Background
  HBRUSH hBg = CreateSolidBrush(CLR_BG);
  FillRect(hdc, &rc, hBg);
  DeleteObject(hBg);

  int iW = rc.right - rc.left;
  int iH = rc.bottom - rc.top;
  int ox = rc.left + MARGIN; 
  int oy = rc.top + iH / 2;

  DrawAxes(hdc, rc, ox, oy);
  DrawLabel(hdc, rc.left + 8, rc.top + 6, L"Phasor Diagram", CLR_DIM);

  // Early exits
  if(!s.bValid) {
    DrawLabel(hdc, rc.left + iW / 2 - 40, oy - 8, L"Invalid circuit", CLR_DIM);
    return;
  }
  if(s.fVTotal < 1e-6f) {
    DrawLabel(hdc, rc.left + iW / 2 - 20, oy - 8, L"V = 0", CLR_DIM);
    return;
  }

  // Scale
  // Horizontal constraint: V_Total must fit within the available width.
  // Vertical constraint: max(V_L, V_C) must fit within the half-height.
  // Use whichever is more restrictive so nothing clips.
  int iAvailRight = (rc.right - MARGIN) - ox;
  int iAvailVert = iH / 2 - MARGIN;
  float fMaxVert = max(s.fVL, s.fVC);

  float fScaleH = (float)iAvailRight / s.fVTotal;
  float fScaleV = (fMaxVert > 1e-6f) ? (float)iAvailVert / fMaxVert : fScaleH;
  float fScale = min(fScaleH, fScaleV);

  // Screen endpoints
  // Screen convention +x = right, +y = down
  // Phasor at angle theta : screen_x = ox + V * cos(theta) * scale
  //                         screen_y = oy - V * sin(theta) * scale (Y flipped)
  int iVRx = ox + (int)(s.fVR * fScale); // V_R: angle 0
  int iVRy = oy;

  int iVLx = ox; // V_L: angle +90 deg
  int iVLy = oy - (int)(s.fVL * fScale);

  int iVCx = ox; // V_C: angle -90 deg
  int iVCy = oy + (int)(s.fVC * fScale);

  int iVTx = ox + (int)(s.fVTotal * cosf(s.fPhi_rad) * fScale); // V_Total
  int iVTy = oy - (int)(s.fVTotal * sinf(s.fPhi_rad) * fScale);

  // I is shown as a fixed-length direction indicator only.
  // Curent and voltage have different units so I is not drawn to the 
  // same scale. It only marks the reference axis.
  const int iILen = 70;
  int iIx = ox + iILen;
  int iIy = oy;

  // Phase arc
  // Only draw if angle is large enough to be visible
  if(fabsf(s.fPhi_rad) > 0.02f)
    DrawPhaseArc(hdc, ox, oy, s.fPhi_rad, CLR_PHI);

  // Phasors
  // Draw order: I, then V_R/V_L/V_C, then V_Total on top. (Painter's algorithm)
  // Skip any phasor whose screen length is less than 2px (Basically invisible)
  DrawArrow(hdc, ox, oy, iIx, iIy, CLR_I, true);
  DrawLabel(hdc, iIx + 4, iIy + 5, L"I", CLR_I);

  if((int)(s.fVR * fScale) > 2) {
    DrawArrow(hdc, ox, oy, iVRx, iVRy, CLR_VR);
    DrawLabel(hdc, iVRx + 5, iVRy - 18, L"VR", CLR_VR);
  }

  if((int)(s.fVL * fScale) > 2) {
    DrawArrow(hdc, ox, oy, iVLx, iVLy, CLR_VL);
    DrawLabel(hdc, iVLx + 5, iVLy - 2, L"VL", CLR_VL);
  }

  if((int)(s.fVC * fScale) > 2) {
    DrawArrow(hdc, ox, oy, iVCx, iVCy, CLR_VC);
    DrawLabel(hdc, iVCx + 5, iVCy - 2, L"VC", CLR_VC);
  }

  // V_Total. Drawn last so it sits on top of everything
  DrawArrow(hdc, ox, oy, iVTx, iVTy, CLR_VTOT);
  int iVTLblY = iVTy + (s.fPhi_rad >= 0.0f ? -18 : 5);
  DrawLabel(hdc, iVTx + 5, iVTLblY, L"V", CLR_VTOT);

  // Phase angle label
  if(fabsf(s.fPhi_rad) > 0.02f) {
    float fMid = s.fPhi_rad * 0.5f;
    const int iArcR = 62;
    DrawLabel(hdc, 
      ox + (int)(iArcR * cosf(fMid)) - 6, 
      oy - (int)(iArcR * sinf(fMid)) - 8,
      L"φ", CLR_PHI);
  }
}

void PhasorPanel::DrawAxes(HDC hdc, const RECT& rc, int ox, int oy) {
  // Vertical axis
  HPEN hPen = CreatePen(PS_SOLID, 1, CLR_AXIS);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, ox, rc.top + 4, NULL);
  LineTo(hdc, ox, rc.bottom - 4);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);

  // Horizontal axis
  hPen = CreatePen(PS_SOLID, 1, CLR_AXISX);
  hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, rc.left + 4, oy, NULL);
  LineTo(hdc, rc.right - 4, oy);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
}

void PhasorPanel::DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, bool bDashed) {
  const int iArrowLen = 11;
  const float fAngle = 0.42f; // -24 degrees

  // Shaft
  // PS_DASH only works at pen width 1
  HPEN hPen = CreatePen(bDashed ? PS_DASH : PS_SOLID, bDashed ? 1 : 2, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, x0, y0, NULL);
  LineTo(hdc, x1, y1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);

  // Arrowhead
  float dx = (float)(x1 - x0);
  float dy = (float)(y1 - y0);
  float fLen = sqrtf(dx * dx + dy * dy);
  if(fLen < 1.0f) return;
  dx /= fLen;
  dy /= fLen;

  // Rotate the direction vector by ±fAngle to get the two arrowhead lines.
  // Rotate(dx, dy, a) = (dx*cos(a) - dy*sin(a), dx*sin(a) + dy*cos(a))
  float ax1 = x1 - iArrowLen * (dx * cosf( fAngle) - dy * sinf( fAngle));
  float ay1 = y1 - iArrowLen * (dy * cosf( fAngle) + dx * sinf( fAngle));
  float ax2 = x1 - iArrowLen * (dx * cosf(-fAngle) - dy * sinf(-fAngle));
  float ay2 = y1 - iArrowLen * (dy * cosf(-fAngle) + dx * sinf(-fAngle));

  HPEN hHead = CreatePen(PS_SOLID, 2, clr);
  hOld = (HPEN)SelectObject(hdc, hHead);
  MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, (int)ax1, (int)ay1);
  MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, (int)ax2, (int)ay2);
  SelectObject(hdc, hOld);
  DeleteObject(hHead);
}

void PhasorPanel::DrawPhaseArc(HDC hdc, int cx, int cy, float fPhi_rad, COLORREF clr) {
  // Polyline arc — 40 segments between angle 0 and fPhi_rad.
  // screen_x = cx + r*cos(t),  screen_y = cy - r*sin(t)  (Y axis flipped)
  const int iRadius = 46;
  const int iSteps  = 40;

  POINT pts[41];
  for (int i = 0; i <= iSteps; i++) {
    float t  = fPhi_rad * i / (float)iSteps;
    pts[i].x = cx + (int)(iRadius * cosf(t));
    pts[i].y = cy - (int)(iRadius * sinf(t));
  }

  HPEN hPen = CreatePen(PS_SOLID, 1, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  Polyline(hdc, pts, iSteps + 1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
}

void PhasorPanel::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}