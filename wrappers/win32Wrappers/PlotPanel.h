/// @file PlotPanel.h
/// @brief Reusable GDI panel for rendering a 2D line plot.
///   Draws a dark background plot area, axis labels, an optional
///   horizontal line marker, and a polyline though caller-supplied
///   data points.
///
/// Usage:
///   Plot panel(rcPlot, L"RC Charge Curve", L"Time (ms)", L"Voltage (V)");
///   panel.SetMarker(0.632f);
///   panel.Draw(hdc, arrX, arrY, iCount, 0.0f, fMaxTime, 0.0f, 5.0f);

#pragma once
#include <windows.h>

// Color Palette
namespace PlotPanelColors {
  inline constexpr COLORREF CLR_BACKGROUND = RGB(20, 20, 20);
  inline constexpr COLORREF CLR_BORDER = RGB(80, 80, 80);
  inline constexpr COLORREF CLR_AXIS = RGB(100, 100, 100);
  inline constexpr COLORREF CLR_LABEL = RGB(180, 180, 180);
  inline constexpr COLORREF CLR_TITLE = RGB(220, 220, 220);
  inline constexpr COLORREF CLR_CURVE_DEF = RGB(50, 200, 100);
  inline constexpr COLORREF CLR_MARKER_DEF = RGB(200, 140, 30);
}

// PlotPanelLayout constants
namespace PlotPanelLayout {
  inline constexpr int MARGIN_TOP = 24;
  inline constexpr int MARGIN_BOTTOM = 28;
  inline constexpr int MARGIN_LEFT = 12;
  inline constexpr int MARGIN_RIGHT = 8;
}

class PlotPanel {
  public:
    /// @brief Constructs a PlotPanel occuying the given rect
    /// @param rcPanel Bounding rect in parent client coordinates
    /// @param szTitle Title drawn inside the top of the plot area
    /// @param szLabelX X axis label drawn below the plot
    /// @param szLabelY Y axis label drawn left of the plot
    PlotPanel(RECT rcPanel, const wchar_t* szTitle, const wchar_t* szLabelX, const wchar_t* szLabelY)
      : rc_panel(rcPanel)
      , fMarkerNorm(0.0f)
      , bMarkerVisible(FALSE)
      , clr_curve(PlotPanelColors::CLR_CURVE_DEF)
      , clr_marker(PlotPanelColors::CLR_MARKER_DEF) 
      {
        wcsncpy(arr_title, szTitle, 63);
        arr_title[63] = L'\0';
        wcsncpy(arr_label_x, szLabelX, 31);
        arr_label_x[31] = L'\0';
        wcsncpy(arr_label_y, szLabelY, 31);
        arr_label_y[31] = L'\0';
      }

    /// @brief Enables a horizontal threshold marker line
    ///   The marker is drawn at the Y value cooresponding to
    ///   fNormalised Y * (fMax - fMin) + fYmin in data space,
    ///   so pass a normalised [0,1] fraction of the Y range.
    ///   e.g. 0.632f draws the RC 63.2% tau marker
    ///   Call with bVisible = FALSE to hide it
    /// @param fNormalisedY Fraction of the Y axis range [0,1]
    /// @param bVisible 
    void SetMarker(float fNormalisedY, BOOL bVisible = TRUE) {
      fMarkerNorm = fNormalisedY;
      bMarkerVisible = bVisible;
    }

    /// @brief Overrides the default curve color (green)
    /// @param clr clr New curve color as a COLORREF 
    void SetCurveColor(COLORREF clr) {
      clr_curve = clr;
    }

    /// @brief Overrides the default marker colot (amber)
    /// @param clr New marker color as a COLORREF
    void SetMarkerColor(COLORREF clr) {
      clr_marker = clr;
    }

