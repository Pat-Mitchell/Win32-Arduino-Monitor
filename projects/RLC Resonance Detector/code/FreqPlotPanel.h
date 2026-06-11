#include "../../../pch.h"
#include "../../../Utils/Utils.h"

#define READ_BUF    512
#define MAX_SAMPLES 512

namespace {
  const COLORREF kCurve = RGB(80, 180, 255);
  const COLORREF kPeak = RGB(255, 160, 40);
  const COLORREF kRefLine = RGB(60, 140, 60);
  const COLORREF kAxis = RGB(100, 100, 100);
  const COLORREF kText = RGB(140, 140, 140);
}

struct FreqPlotPanel {
  float arrFreq[MAX_SAMPLES];
  float arrV[MAX_SAMPLES];
  int iCount;
  int iPeakIdx;
  int iPadL, iPadT, iPadR, iPadB;
  RECT rect_bounds;

  void Init(int iX, int iY, int iW, int iH);
  void Clear();
  void AddSample(float fFreq, float fV);
  
  float GetPeakFreq() const;
  float GetPeakV() const;

  int MapXByIdx(int i) const;
  int MapY(float fV, float fVmax) const;

  void Draw(HDC hdc, float fVpin) const;
};