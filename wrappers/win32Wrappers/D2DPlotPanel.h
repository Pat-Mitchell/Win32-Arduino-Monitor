/// @file D2DPlotPanel.h
/// @brief Reusable Direct2D line plot panel.
///
/// Caller-passes-buffer patter: Panel owns no data.

#pragma once
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

// ────── ⋆⋅☆⋅⋆ ────────
// Color Palette
// ────── ⋆⋅☆⋅⋆ ────────
namespace D2DPlotColors {
  inline D2D1::ColorF Background() { return D2D1::ColorF(0.078f, 0.078f, 0.094f); }
  inline D2D1::ColorF Border() { return D2D1::ColorF(0.314f, 0.314f, 0.314f); }
  inline D2D1::ColorF Axis() { return D2D1::ColorF(0.392f, 0.392f, 0.392f); }
  inline D2D1::ColorF Label() { return D2D1::ColorF(0.706f, 0.706f, 0.706f); }
  inline D2D1::ColorF Title() { return D2D1::ColorF(0.863f, 0.863f, 0.863f); }
  inline D2D1::ColorF CurveDef() { return D2D1::ColorF(0.196f, 0.784f, 0.392f); }
  inline D2D1::ColorF MarkerDef() { return D2D1::ColorF(0.784f, 0.549f, 0.118f); }
}

// ────── ⋆⋅☆⋅⋆ ────────
// Layout constants
// ────── ⋆⋅☆⋅⋆ ────────
namespace D2DPlotLayout {
  inline constexpr float MarginTop = 28.0f;
  inline constexpr float MarginBottom = 32.0f;
  inline constexpr float MarginLeft = 14.0f;  
  inline constexpr float MarginRight = 10.0f;  
}

class D2DPlotPanel {
  public:
    D2DPlotPanel(D2D1_RECT_F rcPanel, ID2D1Factory* pFactory, const wchar_t* szTitle, const wchar_t* szLabelX, const wchar_t* szLabelY)
      : rc_panel(rcPanel)
      , pFactory(pFactory)
      , fMarkerNorm(0.0f)
      , bMarkerVisible(FALSE)
      , fCurveWidth(1.5f)
      , clr_curve(D2DPlotColors::CurveDef())
      , clr_marker(D2DPlotColors::MarkerDef())
      , pStrokeDash(nullptr)
    {
      wcsncpy(arr_title, szTitle, 63);
      arr_title[63] = L'\0';
      wcsncpy(arr_label_x, szLabelX, 31);
      arr_label_x[31] = L'\0';
      wcsncpy(arr_label_y, szLabelY, 31);
      arr_label_y[31] = L'\0';

      // Create the dashed stroke style for marker lines
      // Device-independent. Created from factory. Survives device loss
      if(pFactory) {
        float fDashes[] = { 5.0f, 4.0f };
        D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties();
        props.dashStyle = D2D1_DASH_STYLE_CUSTOM;
        pFactory->CreateStrokeStyle(props, fDashes, 2, &pStrokeDash);
      }
    }

