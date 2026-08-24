/// @file Oscilloscope.cpp

#include "Oscilloscope.h"

namespace {
  const COLORREF kBg        = RGB( 15,  15,  20);
  const COLORREF kGrid      = RGB( 38,  38,  48);
  const COLORREF kGridCtr   = RGB( 58,  58,  72);
  const COLORREF kZeroLine  = RGB( 90,  90, 110);
  const COLORREF kWaveform  = RGB( 50, 210, 110);  // oscilloscope green
  const COLORREF kTrigLine  = RGB(200, 140,  30); 
  const COLORREF kAxisText  = RGB(130, 130, 150);
  const COLORREF kNoSig     = RGB( 80,  80, 100);
}

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Exports the oscilloscope frame as time / ADC / V_out / V_sig columns
/// @param szPath Full path from save dialog
/// @param panel Panel containing the current frame and metadata
/// @return TRUE on success
BOOL ExportCSV(const wchar_t* szPath, const OscilloscopePanel& panel) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  DWORD dw = 0;
  const char* szHdr = "Time_ms,ADC_count,Vout_V,Vsig_mV\r\n";
  WriteFile(hFile, szHdr, lstrlenA(szHdr), &dw, NULL);

  float fDt_ms = 1000.0f / SCOPE_SAMPLE_RATE;

  for(int i = 0; i < panel.iCount; i++) {
    float fT = i * fDt_ms;
    int iAdc = panel.arrSamples[i];
    float fVout = (iAdc / 1023.0f) * panel.fVcc;
    float fVsig_mV = panel.AdcToVsig(iAdc) * 1000.0f;

    char arrRow[96];
    int iT_whole = (int)fT;
    int iT_frac = (int)((fT - iT_whole) * 1000);
    int iVo_whole = (int)fVout;
    int iVo_frac = (int)((fVout - iVo_whole) * 1000);
    int iVs = (int)fVsig_mV;

    int iLen = wsprintfA(arrRow, "%d.%03d,%d,%d.%03d,%d\r\n", iT_whole, iT_frac, iAdc, iVo_whole, iVo_frac, iVs);
    WriteFile(hFile, arrRow, iLen, &dw, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// OscilloscopePanel
// ────── ⋆⋅☆⋅⋆ ────────

void OscilloscopePanel::Init(int iX, int iY, int iW, int iH) {
  rect_bounds = { iX, iY, iX + iW, iY + iH };
  iPadL = 56;
  iPadT = 20;
  iPadR = 20;
  iPadB = 36;

  fVcc = 5.0;
  fGain = 10.0f;
  fVbias = 1.75f;
  Clear();
}

void OscilloscopePanel::Clear() {
  iCount = 0;
  bHasFrame = false;
  bLastTriggered = false;
  iTrigADC = 360;
  fLastVpp = 0.0f;
  fLastVrms = 0.0f;
  fLastFreq = -1.0f;
  ZeroMemory(arrSamples, sizeof(arrSamples));
}

void OscilloscopePanel::UpdateDisplayParams(float fNewGain, float fNewVbias) {
  fGain = (fNewGain > 0.0f) ? fNewGain : 1.0f;
  fVbias = fNewVbias;
  if(bHasFrame) {
    ComputeMeasurements();
  }
}

void OscilloscopePanel::LoadFrame(const int* arrData, int iN, float fNewVcc, float fNewGain, float fNewVbias, int iTrig, bool bTriggered) {
  iCount = (((iN) < (MAX_SCOPE_SAMPLES)) ? (iN) : (MAX_SCOPE_SAMPLES));
  for(int i = 0; i < iCount; i++) {
    arrSamples[i] = arrData[i];
  }

  fVcc = fNewVcc;
  fGain = (fNewGain > 0.0f) ? fNewGain : 1.0f;
  fVbias = fNewVbias;
  iTrigADC = iTrig;
  bLastTriggered = bTriggered;
  bHasFrame = true;

  ComputeMeasurements();
}

float OscilloscopePanel::AdcToVsig(int iAdc) const {
  float fVout = (iAdc / 1023.0f) * fVcc;
  // Inverting amplifer: Vin = Vbias - (Vout - Vbias) / Gain
  // Vsig = Vin - Vbias = -(Vout - Vbias) / Gain = (Vbias - Vout) / Gain
  return (fVbias - fVout) / fGain;
}

void OscilloscopePanel::ComputeMeasurements() {
  if(iCount < 2) {
    fLastVpp = 0;
    fLastVrms = 0;
    fLastFreq = -1.0f;
    return;
  }

  float fMin = AdcToVsig(arrSamples[0]);
  float fMax = fMin;
  float fSumSq = 0.0f;
  int iUpCrossings = 0;
  float fPrev = fMin;

  for(int i = 0; i < iCount; i++) {
    float fV = AdcToVsig(arrSamples[i]);
    if(fV < fMin) {
      fMin = fV;
    }
    if(fV > fMax) {
      fMax = fV;
    }
    fSumSq += fV * fV;

    // Upward zero crossing (signal crosses from <= 0 to > 0)
    if(fPrev < 0.0f && fV > 0.0f) {
      iUpCrossings++;
    }
    fPrev = fV;
  }

  fLastVpp = fMax - fMin;
  fLastVrms = sqrtf(fSumSq / iCount);

  // Frequency: each upward crossing = one full period
  if(iUpCrossings >= 2) {
    float fTotalTime = (float)iCount / SCOPE_SAMPLE_RATE;
    fLastFreq = (float)iUpCrossings / fTotalTime;
  } else {
    fLastFreq = -1.0f;
  }
}

int OscilloscopePanel::MapX(int iIdx) const {
  int iW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
  if(iCount <=1) {
    return rect_bounds.left + iPadL;
  }
  return rect_bounds.left + iPadL + (int)((float)iIdx / (iCount - 1) * iW);
}

int OscilloscopePanel::MapY(float fVsig, float fVmin, float fVmax) const {
  int iH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
  float fRange = fVmax - fVmin;
  if(fRange < 0.0001f) {
    fRange = 0.0001f;
  }
  return rect_bounds.top + iPadT + iH - (int)(((fVsig - fVmin) / fRange) * iH);
}

void OscilloscopePanel::Draw(HDC hdc) const {
  int iL = rect_bounds.left;
  int iT = rect_bounds.top;
  int iR = rect_bounds.right;
  int iB = rect_bounds.bottom;

  // Plot area inner bounds
  int iPL = iL + iPadL;
  int iPT = iT + iPadT;
  int iPR = iR - iPadR;
  int iPB = iB - iPadB;
  int iPlotW = iPR - iPL;
  int iPlotH = iPB - iPT;

  // Background
  HBRUSH hbr = CreateSolidBrush(kBg);
  RECT rfull = rect_bounds;
  FillRect(hdc, &rfull, hbr);
  DeleteObject(hbr);

  // Y-axis auto-scale
  float fVmin = -0.200f;
  float fVmax = 0.200f;

  if(bHasFrame && iCount > 1) {
    float fObsMin = AdcToVsig(arrSamples[0]);
    float fObsMax = fObsMin;
    for(int i = 1; i < iCount; i++) {
      float fV = AdcToVsig(arrSamples[i]);
      if(fV < fObsMin) {
        fObsMin = fV;
      }
      if(fV > fObsMax) {
        fObsMax = fV;
      }
    }

    float fObsRange = fObsMax - fObsMin;
    if(fObsRange < 0.010f) {
      // No signal
      // use default +/-200mV window
      fVmin = -0.200f;
      fVmax = 0.200f;
    } else {
      float fMargin = fObsRange * 2.0f;
      fVmin = fObsMin - fMargin;
      fVmax = fObsMax + fMargin;
    }
  }

  SetBkMode(hdc, TRANSPARENT);

  // Grid (8x6 divisinos)
  HPEN hpen_grid = CreatePen(PS_SOLID, 1, kGrid);
  HPEN hpen_grid_ctr = CreatePen(PS_SOLID, 1, kGridCtr);
  HPEN hpen_old = (HPEN)SelectObject(hdc, hpen_grid);

  const int iVDivs = 8; // time (horizontal) divisions
  const int iHDivs = 6; // voltage (vertical) divisions

  for(int v = 1; v < iVDivs; v++) {
    int iX = iPL + (iPlotW * v) / iVDivs;
    if(v == iVDivs / 2) {
      SelectObject(hdc, hpen_grid_ctr);
    } else {
      SelectObject(hdc, hpen_grid);
    }
    MoveToEx(hdc, iX, iPT, NULL);
    LineTo(hdc, iX, iPB);
  }
  for(int h = 1; h < iHDivs; h++) {
    int iY = iPT + (iPlotH * h) / iHDivs;
    if(h == iHDivs / 2) {
      SelectObject(hdc, hpen_grid_ctr);
    } else {
      SelectObject(hdc, hpen_grid);
    }
    MoveToEx(hdc, iPL, iY, NULL);
    LineTo(hdc, iPR, iY);
  }

  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_grid);
  DeleteObject(hpen_grid_ctr);

  // Axes
  HPEN hpen_ax = CreatePen(PS_SOLID, 1, kZeroLine);
  SelectObject(hdc, hpen_ax);
  MoveToEx(hdc, iPL, iPT, NULL);
  LineTo(hdc, iPL, iPB);
  MoveToEx(hdc, iPL, iPB, NULL);
  LineTo(hdc, iPR, iPB);
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_ax);

  // Y axis labels
  SetTextColor(hdc, kAxisText);

  wchar_t arrBuf[24];
  float fVmid = (fVmin + fVmax) / 2.0f;

  // Top, mid, bottom
  wsprintf(arrBuf, L"%d", (int)(fVmax * 1000));
  TextOut(hdc, iL + 2, MapY(fVmax, fVmin, fVmax) - 6, arrBuf, lstrlen(arrBuf));
  wsprintf(arrBuf, L"%d", (int)(fVmid * 100));
  TextOut(hdc, iL + 2, MapY(fVmid, fVmin, fVmax) - 6, arrBuf, lstrlen(arrBuf));
  wsprintf(arrBuf, L"%d", (int)(fVmin * 1000));
  TextOut(hdc, iL + 2, MapY(fVmin, fVmin, fVmax) - 6, arrBuf, lstrlen(arrBuf));

  // Y axis unit label
  TextOut(hdc, iL + 2, iPT - 14, L"mV", 2);

  // X axis labels
  float fTotalMs = (float)(((iCount) > (1)) ? (iCount) : (1)) / SCOPE_SAMPLE_RATE * 1000.0f;
  int iTms_whole = (int)fTotalMs;
  int iTms_frac = (int)((fTotalMs - iTms_whole) * 10);
  wsprintf(arrBuf, L"%d.%dms", iTms_whole, iTms_frac);
  TextOut(hdc, iPR - 28, iPB + 6, arrBuf, lstrlen(arrBuf));
  TextOut(hdc, iPL - 4, iPB = 6, L"0", 1);

  // Zero line (V_sig = 0, i.e. no imput signal)
  int iY0 = MapY(0.0f, fVmin, fVmax);
  if(iY0 >= iPT && iY0 <= iPB) {
    HPEN hpen_zero = CreatePen(PS_DOT, 1, kZeroLine);
    SelectObject(hdc, hpen_zero);
    MoveToEx(hdc, iPL, iY0, NULL);
    LineTo(hdc, iPR, iY0);
    SelectObject(hdc, hpen_old);
    DeleteObject(hpen_zero);

    SetTextColor(hdc, kZeroLine);
    TextOut(hdc, iPL + 4, iY0 - 12, L"0", 1);
  }

  // Trigger level line
  float fTrigVsig = AdcToVsig(iTrigADC);
  int iYtrig = MapY(fTrigVsig, fVmin, fVmax);
  if(iYtrig >= iPT && iYtrig <= iPB) {
    HPEN hpen_trig = CreatePen(PS_DASH, 1, kTrigLine);
    SelectObject(hdc, hpen_trig);
    MoveToEx(hdc, iPL, iYtrig, NULL);
    LineTo(hdc, iPR, iYtrig);
    SelectObject(hdc, hpen_old);
    DeleteObject(hpen_trig);

    SetTextColor(hdc, kTrigLine);
    TextOut(hdc, iPL + 4, iYtrig - 12, L"T", 1);
  }

  // No Frame placeholder
  if(!bHasFrame || iCount < 2) {
    SetTextColor(hdc, kNoSig);
    RECT rcCenter = { iPL, iPT, iPR, iPB };
    DrawText(hdc, L"No signal - connect and start streaming", -1, &rcCenter, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return;
  }

  // Waveform
  HPEN hpen_wave = CreatePen(PS_SOLID, 2, kWaveform);
  SelectObject(hdc, hpen_wave);

  MoveToEx(hdc, MapX(0), MapY(AdcToVsig(arrSamples[0]), fVmin, fVmax), NULL);
  for(int i = 1; i < iCount; i++) {
    LineTo(hdc, MapX(i), MapY(AdcToVsig(arrSamples[i]), fVmin, fVmax));
  }

  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_wave);
}

