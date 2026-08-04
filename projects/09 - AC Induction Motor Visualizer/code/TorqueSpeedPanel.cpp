/// @file TorqueSpeedPanel.cpp
#include "TorqueSpeedPanel.h"

namespace {
  const COLORREF CLR_CURVE = RGB(99, 172, 255);  // torque curve
  const COLORREF CLR_DIM = RGB(55, 58, 90);
  const COLORREF CLR_NS = RGB(80, 85, 120);  // Ns dashed marker
  const COLORREF CLR_START = RGB(255, 152, 67);  // starting torque
  const COLORREF CLR_BREAK = RGB(255, 80, 80);  // breakdown torque
  const COLORREF CLR_FL = RGB(72, 199, 142);  // full load point
}

int TorqueSpeedPanel::MapX(float fVal, float fMin, float fMax, const RECT& rcPlot) {
    int iPlotW = rcPlot.right - rcPlot.left;
    return rcPlot.left + (int)((fVal - fMin) / (fMax - fMin) * iPlotW);
}

int TorqueSpeedPanel::MapY(float fVal, float fMin, float fMax, const RECT& rcPlot) {
    int iPlotH = rcPlot.bottom - rcPlot.top;
    return rcPlot.bottom - (int)((fVal - fMin) / (fMax - fMin) * iPlotH);
}

void TorqueSpeedPanel::Draw(HDC hdc, const RECT& rc, const MotorState& s) {
  PlotPanel panel(rc, L"Torque-Speed Curve", L"Rotor Speed (RPM)", L"Torque (norm.)");
  panel.SetCurveColor(CLR_CURVE);

  // if invalid state, draw empty axes with no curve
  if(!s.bValid) {
    panel.Draw(hdc, nullptr, nullptr, 0, 0.0f, 1.0f, 0.0f, Y_MAX);
    return;
  }

  panel.Draw(hdc, s.arrNr_rpm, s.arrTorque, CURVE_STEPS + 1, 0.0f, s.fNs_rpm, 0.0f, Y_MAX);

  RECT rcPlot = {
    rc.left + PlotPanelLayout::MARGIN_LEFT,
    rc.top + PlotPanelLayout::MARGIN_TOP,
    rc.right - PlotPanelLayout::MARGIN_RIGHT,
    rc.bottom - PlotPanelLayout::MARGIN_BOTTOM
  };

  DrawNsMarker(hdc, rcPlot);
  DrawKeyPoints(hdc, rcPlot, s);
}

void TorqueSpeedPanel::DrawNsMarker(HDC hdc, const RECT& rcPlot) {
  HPEN hPen = CreatePen(PS_DASH, 1, CLR_NS);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, rcPlot.right, rcPlot.top + 2, NULL);
  LineTo(hdc, rcPlot.right, rcPlot.bottom);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
}

void TorqueSpeedPanel::DrawKeyPoints(HDC hdc, const RECT& rcPlot, const MotorState& s) {
  const int iDotR= 5;
  wchar_t buf[48];

  // Starting torque
  int iSx = MapX(0.0f, 0.0f, s.fNs_rpm, rcPlot);
  int iSy = MapY(s.fT_start, 0.0f, Y_MAX, rcPlot);
  DrawDot(hdc, iSx, iSy, iDotR, CLR_START);
  swprintf(buf, 48, L"T_start = %.2f", s.fT_start);
  DrawLabel(hdc, iSx + 8, iSy - 18, buf, CLR_START);

  // Breakdown torque
  int iBx = MapX(s.fNr_breakdown, 0.0f, s.fNs_rpm, rcPlot);
  int iBy = MapY(s.fT_breakdown, 0.0f, Y_MAX, rcPlot);
  DrawDot(hdc, iBx, iBy, iDotR, CLR_BREAK);
  swprintf(buf, 48, L"Breakdown (%.0f RPM)", s.fNr_breakdown);
  DrawLabel(hdc, iBx - 60, iBy - 18, buf, CLR_BREAK);

  // Full-load point
  int iFx = MapX(s.fNr_fullload, 0.0f, s.fNs_rpm, rcPlot);
  int iFy = MapY(s.fT_fullload, 0.0f, Y_MAX, rcPlot);
  DrawDot(hdc, iFx, iFy, iDotR, CLR_FL);
  swprintf(buf, 48, L"Full load (%.0f RPM)", s.fNr_fullload);
  DrawLabel(hdc, iFx + 8, iFy - 18, buf, CLR_FL);
}

void TorqueSpeedPanel::DrawDot(HDC hdc, int x, int y, int iR, COLORREF clr) {
  HBRUSH hBr = CreateSolidBrush(clr);
  HPEN hPen = CreatePen(PS_SOLID, 1, clr);
  HBRUSH hOldBr = (HBRUSH)SelectObject(hdc, hBr);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
  Ellipse(hdc, x - iR, y - iR, x + iR, y + iR);
  SelectObject(hdc, hOldBr);
  SelectObject(hdc, hOldPen);
  DeleteObject(hBr);
  DeleteObject(hPen);
}

void TorqueSpeedPanel::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}