    ~D2DPlotPanel() {
      if(pStrokeDash) {
        pStrokeDash->Release();
        pStrokeDash = nullptr;
      }
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Configuration
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Updates the bounding rect. Call after a WM_SIZE if the panel
    ///   tracks a fraction of the window size.
    void SetRect(D2D1_RECT_F rcNew) {
      rc_panel = rcNew;
    }

    /// @brief Enables a horizontal threshold marker line at a normalised Y position
    /// @param fNormalisedY Fraction of the Y axis range [0,1]. 0.632f = RC tau marker.
    /// @param bVisible Pass FALSE to hide the marker
    void SetMarker(float fNormalisedY, BOOL bVisible = TRUE) {
      fMarkerNorm = fNormalisedY;
      bMarkerVisible = bVisible;
    }

    void SetCurveColor(D2D1::ColorF clr) {
      clr_curve = clr;
    }
    void SetMarkerColor(D2D1::ColorF clr) {
      clr_marker = clr;
    }
    void SetCurveWidth(float fWidth) {
      fCurveWidth = fWidth;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Rendering
    // ────── ⋆⋅☆⋅⋆ ────────
    void Draw(ID2D1HwndRenderTarget* pRT, IDWriteFactory* pDWrite, IDWriteTextFormat* pFmtSmall, IDWriteTextFormat* pFmtTitle, const float* arrX, const float * arrY, int iCount, float fXMin, float fXMax, float fYMin, float fYMax) const {
      using namespace D2DPlotLayout;
      if(!pRT) {
        return;
      }

      D2D1_RECT_F rcPlot = {
        rc_panel.left + MarginLeft,
        rc_panel.top + MarginTop,
        rc_panel.right - MarginRight,
        rc_panel.bottom - MarginBottom
      };

      // Background
      {
        auto* pBrush = MakeBrush(pRT, D2DPlotColors::Background());
        if(pBrush) {
          pRT->FillRectangle(rc_panel, pBrush);
          pBrush->Release();
        }
      }
      {
        auto* pBrush = MakeBrush(pRT, D2DPlotColors::Border());
        if(pBrush) {
          pRT->DrawRectangle(rc_panel, pBrush, 1.0f);
          pBrush->Release();
        }
      }

      // Title
      if(pFmtTitle) {
        D2D1_RECT_F rcTitle = {
          rc_panel.left, rc_panel.top + 4.0f, rc_panel.right, rc_panel.top + MarginTop
        };
        PutText(pRT, pDWrite, pFmtTitle, arr_title, D2DPlotColors::Title(), rcTitle, DWRITE_TEXT_ALIGNMENT_CENTER);
      }

      // X axis label
      if(pFmtSmall) {
        D2D1_RECT_F rcLX = {
          rcPlot.left, rc_panel.bottom - MarginBottom + 6.0f, rcPlot.right, rc_panel.bottom - 2.0f
        };
        PutText(pRT, pDWrite, pFmtSmall, arr_label_x, D2DPlotColors::Label(), rcLX, DWRITE_TEXT_ALIGNMENT_CENTER);
      }

      // Y axis label (rotated)
      if(pFmtSmall) {
        D2D1_MATRIX_3X2_F prev;
        pRT->GetTransform(&prev);

        float fCX = rc_panel.left + 10.0f;
        float fCY = (rcPlot.top + rcPlot.bottom) / 2.0f;

        pRT->SetTransform(D2D1::Matrix3x2F::Rotation(-90.0f, D2D1::Point2F(fCX, fCY)));

        D2D1_RECT_F rcLY = { fCX - 60.0f, fCY - 10.0f, fCX + 60.0f, fCY + 10.0f };

        PutText(pRT, pDWrite, pFmtSmall, arr_label_y, D2DPlotColors::Label(), rcLY, DWRITE_TEXT_ALIGNMENT_CENTER);

        pRT->SetTransform(prev);
      }

      // Axes
      {
        auto* pBrush = MakeBrush(pRT, D2DPlotColors::Axis());
        if(pBrush) {
          pRT->DrawLine(D2D1::Point2F(rcPlot.left, rcPlot.top), D2D1::Point2F(rcPlot.left, rcPlot.bottom), pBrush, 1.0f);
          pRT->DrawLine(D2D1::Point2F(rcPlot.left, rcPlot.bottom), D2D1::Point2F(rcPlot.right, rcPlot.bottom), pBrush, 1.0f);
          pBrush->Release();
        }
      }

      // Marker line
      // Drawn before the curve so the curve renders on top of it.
      if(bMarkerVisible && fYMax != fYMin) {
        float fMarkerY = fYMin + fMarkerNorm + (fYMax - fYMin);
        float fPx = MapY(fMarkerY, fYMin, fYMax, rcPlot);
        auto* pBrush = MakeBrush(pRT, clr_marker);
        if(pBrush) {
          // pStrokeDash is device-independent. Safe to reuse across frames
          pRT->DrawLine(D2D1::Point2F(rcPlot.left, fPx), D2D1::Point2F(rcPlot.right, fPx), pBrush, 1.0f, pStrokeDash);
          pBrush->Release();
        }
      }

      // Curve
      // Uses ID2D1PathGeometry to draw the full polyline in a single 
      // DrawGeometry call. More efficient than one DrawLine pe segment
      // at high sameple counts (512+)
      if(iCount >= 2 && arrX && arrY && fXMax != fXMin && fYMax != fYMin && pFactory) {
        ID2D1PathGeometry* pGeometry = nullptr;
        if(SUCCEEDED(pFactory->CreatePathGeometry(&pGeometry))) {
          ID2D1GeometrySink* pSink = nullptr;
          if(SUCCEEDED(pGeometry->Open(&pSink))) {
            pSink->BeginFigure(D2D1::Point2F(MapX(arrX[0], fXMin, fXMax, rcPlot), MapY(arrY[0], fYMin, fYMax, rcPlot)), D2D1_FIGURE_BEGIN_HOLLOW);

            for(int i = 1; i < iCount; i++) {
              pSink->AddLine(D2D1::Point2F(MapX(arrX[i], fXMin, fXMax, rcPlot), MapY(arrY[i], fYMin, fYMax, rcPlot)));
            }

            pSink->EndFigure(D2D1_FIGURE_END_OPEN);
            pSink->Close();
            pSink->Release();

            auto* pBrush = MakeBrush(pRT, clr_curve);
            if(pBrush) {
              pRT->DrawGeometry(pGeometry, pBrush, fCurveWidth);
              pBrush->Release();
            }
          }
          pGeometry->Release();
        }
      }
    }

  private:
    // ────── ⋆⋅☆⋅⋆ ────────
    // Coordinate Mapping
    // ────── ⋆⋅☆⋅⋆ ────────
    // Returns floats. no integer truncation, inherently sub-pixel precise.
    static float MapX(float fVal, float fMin, float fMax, const D2D1_RECT_F& rc) {
      return rc.left + (fVal - fMin) / (fMax - fMin) * (rc.right - rc.left);
    }
    static float MapY(float fVal, float fMin, float fMax, const D2D1_RECT_F& rc) {
      // Invert: Larger values map to smaller Y (up on screen)
      return rc.bottom - (fVal - fMin) / (fMax - fMin) * (rc.bottom - rc.top);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Brush helper
    // ────── ⋆⋅☆⋅⋆ ────────
    static ID2D1SolidColorBrush* MakeBrush(ID2D1HwndRenderTarget* pRT, D2D1::ColorF clr) {
      ID2D1SolidColorBrush* pBrush = nullptr;
      pRT->CreateSolidColorBrush(clr, &pBrush);
      return pBrush;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Text helper
    // ────── ⋆⋅☆⋅⋆ ────────
    static void PutText(ID2D1HwndRenderTarget* pRT, IDWriteFactory* pDWrite, IDWriteTextFormat* pFmt, const wchar_t* szText, D2D1::ColorF clr, D2D1_RECT_F rcLayout, DWRITE_TEXT_ALIGNMENT eAlign) {
      if(!szText || !pFmt || !pRT) {
        return;
      }
      pFmt->SetTextAlignment(eAlign);
      pFmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

      auto* pBrush = MakeBrush(pRT, clr);
      if(pBrush) {
        pRT->DrawText(szText, (UINT32)wcslen(szText), pFmt, rcLayout, pBrush);
        pBrush->Release();
      }
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────
    D2D1_RECT_F rc_panel;
    ID2D1Factory* pFactory;
    wchar_t arr_title[64];
    wchar_t arr_label_x[32];
    wchar_t arr_label_y[32];

    float fMarkerNorm;
    BOOL bMarkerVisible;
    float fCurveWidth;

    D2D1::ColorF clr_curve;
    D2D1::ColorF clr_marker;

    ID2D1StrokeStyle* pStrokeDash;
};