OscilloscopeWindow::OscilloscopeWindow()
  : cmb_port(nullptr)
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , btn_trig_toggle(nullptr)
  , btn_trig_rise(nullptr)
  , btn_trig_fall(nullptr)
  , trk_trig(nullptr)
  , lbl_trig_val(nullptr)
  , edit_samples(nullptr)
  , edit_gain(nullptr)
  , edit_vbias(nullptr)
  , btn_apply(nullptr)
  , btn_export(nullptr)
  , lbl_vpp_val(nullptr)
  , lbl_vrms_val(nullptr)
  , lbl_freq_val(nullptr)
  , lbl_vcc_val(nullptr)
  , lbl_status(nullptr)
  , iLineBufLen(0)
  , bExpectingData(false)
  , iExpectedSamples(256)
  , fFrameVcc(5.0f)
  , fFrameGain(10.0f)
  , iFrameTrig(360)
  , bFrameTriggered(false)
  , fCurrentVcc(5.0f)
  , fCurrentGain(10.0f)
  , fCurrentVbias(1.75f)
  , bAutoTrigger(true)
  {
    ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
  }

  OscilloscopeWindow::~OscilloscopeWindow() {
  delete cmb_port;
  delete btn_connect;
  delete btn_disc;
  delete btn_trig_toggle;
  delete btn_trig_rise;
  delete btn_trig_fall;
  delete trk_trig;
  delete lbl_trig_val;
  delete edit_samples;
  delete edit_gain;
  delete edit_vbias;
  delete btn_apply;
  delete btn_export;
  delete lbl_vpp_val;
  delete lbl_vrms_val;
  delete lbl_freq_val;
  delete lbl_vcc_val;
  delete lbl_status;
  }

  void OscilloscopeWindow::OnCreate() {
    // Connection bar
    new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
    cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 120, 200);
    btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 318, 14, 100, 28);
    btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 318, 14, 100, 28);
    new Label(hwnd_self, L"115200 baud", 428, 18, 100, 24);
    btn_disc->Disable();
    ScanComPorts(cmb_port);

    // Trigger controls
    new Label(hwnd_self, L"Trigger:", 16, 58, 60 ,22);
    btn_trig_toggle = new Button(hwnd_self, L"Auto", ID_BTN_TRIG_TOGGLE, 78, 52, 70, 28);
    btn_trig_rise = new Button(hwnd_self, L"Rise \x2191", ID_BTN_TRIG_RISE, 156, 52, 72, 28);
    btn_trig_fall = new Button(hwnd_self, L"Fall \x2193", ID_BTN_TRIG_FALL, 236, 52, 72, 28);
    new Label(hwnd_self, L"Level:", 316, 58, 46, 22);
    // Trackbar range 0-716 (0V to 2.5V op-amp output range)
    trk_trig = new Trackbar(hwnd_self, ID_TRACKBAR_TRIG, 366, 48, 190, 38, 0, 716);
    trk_trig->SetPos(360);
    trk_trig->SetTickFreq(71); // One tick per -0.35V
    lbl_trig_val = new Label(hwnd_self, L"ADC (±0 mV)", 562, 58, 120, 22);
    btn_trig_rise->Disable();
    btn_trig_fall->Disable();

    // Settings bar
    new Label(hwnd_self, L"Samples:", 16, 102, 64, 22);
    edit_samples = new TextInput(hwnd_self, ID_EDIT_SAMPLES, 84, 96, 52, 28);
    edit_samples->SetText(L"256");

    new Label(hwnd_self, L"Gain:", 144, 102, 38, 22);
    edit_gain = new TextInput(hwnd_self, ID_EDIT_GAIN, 186, 96, 50, 28);
    edit_gain->SetText(L"10");

    new Label(hwnd_self, L"V_bias (V):", 244, 102, 74, 22);
    edit_vbias = new TextInput(hwnd_self, ID_EDIT_VBIAS, 322, 96, 56, 28);
    edit_vbias->SetText(L"1.75");

    btn_apply = new Button(hwnd_self, L"Apply", ID_BTN_APPLY, 386, 96, 70, 28);
    btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 464, 96, 100, 28);
    btn_export->Disable();

    // Measurement readouts
    new Label(hwnd_self, L"V_pp:", 16, 136, 40, 22);
    new Label(hwnd_self, L"V_rms:", 178, 136, 44, 22);
    new Label(hwnd_self, L"Freq:", 340, 136, 38, 22);
    new Label(hwnd_self, L"VCC:", 488, 136, 36, 22);

    lbl_vpp_val = new Label(hwnd_self, L"---", 58, 136, 110, 22);
    lbl_vrms_val = new Label(hwnd_self, L"---", 226, 136, 106, 22);
    lbl_freq_val = new Label(hwnd_self, L"---", 382, 136, 100, 22);
    lbl_vcc_val = new Label(hwnd_self, L"---", 528, 136, 90, 22);

    // Status
    lbl_status = new Label(hwnd_self, L"Disconnected", 16, 163, 580, 18);

    // Oscilloscope panel
    panel.Init(16, 186, 648, 390);
  }

  void OscilloscopeWindow::OnPaint(HDC hdc) {
    // Fill non-panel client area to prevent redraw flicker
    RECT rcClient;
    GetClientRect(hwnd_self, &rcClient);

    RECT rcTop = { 0, 0, rcClient.right, 186 };
    RECT rcBtm = { 0, 576, rcClient.right, rcClient.bottom };
    HBRUSH hbr = (HBRUSH)(COLOR_WINDOW + 1);
    FillRect(hdc, &rcTop, hbr);
    FillRect(hdc, &rcBtm, hbr);

    panel.Draw(hdc);
  }

  void OscilloscopeWindow::OnCommand(int iControlId, int iNotifCode) {
    switch(iControlId) {
      case ID_BTN_CONNECT:
        OnConnect();
        break;

      case ID_BTN_DISCONNECT:
        OnDisconnect();
        break;

      case ID_BTN_TRIG_TOGGLE:
        SetTriggerMode(!bAutoTrigger);
        if(!bAutoTrigger) {
          port.Write("TRIG:AUTO");
        } else {
          // Resend current trigger level on switch to triggered
          char arrCmd[16];
          wsprintfA(arrCmd, "TRIG:%d", trk_trig->GetPos());
          port.Write(arrCmd);
        }
        break;

      case ID_BTN_TRIG_RISE:
        port.Write("TRIG:RISE");
        lbl_status->SetText(L"Trigger: Rising edge");
        break;

      case ID_BTN_TRIG_FALL:
        port.Write("TRIG:FALL");
        lbl_status->SetText(L"Trigger: Falling edge");
        break;

      case ID_BTN_APPLY:
        ApplySettings();
        break;

      case ID_BTN_EXPORT: {
        if(!panel.bHasFrame) {
          MessageBox(hwnd_self, L"No frame to export", L"Export", MB_OK | MB_ICONWARNING);
          return;
        }
        wchar_t arrPath[MAX_PATH];
        if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0All Files\0*.*\0", L"csv", arrPath, MAX_PATH)) {
          return;
        }
        if(!ExportCSV(arrPath, panel)) {
          MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
        }
        break;
      }
    }
  }

  void OscilloscopeWindow::OnScroll(HWND hwnd_control, int iCode) {
    if(!trk_trig) {
      return;
    }
    if(hwnd_control != trk_trig->GetHandle()) {
      return;
    }
    if(!port.IsOpen()) {
      return;
    }

    int iPos = trk_trig->GetPos();
    UpdateTrigLabel(iPos);

    // Send new trigger level to Arduino
    char arrCmd[16];
    wsprintfA(arrCmd, "TRIG:%d", iPos);
    port.Write(arrCmd);

    // Update panel trigger line and repaint
    panel.iTrigADC = iPos;
    InvalidateRect(hwnd_self, NULL, FALSE);
  }

  void OscilloscopeWindow::OnTimer(int iTimerId) {
    if(iTimerId != ID_TIMER_POLL) {
      return;
    }
    if(!port.IsOpen()) {
      return;
    }

    char arrRaw[READ_BUF];
    DWORD dwRead = 0;
    port.Read(arrRaw, READ_BUF, dwRead);
    if(dwRead == 0) {
      return;
    }

    // Append to persistent line buffer
    if(iLineBufLen + (int)dwRead >= (int)sizeof(arrLineBuf)) {
      iLineBufLen = 0;
      ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
      bExpectingData = false;
    }

    memcpy(arrLineBuf + iLineBufLen, arrRaw, dwRead);
    iLineBufLen += (int)dwRead;
    arrLineBuf[iLineBufLen] = '\0';

    char* pLine = arrLineBuf;
    char* pEnd = nullptr;
    bool bRepaint = false;

    while((pEnd = strchr(pLine, '\n')) != nullptr) {
      *pEnd = '\0';
      int iLen = (int)strlen(pLine);
      if(iLen > 0 && pLine[iLen - 1] == '\r') {
        pLine[iLen - 1] = '\0';
      }

      wchar_t arrWide[READ_BUF * 2];
      MultiByteToWideChar(CP_ACP, 0, pLine, -1, arrWide, READ_BUF * 2);

      if(ParseLine(arrWide)) bRepaint = true;
      pLine = pEnd + 1;
    }

    int iRemaining = iLineBufLen - (int)(pLine - arrLineBuf);
    if(iRemaining > 0) memmove(arrLineBuf, pLine, iRemaining);
    iLineBufLen = iRemaining;
    arrLineBuf[iLineBufLen] = '\0';

    if(bRepaint) {
      InvalidateRect(hwnd_self, NULL, FALSE);
    }
  }

  void OscilloscopeWindow::OnDestroy() {
    KillTimer(hwnd_self, ID_TIMER_POLL);
    port.Close();
  }

  bool OscilloscopeWindow::ParseLine(const wchar_t* szLine) {
    // FRAME: header. Store metadata, expect data next
    if(wcsstr(szLine, L"FRAME:")) {
      iExpectedSamples = (int)ParseFloat(szLine, L"FRAME:");
      fFrameVcc = ParseFloat(szLine, L"VCC:");
      fFrameGain = ParseFloat(szLine, L"GAIN:");
      iFrameTrig = (int)ParseFloat(szLine, L"TRIG:");
      float fOk = ParseFloat(szLine, L"TRIG_OK:");
      bFrameTriggered = (fOk > 0.5f);

      if(fFrameVcc > 0.0f) {
        fCurrentVcc = fFrameVcc;
      }
      if(fFrameGain > 0.0f) {
        fCurrentGain = fFrameGain;
      }

      // update VCC label live from every frame header
      wchar_t arrBuf[24];
      int iW = (int)fCurrentVcc;
      int iF = (int)((fCurrentVcc - iW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iW, iF);
      lbl_vcc_val->SetText(arrBuf);

      // Update status
      lbl_status->SetText(bAutoTrigger ? L"Auto - streaming" : (bFrameTriggered ? L"Triggered" : L"Triggered - timeout"));

      bExpectingData = (iExpectedSamples > 0 && iExpectedSamples <= MAX_SCOPE_SAMPLES);
      return false;
    }

    // Data line. Comma-separated ADC values
    if(bExpectingData) {
      bExpectingData = false;

      int arrTemp[MAX_SCOPE_SAMPLES];
      int iActual = 0;
      if(ParseDataLine(szLine, arrTemp, iExpectedSamples, iActual) && iActual >=2) {
        panel.LoadFrame(arrTemp, iActual, fCurrentVcc, fCurrentGain, fCurrentVbias, iFrameTrig, bFrameTriggered);
        UpdateReadouts();
        btn_export->Enable();
        return true; // Repaint
      }
      return false;
    }

    // VCC message (startup or MEASURE_VCC response)
    if(wcsstr(szLine, L"VCC:") && !wcsstr(szLine, L"FRAME:")) {
      float fV = ParseFloat(szLine, L"VCC:");
      if(fV > 0.0f) {
        fCurrentVcc = fV;
        wchar_t arrBuf[16];
        int iW = (int)fV;
        int iF = (int)((fV - iW) * 1000);
        wsprintf(arrBuf, L"%d.%03d V", iW, iF);
        lbl_vcc_val->SetText(arrBuf);
      }
      return false;
    }
    return false;
  }

  bool OscilloscopeWindow::ParseDataLine(const wchar_t* szLine, int* arrOut, int iExpected, int& iActual) {
    iActual = 0;
    const wchar_t* p = szLine;

    while(*p && iActual < iExpected) {
      wchar_t* pEnd = nullptr;
      long lVal = wcstol(p, &pEnd, 10);
      if(pEnd == p) {
        break;
      }

      arrOut[iActual++] = (int)lVal;

      if(*pEnd == L',') {
        p = pEnd + 1;
      } else {
        p = pEnd;
      }

      return (iActual > 0);
    }
  }

  int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    OscilloscopeWindow win_main;
    win_main.Create(hInstance, L"Op-Amp Oscilloscope", 680, 600);
    win_main.Show(nCmdShow);
    return win_main.Run();
  }