    /// @brief Renders the full panel into hdc
    ///   If iCount is 0 the panel draws empty axes with no curve
    /// @param hdc Deveice context to draw into
    /// @param arrX Caller-owned array of X values in data space
    /// @param arrY Caller owned array of Y values in data space
    /// @param iCount Number of valid points in arrX / arrY
    /// @param fXMin Minimum X axis value 
    /// @param fXMax Maximum X axis value
    /// @param fYMin Minimum Y axis value
    /// @param fYMax Maximum Y axis value
    void Draw(HDC hdc, const float* arrX, const float* arrY, int iCount, float fXMin, float fXMax, float fYMin, float fYMax) const {
      RECT rcPlot = {
        rc_panel.left + PlotPanelLayout::MARGIN_LEFT,
        rc_panel.top + PlotPanelLayout::MARGIN_TOP,
        rc_panel.right - PlotPanelLayout::MARGIN_RIGHT,
        rc_panel.bottom - PlotPanelLayout::MARGIN_BOTTOM
      };

      // Panel background and border
      HBRUSH hBrushBg = CreateSolidBrush(PlotPanelColors::CLR_BACKGROUND);
      HPEN hPenBorder = CreatePen(PS_SOLID, 1, PlotPanelColors::CLR_BORDER);

      HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushBg);
      HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);

      Rectangle(hdc, rc_panel.left, rc_panel.top, rc_panel.right, rc_panel.bottom);

      SelectObject(hdc, hOldBrush);
      SelectObject(hdc, hOldPen);
      DeleteObject(hBrushBg);
      DeleteObject(hPenBorder);

      // Title
      SetBkMode(hdc, TRANSPARENT);
      SetTextColor(hdc, PlotPanelColors::CLR_TITLE);

      RECT rcTitle = {
        rc_panel.left,
        rc_panel.top + 4,
        rc_panel.right,
        rc_panel.top + PlotPanelLayout::MARGIN_TOP
      };
      DrawText(hdc, arr_title, -1, &rcTitle, DT_CENTER | DT_SINGLELINE);

      // X axis label
      SetTextColor(hdc, PlotPanelColors::CLR_LABEL);

      RECT rcLabelX = {
        rcPlot.left,
        rc_panel.bottom - PlotPanelLayout::MARGIN_BOTTOM + 6,
        rcPlot.right,
        rc_panel.bottom -2
      };
      DrawText(hdc, arr_label_x, -1, &rcLabelX, DT_CENTER | DT_SINGLELINE);

      // Y axis label
      int iSavedDC = SaveDC(hdc);

      SetGraphicsMode(hdc, GM_ADVANCED);
      XFORM xfRotate = {};
      xfRotate.eM11 = 0.0f; // cos(-90)
      xfRotate.eM12 = 1.0f; // sin(-90)
      xfRotate.eM21 = -1.0f; // -sin(-90)
      xfRotate.eM22 = 0.0f; // cos(-90)
      // Translate so the rotated text centers vertically along the left margin
      xfRotate.eDx = (float)(rc_panel.left + 10);
      xfRotate.eDy = (float)((rcPlot.top + rcPlot.bottom) / 2);
      SetWorldTransform(hdc, &xfRotate);

      SetTextColor(hdc, PlotPanelColors::CLR_LABEL);
      SetBkMode(hdc, TRANSPARENT);

      // In rotated space, draw centered at origin. The transform places it correctly
      RECT rcLabelY = {-60, -10, 60, 10 };
      DrawText(hdc, arr_label_y, -1, &rcLabelY, DT_CENTER | DT_SINGLELINE);

      RestoreDC(hdc, iSavedDC);

      // Axis Lines
      HPEN hPenAxis = CreatePen(PS_SOLID, 1, PlotPanelColors::CLR_AXIS);
      HPEN hOldPenAxis = (HPEN)SelectObject(hdc, hPenAxis);

      // Left axis
      MoveToEx(hdc, rcPlot.left, rcPlot.top, NULL);
      LineTo(hdc, rcPlot.left, rcPlot.bottom);

      // Bottom axis
      MoveToEx(hdc, rcPlot.left, rcPlot.bottom, NULL);
      LineTo(hdc, rcPlot.right, rcPlot.bottom);

      SelectObject(hdc, hOldPenAxis);
      DeleteObject(hPenAxis);

      // Marker line (Drawn before the curve so the curve renders on top of it.)
      if(bMarkerVisible) {
        // Map the normalised Y fraction to a data space value, the to pixels
        float fMarkerY = fYMin + fMarkerNorm * (fYMax - fYMin);
        int iMarkerPx = MapY(fMarkerY, fYMin, fYMax, rcPlot);

        HPEN hPenMarker = CreatePen(PS_DOT, 1, clr_marker);
        HPEN hOldPenMarker = (HPEN)SelectObject(hdc, hPenMarker);

        MoveToEx(hdc, rcPlot.left, iMarkerPx, NULL);
        LineTo(hdc, rcPlot.right, iMarkerPx);

        SelectObject(hdc, hOldPenMarker);
        DeleteObject(hPenMarker);
      }

      // Curve
      // Only draw if there are at least 2 points
      if(iCount >= 2 && arrX != nullptr && arrY != nullptr) {
        // Guard against degenerate axis ranges
        if(fXMax != fXMin && fYMax != fYMin) {
          HPEN hPenCurve = CreatePen(PS_SOLID, 1, clr_curve);
          HPEN hOldPenCurve = (HPEN)SelectObject(hdc, hPenCurve);

          MoveToEx(hdc, MapX(arrX[0], fXMin, fXMax, rcPlot), MapY(arrY[0], fYMin, fYMax, rcPlot), NULL);

          for(int i = 1; i < iCount; i++) {
            LineTo(hdc, MapX(arrX[i], fXMin, fXMax, rcPlot), MapY(arrY[i], fYMin, fYMax, rcPlot));
          }

          SelectObject(hdc, hOldPenCurve);
          DeleteObject(hPenCurve);
        }
      }
    }

  private:
    /// @brief Maps a data dpace X value to a pixel X coordinate within the inner plot area
    /// @param fVale Data value to map
    /// @param fMin Data minimum
    /// @param fMax Data maximum
    /// @param rcPlot Inner plot rect
    /// @return Pixel X coordinate
    static int MapX(float fVal, float fMin, float fMax, const RECT& rcPlot) {
      int iPlotW = rcPlot.right - rcPlot.left;
      return rcPlot.left + (int)((fVal - fMin) / (fMax - fMin) * iPlotW);
    }

    /// @brief Maps a data space Y value to a pixel Y coordinate
    ///   Inverts the axis so higher values appear higher on the screen
    /// @param fVal Data value to map
    /// @param fMin Data minim
    /// @param fMax Data maximum
    /// @param rcPlot Inner plot rect
    /// @return Y coordinate
    static int MapY(float fVal, float fMin, float fMax, const RECT& rcPlot) {
      int iPlotH = rcPlot.bottom - rcPlot.top;
      // Invert
      return rcPlot.bottom - (int)((fVal - fMin) / (fMax - fMin) * iPlotH);
    }

    RECT rc_panel; // Bounding rect of the whole panel
    wchar_t arr_title[64]; // plot title
    wchar_t arr_label_x[32]; // X axis label
    wchar_t arr_label_y[32]; // Y axis label

    float fMarkerNorm; // Normalised Y position of the marker [0,1]
    BOOL bMarkerVisible; // whether the marker line is drawn

    COLORREF clr_curve; // Curve line color
    COLORREF clr_marker; // Marker line color
};