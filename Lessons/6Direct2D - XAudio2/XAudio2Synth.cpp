/// @file XAudio2Synth.cpp

#include "XAudio2Synth.h"

void GenerateWave(int16_t* arrBuf, int iSamples, float fFreq_Hz, WaveShape eShape) {
  for(int i = 0; i < iSamples; i++) {
    float fT = (float)i / SAMPLE_RATE;
    float fPhase = fFreq_Hz * fT;
    float fSample = 0.0f;

    switch(eShape) {
      case SINE:
        fSample = sinf(2.0f * M_PI * fPhase);
        break;
      case SQUARE:
        fSample = (sinf(2.0f * M_PI * fPhase) >= 0.0f) ? 1.0f : -1.0f;
        break;
      case SAWTOOTH:
        fSample = 2.0f * (fPhase - floorf(fPhase)) - 1.0f;
        break;
    }

    arrBuf[i] = (int16_t)(fSample * AMPLITUDE);
  }
}

void BuildPlotData(float* arrX, float* arrY, int iPoints, float fFreq_Hz, WaveShape eShape) {
  float fPeriod_ms = 1000.0f / fFreq_Hz;

  for(int i = 0; i < iPoints; i++) {
    float fT_norm = (float)i / (iPoints - 1);
    arrX[i] = fT_norm * fPeriod_ms;

    switch(eShape) {
      case SINE:
        arrY[i] = sinf(2.0f * M_PI * fT_norm);
        break;
      case SQUARE:
        arrY[i] = (sinf(2.0f * M_PI * fT_norm) >= 0.0f) ? 1.0f : -1.0f;
        break;
      case SAWTOOTH:
        arrY[i] = 2.0f * fT_norm - 1.0f;
        break;
    }
  }
}

XAudio2Window::XAudio2Window()
  : pXAudio2(nullptr)
  , pMasterVoice(nullptr)
  , pSourceVoice(nullptr)
  , pPlot(nullptr)
  , fFreq(440.0f)
  , eShape(SINE)
  , bPlaying(false)
  , edit_freq(nullptr)
  , btn_apply(nullptr)
  , btn_startstop(nullptr)
  , lbl_status(nullptr)
  , hwnd_radio_sine(NULL)
  , hwnd_radio_square(NULL)
  , hwnd_radio_saw(NULL)
  {
    ZeroMemory(arrAudioBuf, sizeof(arrAudioBuf));
    ZeroMemory(arrPlotX, sizeof(arrPlotX));
    ZeroMemory(arrPlotY, sizeof(arrPlotY));
  }

XAudio2Window::~XAudio2Window() {
  StopAudio();
  ReleaseXAudio2();

  delete pPlot;
  delete edit_freq;
  delete btn_apply;
  delete btn_startstop;
  delete lbl_status;
}

void XAudio2Window::OnCreate() {
  // Controls
  new Label(hwnd_self, L"Frequency (HZ):", 16, 20, 110, 24);
  edit_freq = new TextInput(hwnd_self, ID_EDIT_FREQ, 132, 16, 80, 28);
  edit_freq->SetText(L"440");
  btn_apply = new Button(hwnd_self, L"Apply", ID_BTN_APPLY, 220, 16, 70, 28);

  new Label(hwnd_self, L"Shape:", 16, 60, 52, 24);

  hwnd_radio_sine = CreateWindowEx(0, L"BUTTON", L"Sine", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 76, 58, 80, 24, hwnd_self, (HMENU)ID_RADIO_SINE, NULL, NULL);
  hwnd_radio_square = CreateWindowEx(0, L"BUTTON", L"Square", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 164, 58, 80, 24, hwnd_self, (HMENU)ID_RADIO_SQUARE, NULL, NULL);
  hwnd_radio_saw = CreateWindowEx(0, L"BUTTON", L"Sawtooth", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 252, 58, 90, 24, hwnd_self, (HMENU)ID_RADIO_SAW, NULL, NULL);

  SendMessage(hwnd_radio_sine, BM_SETCHECK, BST_CHECKED, 0);

  btn_startstop = new Button(hwnd_self, L"Start", ID_BTN_STARTSTOP, 16, 96, 90, 30);
  lbl_status = new Label(hwnd_self, L"Stopped | 440 Hz | Sine", 116, 102, 400, 22);

  // D2DPlotPanel
  // Created here because s_pFactory only exists after D2DWindow::InitD2D()
  //   has been called from WinMain.
  //   pRT is also valid at this point. CreateDeviceResources ran first
  pPlot = new D2DPlotPanel (
    D2D1::RectF(16.0f, 140.0f, 624.0f, 420.0f),
    s_pFactory,
    L"Waveform Preview (1 period)",
    L"Time (ms)",
    L"Amplitude"
  );

  // XAudio2
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if(FAILED(hr)) {
    lbl_status->SetText(L"CoInitializeEx failed.");
    return;
  }

  hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
  if(FAILED(hr)) {
    lbl_status->SetText(L"Audio2Create failed.");
    return;
  }

  hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
  if(FAILED(hr)) {
    lbl_status->SetText(L"CreateMasteringVoice failed.");
    return;
  }

  RefreshPlot();
}

