#include "../../../pch.h"
#include "../../../Utils/Utils.h"

#define MAX_SAMPLES 512

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