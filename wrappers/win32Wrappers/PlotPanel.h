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

class PlotPanel {
  public:
    /// @brief Constructs a PlotPanel occuying the given rect
    /// @param rcPanel Bounding rect in parent client coordinates
    /// @param szTitle Title drawn inside the top of the plot area
    /// @param szLabelX X axis label drawn below the plot
    /// @param szLabelY Y axis label drawn left of the plot
    PlotPanel(RECT rcPanel, const wchar_t* szTitle, const wchar_t* szLabelX, const wchar_t* szLabelY);

    /// @brief Enables a horizontal threshold marker line
    ///   The marker is drawn at the Y value cooresponding to
    ///   fNormalised Y * (fMax - fMin) + fYmin in data space,
    ///   so pass a normalised [0,1] fraction of the Y range.
    ///   e.g. 0.632f draws the RC 63.2% tau marker
    ///   Call with bVisible = FALSE to hide it
    /// @param fNormalisedY Fraction of the Y axis range [0,1]
    /// @param bVisible 
    void SetMarker(float fNormalisedY, BOOL bVisible = TRUE);

    /// @brief Overrides the default curve color (green)
    /// @param clr clr New curve color as a COLORREF 
    void SetCurveColor(COLORREF clr);

    /// @brief Overrides the default marker colot (amber)
    /// @param clr New marker color as a COLORREF
    void SetMarkerColor(COLORREF clr);

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
    void Draw(HDC hdc, const float* arrX, const float* arrY, int iCount, float fXMin, float fXMax, float fYMin, float fYMax) const;

  private:
    /// @brief Maps a data dpace X value to a pixel X coordinate within the inner plot area
    /// @param fVale Data value to map
    /// @param fMin Data minimum
    /// @param fMax Data maximum
    /// @param rcPlot Inner plot rect
    /// @return Pixel X coordinate
    static int MapX(float fVale, float fMin, float fMax, const RECT& rcPlot);

    /// @brief Maps a data space Y value to a pixel Y coordinate
    ///   Inverts the axis so higher values appear higher on the screen
    /// @param fVal Data value to map
    /// @param fMin Data minim
    /// @param fMax Data maximum
    /// @param rcPlot Inner plot rect
    /// @return Y coordinate
    static int MapY(float fVal, float fMin, float fMax, const RECT& rcPlot);

    RECT rc_panel; // Bounding rect of the whole panel
    wchar_t arr_title[64]; // plot title
    wchar_t arr_label[32]; // X axis label
    wchar_t arr_label[32]; // Y axis label

    float fMarkerNorm; // Normalised Y position of the marker [0,1]
    BOOL bMarkerVisible; // whether the marker line is drawn

    COLORREF clr_curve; // Curve line color
    COLORREF clr_marker; // Marker line color
};