// Called by D2DWindow between BeginDraw/EndDraw
void XAudio2Window::OnD2DPaint(ID2D1HwndRenderTarget* pRT) {
  // Clear the full surface. Native controls repoaint themselves on top
  //   Use the system window color so the controls strip matches.
  pRT->Clear(ColorF(GetSysColor(COLOR_WINDOW)));

  if(!pPlot) {
    return;
  }

  float fPeriod_ms = 1000.0f / fFreq;

  // D2DPlotPanel::Draw takes the render target and DirectWrite resources directly
  //   No HDC, no GDI objects.
  pPlot->Draw(
    pRT,
    pDWrite, // IDWriteFactory* from D2DWindow base
    pFmtSmall, // IDWriteTextFormat* from D2DWindow base 
    pFmtTitle, // IDWriteTextFormat* from D2DWindow base
    arrPlotX, arrPlotY, PLOT_POINTS,
    0.0f, fPeriod_ms, -1.1f, 1.1f
  );
}

// D2DWindow calls this after resizing the render target.
//   for a fixed-size window this is a no-op, but it's the hook for 
// updating pPlot->SetRect() if the layout tracks the window size.
void XAudio2Window::OnSize(UINT iW, UINT iH) {}

// OnCommand
void XAudio2Window::OnCommand(int iControlId, int iNotifCode) {
  switch(iControlId) {
    case ID_BTN_APPLY:
      ApplyFrequency();
      break;
    case ID_BTN_STARTSTOP:
      if(bPlaying) {
        StopAudio();
      } else {
        StartAudio();
      }
      break;
    case ID_RADIO_SINE:
      eShape = SINE;
      OnSettingsChanged();
      break;
    case ID_RADIO_SQUARE:
      eShape = SQUARE;
      OnSettingsChanged();
      break;
    case ID_RADIO_SAW:
      eShape = SAWTOOTH;
      OnSettingsChanged();
      break;
  }
}

void XAudio2Window::OnDestroy() {
  StopAudio();
  ReleaseXAudio2();
  CoUninitialize();
}

void XAudio2Window::StartAudio() {
  if(!pXAudio2 || !pMasterVoice) {
    return;
  }

  if(pSourceVoice) {
    pSourceVoice->Stop(0);
    pSourceVoice->DestroyVoice();
    pSourceVoice = nullptr;
  }

  GenerateWave(arrAudioBuf, SAMPLE_RATE, fFreq, eShape);

  WAVEFORMATEX wfx = {};
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 1;
  wfx.nSamplesPerSec = SAMPLE_RATE;
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
  wfx.cbSize = 0;

  HRESULT hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx);
  if(FAILED(hr)) {
    lbl_status->SetText(L"CreateSourceVoice failed.");
    return;
  }

  XAUDIO2_BUFFER buf = {};
  buf.AudioBytes = sizeof(arrAudioBuf);
  buf.pAudioData = reinterpret_cast<const BYTE*>(arrAudioBuf);
  buf.LoopCount = XAUDIO2_LOOP_INFINITE;
  buf.Flags = 0;

  pSourceVoice->SubmitSourceBuffer(&buf);
  pSourceVoice->Start(0);

  bPlaying = true;
  btn_startstop->SetText(L"Stop");
  UpdateStatusLabel();
}

void XAudio2Window::StopAudio() {
  if(pSourceVoice) {
    pSourceVoice->Stop(0);
    pSourceVoice->FlushSourceBuffers();
    pSourceVoice->DestroyVoice();
    pSourceVoice = nullptr;
  }
  bPlaying = false;
  if(btn_startstop) {
    btn_startstop->SetText(L"Start");
  }
  if(lbl_status) {
    UpdateStatusLabel();
  }
}

void XAudio2Window::ReleaseXAudio2() {
  if(pMasterVoice) {
    pMasterVoice->DestroyVoice();
    pMasterVoice = nullptr;
  }
  if(pXAudio2) {
    pXAudio2->Release();
    pXAudio2 = nullptr;
  }
}

void XAudio2Window::ApplyFrequency() {
  wchar_t arrBuf[16];
  edit_freq->GetText(arrBuf, 16);
  float fNew = wcstof(arrBuf, nullptr);

  fNew = (fNew < 20.0f) ? 20.0f : fNew;
  fNew = (fNew > 20000.0f) ? 20000.0f : fNew;
  fFreq = fNew;

  // swprintf for float formatting. wsprintf doesn't support %f
  swprintf(arrBuf, 16, L"%.0f", fFreq);
  edit_freq->SetText(arrBuf);

  OnSettingsChanged();
}

void XAudio2Window::OnSettingsChanged() {
  RefreshPlot();
  if(bPlaying) {
    StopAudio();
    StartAudio();
  }
  UpdateStatusLabel();
}

void XAudio2Window::RefreshPlot() {
  BuildPlotData(arrPlotX, arrPlotY, PLOT_POINTS, fFreq, eShape);
  InvalidateRect(hwnd_self, NULL, FALSE);
}

void XAudio2Window::UpdateStatusLabel() {
  if(!lbl_status) {
    return;
  }

  const wchar_t* szShape = (eShape == SINE) ? L"Sine" : (eShape == SQUARE) ? L"Square" : L"Sawtooth";

  const wchar_t* szState = bPlaying ? L"Playing" : L"Stopped";

  wchar_t arrBuf[64];
  // swprintf flor float formatting. wsprintf doesn't support %.0f
  swprintf(arrBuf, 64, L"%ls | %.0f Hz | %ls", szState, fFreq, szShape);
  lbl_status->SetText(arrBuf);
}