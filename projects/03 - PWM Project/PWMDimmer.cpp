/// @file PWMDimmer.cpp
/// @brief PWM LED Dimmer with Frequency Display.
///   Slider controls duty cycle 0-100%. Sends PWM:xx commands
///   to the Arduino and displays duty cycle, theoretical V_avg.
///   and ADC-measured V_avg as live readouts.

#include "../../pch.h"
#include "../../Utils/Utils.h"
#include "../../wrappers/win32Wrappers/Window.h"
#include "../../wrappers/win32Wrappers/Button.h"
#include "../../wrappers/win32Wrappers/Label.h"
#include "../../wrappers/win32Wrappers/ComboBox.h"
#include "../../wrappers/win32Wrappers/Trackbar.h"
#include "../../wrappers/serialPortWrappers/SerialPort.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT        101
#define ID_BTN_CONNECT       102
#define ID_BTN_DISCONNECT    103
#define ID_TRACKBAR_DUTY     104
#define ID_BTN_RESET         105
#define ID_BTN_RECORD_RIPPLE 106
#define ID_BTN_STEP_RESPONSE 107
#define ID_BTN_EXPORT        108
#define ID_TIMER_POLL        1

#define POLL_MS     200
#define READ_BUF    256
#define MAX_SAMPLES 512

// Recording durations in milliseconds
#define RIPPLE_DURATION_MS 5000
#define STEP_PHASE1_MS     2000
#define STEP_PHASE2_MS     18000

// Step response duty cycle values
#define STEP_DUTY_LOW  10
#define STEP_DUTY_HIGH 90

enum RecordState {
  REC_IDLE,
  REC_RIPPLE,       // Recording at fixed duty for 5s
  REC_STEP_PHASE1,  // Recording at 10% for 1s
  REC_STEP_PHASE2   // Recording at 90% for 10s
};

// ────── ⋆⋅☆⋅⋆ ────────
// Plot Panel
// ────── ⋆⋅☆⋅⋆ ────────
struct PlotPanel {
  float arrTime[MAX_SAMPLES];
  float arrVolt[MAX_SAMPLES];
  int iCount;
  int iPadL, iPadT, iPadR, iPadB;
  RECT rect_bounds;

  void Init(int iX, int iY, int iW, int iH) {
    rect_bounds = { iX, iY, iX + iW, iY + iH };
    iPadL = 50; iPadT = 16; iPadR = 16; iPadB = 36;
    Clear();
  }

  void AddSample(float fT, float fV) {
    if(iCount >= MAX_SAMPLES) return;
    arrTime[iCount] = fT;
    arrVolt[iCount] = fV;
    iCount++;
  }

  void Clear() {
    iCount = 0;
    ZeroMemory(arrTime, sizeof(arrTime));
    ZeroMemory(arrVolt, sizeof(arrVolt));
  }

  int MapX(float fT, float fTmax) const {
    int iPlotW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
    return rect_bounds.left + iPadL + (int)((fT / fTmax) * iPlotW);
  }

  int MapY(float fV) const {
    int iPlotH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
    return rect_bounds.top + iPadT + iPlotH - (int)((fV / 5.0f) * iPlotH);
  }

  /// @brief Draws a dashed horizontal reference line at a given voltage.
  ///        Used to mark V_avg targets on the step response plot.
  /// @param hdc      Device context
  /// @param fV       Voltage level for the line
  /// @param color    Line color
  /// @param szLabel  Label string drawn at the left end of the line
  void DrawHLine(HDC hdc, float fV, COLORREF color, const wchar_t* szLabel) const {
    HPEN hpen = CreatePen(PS_DASH, 1, color);
    SelectObject(hdc, hpen);

    int iY = MapY(fV);
    MoveToEx(hdc, rect_bounds.left + iPadL, iY, NULL);
    LineTo(hdc, rect_bounds.right - iPadR, iY);

    DeleteObject(hpen);

    SetTextColor(hdc, color);
    TextOut(hdc, rect_bounds.left + iPadL + 4, iY - 14, szLabel, lstrlen(szLabel));
  }

  /// @brief Renders the full plot. Background, axes, reference lines, and data curve
  /// @param hdc Device context
  /// @param eRecState Current record state. Controls which reference lines are drawn
  void Draw(HDC hdc, RecordState eRecState) const {
    int iL = rect_bounds.left;
    int iT = rect_bounds.top;
    int iR = rect_bounds.right;
    int iB = rect_bounds.bottom;

    // Background
    HBRUSH hbr_bg = CreateSolidBrush(RGB(20, 20, 20));
    RECT rect_fill = rect_bounds;
    FillRect(hdc, &rect_fill, hbr_bg);
    DeleteObject(hbr_bg);

    // Axes
    HPEN hpen_axis = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    SelectObject(hdc, hpen_axis);
    MoveToEx(hdc, iL + iPadL, iT + iPadT, NULL);
    LineTo (hdc, iL + iPadL, iB - iPadB);
    MoveToEx(hdc, iL + iPadL, iB - iPadB, NULL);
    LineTo  (hdc, iR - iPadR, iB - iPadB);
    DeleteObject(hpen_axis);

    // Y axis labels
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(140, 140, 140));
    TextOut(hdc, iL + iPadL - 36, MapY(5.0f) - 6, L"5V",  2);
    TextOut(hdc, iL + iPadL - 36, MapY(2.5f) - 6, L"2.5", 3);
    TextOut(hdc, iL + iPadL - 36, MapY(0.0f) - 6, L"0V",  2);
    TextOut(hdc, iL + iPadL - 4,  iB - iPadB + 6, L"0",   1);

    // ────── ⋆⋅☆⋅⋆ ────────
    // Reference lines. Context-dependent on which test is active.
    // Ripple test: one line at V_avg for the current duty cycle.
    // Step test:   two lines at 10% and 90% V_avg targets.
    // ────── ⋆⋅☆⋅⋆ ────────
    if(eRecState == REC_RIPPLE || eRecState == REC_IDLE) {
      // Only draw if we have data — use first sample's voltage as target
      if(iCount > 0) {
        float fTarget = arrVolt[0];
        DrawHLine(hdc, fTarget, RGB(80, 160, 80), L"V avg");
      }
    } else if (eRecState == REC_STEP_PHASE1 || eRecState == REC_STEP_PHASE2) {
      // 10% target: 0.5V, 90% target: 4.5V
      DrawHLine(hdc, STEP_DUTY_LOW  / 100.0f * 5.0f,
      RGB(80, 160, 255),  L"10% (0.5V)");
      DrawHLine(hdc, STEP_DUTY_HIGH / 100.0f * 5.0f,
      RGB(255, 160, 80),  L"90% (4.5V)");
    }

    // Data curve
    if (iCount < 2) return;

    float fTmax = arrTime[iCount - 1];
    if (fTmax < 1.0f) fTmax = 1.0f;

    HPEN hpen_curve = CreatePen(PS_SOLID, 2, RGB(80, 180, 255));
    SelectObject(hdc, hpen_curve);
    MoveToEx(hdc, MapX(arrTime[0], fTmax), MapY(arrVolt[0]), NULL);
    for(int i = 1; i < iCount; i++) {
      LineTo(hdc, MapX(arrTime[i], fTmax), MapY(arrVolt[i]));
    }      
    DeleteObject(hpen_curve);

    // X axis time label at end of curve
    wchar_t arrTmax[16];
    wsprintf(arrTmax, L"%dms", (int)fTmax);
    SetTextColor(hdc, RGB(140, 140, 140));
    TextOut(hdc, iR - iPadR - 28, iB - iPadB + 6, arrTmax, lstrlen(arrTmax));
    }
};

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV. time/voltage pairs with duty cycle column
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Writes the plot dataset to CSV with a duty cycle header note.
/// @param szPath     Full file path.
/// @param arrTime    Time array in milliseconds.
/// @param arrVolt    Voltage array in volts.
/// @param iCount     Number of samples.
/// @return TRUE on success.
BOOL ExportCSV(const wchar_t* szPath, const float* arrTime, const float* arrVolt, int iCount) {
    HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD dwWritten = 0;
    const char* szHeader = "Time_ms,Voltage_V\r\n";
    WriteFile(hFile, szHeader, lstrlenA(szHeader), &dwWritten, NULL);

    for(int i = 0; i < iCount; i++) {
      char arrRow[64];
      int  iWhole = (int)arrVolt[i];
      int  iFrac  = (int)((arrVolt[i] - iWhole) * 1000);
      int  iLen   = wsprintfA(arrRow, "%d,%d.%03d\r\n", (int)arrTime[i], iWhole, iFrac);
      WriteFile(hFile, arrRow, iLen, &dwWritten, NULL);
    }

    CloseHandle(hFile);
    return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// PWMDimmerWindow
// ────── ⋆⋅☆⋅⋆ ────────
class PWMDimmerWindow : public Window {
  public:
    PWMDimmerWindow()
      : cmb_port(nullptr)
      , btn_connect(nullptr)
      , btn_disc(nullptr)
      , btn_reset(nullptr)
      , btn_record_ripple(nullptr)
      , btn_step_response(nullptr)
      , btn_export(nullptr)
      , trk_duty(nullptr)
      , lbl_trk_duty(nullptr)
      , lbl_vavg_theory(nullptr)
      , lbl_vavg_measured(nullptr)
      , lbl_status(nullptr)
      , iLastDuty(-1)
      , iLineBufLen(0)
      , eRecState(REC_IDLE)
      , lRecStart(0)
      , lPhaseStart(0)
    {
      ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
    }

    ~PWMDimmerWindow() {
      delete cmb_port;
      delete btn_connect;
      delete btn_disc;
      delete btn_reset;
      delete btn_record_ripple;
      delete btn_step_response;
      delete btn_export;
      delete trk_duty;
      delete lbl_trk_duty;
      delete lbl_vavg_theory;
      delete lbl_vavg_measured;
      delete lbl_status;
    }

  protected:
    
    // ────── ⋆⋅☆⋅⋆ ────────
    // OnCreate
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCreate() override {
      // --- Connection bar ---
      new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
      cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 130, 200);
      btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 230, 14, 90, 28);
      btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 328, 14, 100, 28);

      btn_disc->Disable();
      ScanComPorts(cmb_port);

      // --- Duty cycle slider ---
      new Label(hwnd_self, L"Duty Cycle:", 16, 66, 80, 24);

      // Trackbar spans most of the window width
      trk_duty = new Trackbar(hwnd_self, ID_TRACKBAR_DUTY, 100, 60, 380, 40, 0, 100);

      // Live duty % label to the right of the slider
      lbl_trk_duty = new Label(hwnd_self, L"0%", 450, 66, 50, 24);

      // Reset button
      btn_reset = new Button(hwnd_self, L"Reset", ID_BTN_RESET, 508, 60, 60, 28);
      btn_reset->Disable();

      // -- Readout panel ---
      new Label(hwnd_self, L"V avg (theory):",  16,  112, 120, 22);
      new Label(hwnd_self, L"V avg (measured):", 16, 136, 120, 22);
      lbl_vavg_theory   = new Label(hwnd_self, L"---", 142, 112, 100, 22);
      lbl_vavg_measured = new Label(hwnd_self, L"---", 142, 136, 100, 22);

      // Recording buttons
      btn_record_ripple = new Button(hwnd_self, L"Record Ripple (5s)", ID_BTN_RECORD_RIPPLE, 16, 168, 160, 28);
      btn_step_response = new Button(hwnd_self, L"Step Response (11s)", ID_BTN_STEP_RESPONSE, 184, 168, 168, 28);
      btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 360, 168, 100, 28);

      btn_record_ripple->Disable();
      btn_step_response->Disable();
      btn_export->Disable();

      // Status label. Updated duting recording sequences
      lbl_status = new Label(hwnd_self, L"", 16, 202, 560, 22);

      // plot panel. Filles lower portion
      plot.Init(16, 228, 592, 290);

      UpdateDutyLabels(0);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnPaint
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnPaint(HDC hdc) override {
      plot.Draw(hdc, eRecState);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnCommand -> connect / disconnect / reset
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCommand(int iControlId, int iNotifCode) override {
      switch(iControlId) {
        case ID_BTN_CONNECT: OnConnect(); break;
        case ID_BTN_DISCONNECT: OnDisconnect(); break;
        case ID_BTN_RESET:
          trk_duty->SetPos(0);
          SendDuty(0);
          UpdateDutyLabels(0);
          break;
        case ID_BTN_RECORD_RIPPLE:
          StartRippleRecord();
          break;
        case ID_BTN_STEP_RESPONSE:
          StartStepResponse();
          break;
        case ID_BTN_EXPORT: {
          if(plot.iCount == 0) {
            MessageBox(hwnd_self, L"No data to export.", L"Export", MB_OK | MB_ICONWARNING);
            return;
          }
          wchar_t arrPath[MAX_PATH];
          if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0All Files\0*.*\0", L"csv", arrPath, MAX_PATH))
            return;
          if(!ExportCSV(arrPath, plot.arrTime, plot.arrVolt, plot.iCount))
            MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
          break;
        }
      }
    }
    
    // ────── ⋆⋅☆⋅⋆ ────────
    // OnScroll -> fires when the trackbar thumb moves.
    // TB_THUMBTRACK fires continuously while dragging.
    // TB_ENDTRACK fires when the thumb is released
    // We send on every tick for a live feel. The Arduino
    // clamps and applies immediately on receipt.
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnScroll(HWND hwnd_control, int iCode) override {
      if(!trk_duty) return;
      if(hwnd_control != trk_duty->GetHandle()) return;
      if(eRecState != REC_IDLE) return; // Lock slider during recording

      int iDuty = trk_duty->GetPos();

      // Only send if the value actually changed. Avoids flooding serial
      // with identical messages when the user holds the slider steady
      if(iDuty == iLastDuty) return;
      iLastDuty = iDuty;
      SendDuty(iDuty);
      UpdateDutyLabels(iDuty);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnTimer
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnTimer(int iTimerId) override {
      if(iTimerId != ID_TIMER_POLL) return;
      if(!port.IsOpen()) return;

      char arrRaw[READ_BUF];
      DWORD dwRead = 0;
      port.Read(arrRaw, READ_BUF, dwRead);
      if(dwRead > 0) {
          // Accumulate into line buffer
          if(iLineBufLen + (int)dwRead >= (int)sizeof(arrLineBuf)) {
            iLineBufLen = 0;
            ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
          }

          memcpy(arrLineBuf + iLineBufLen, arrRaw, dwRead);
          iLineBufLen += (int)dwRead;
          arrLineBuf[iLineBufLen] = '\0';

          // Extract complete lines
          char* pLine = arrLineBuf;
          char * pEnd = nullptr;
          bool bRepaint = false;

          while((pEnd = strchr(pLine, '\n')) != nullptr) {
            *pEnd = '\0';

            int iLen = (int)strlen(pLine);
            if(iLen > 0 && pLine[iLen - 1] == '\r') pLine[iLen - 1] = '\0';

            wchar_t arrWide[READ_BUF];
            MultiByteToWideChar(CP_ACP, 0, pLine, -1, arrWide, READ_BUF);
            
            // Parse and record if active
            float fVmeas = 0.0f;
            if(ParseVAvg(arrWide, fVmeas)) {
              UpdateMeasuredLabel(fVmeas);

              if(eRecState != REC_IDLE) {
                long lElapsed = (long)(GetTickCount() - lRecStart);
                plot.AddSample((float)lElapsed, fVmeas);
                bRepaint = true;
              }
            }

            pLine = pEnd + 1;
          }

          // Shift remaining fragment
          int iRemaining = iLineBufLen - (int)(pLine - arrLineBuf);
          if(iRemaining > 0) memmove(arrLineBuf, pLine, iRemaining);
          iLineBufLen = iRemaining;
          arrLineBuf[iLineBufLen] = '\0';

          if(bRepaint) InvalidateRect(hwnd_self, NULL, FALSE);
      }

      // --- Recording state machine ---
      TickStateMachine();
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnDestroy
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnDestroy() override {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();
    }

  private:

    // ────── ⋆⋅☆⋅⋆ ────────
    // Recording
    // Start functions
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Clears the plot abd begins a 5-second recording at the current duty cycle.
    ///   Disables controls for the duration.
    void StartRippleRecord() {
      plot.Clear();
      eRecState = REC_RIPPLE;
      lRecStart = (long)GetTickCount();
      lPhaseStart = lRecStart;
      
      lbl_status->SetText(L"Recording ripple - 5 seconds at current duty cycle...");
      SetRecordingControls(false);
      btn_export->Disable();
    }

    void StartStepResponse() {
      plot.Clear();
      eRecState = REC_STEP_PHASE1;
      lRecStart = (long)GetTickCount();
      lPhaseStart = lRecStart;

      // Set slider and send 10% duty
      trk_duty->SetPos(STEP_DUTY_LOW);
      iLastDuty = STEP_DUTY_LOW;
      SendDuty(STEP_DUTY_LOW);
      UpdateDutyLabels(STEP_DUTY_LOW);

      lbl_status->SetText(L"Step response - Phase 1: holding at 10% (1s)...");
      SetRecordingControls(false);
      btn_export->Disable();
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // TickStateMachine
    // Called everytimer tick. Checks elapsed time and transitions state.
    // ────── ⋆⋅☆⋅⋆ ────────
    
    /// @brief Advances the recording state machine based on elapsed time.
    ///   Triggers duty cycle changes and terminates recording sequences.
    void TickStateMachine() {
      if(eRecState == REC_IDLE) return;

      long lNow = (long)GetTickCount();
      long lPhaseMs = lNow - lPhaseStart;

      switch(eRecState) {
        case REC_RIPPLE:
          if(lPhaseMs >= RIPPLE_DURATION_MS) {
            eRecState = REC_IDLE;
            lbl_status->SetText(L"Ripple recording complete.");
            SetRecordingControls(true);
            btn_export->Enable();
            InvalidateRect(hwnd_self, NULL, FALSE);
          }
          break;

        case REC_STEP_PHASE1:
          if(lPhaseMs >= STEP_PHASE1_MS) {
            // Transition to Phase 2
            eRecState = REC_STEP_PHASE2;
            lPhaseStart = lNow;

            trk_duty->SetPos(STEP_DUTY_HIGH);
            iLastDuty = STEP_DUTY_HIGH;
            SendDuty(STEP_DUTY_HIGH);
            UpdateDutyLabels(STEP_DUTY_HIGH);

            lbl_status->SetText(L"Step response - Phase 2: stepping to 90% (10s)...");
          }
          break;

        case REC_STEP_PHASE2:
          if(lPhaseMs >= STEP_PHASE2_MS) {
            eRecState = REC_IDLE;
            lbl_status->SetText(L"Step response complete.");
            SetRecordingControls(true);
            btn_export->Enable();
            InvalidateRect(hwnd_self, NULL, FALSE);
          }
          break;
        
        default: break;
      }
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // SendDuty
    // Formats and sends "PWM:xx\n" to the Arduino
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Sends a duty cycle command over serial
    /// @param iDuty Duty cycle 0-100
    void SendDuty(int iDuty) {
      if(!port.IsOpen()) return;

      char arrCmd[16];
      wsprintfA(arrCmd, "PWM:%d", iDuty);
      port.Write(arrCmd);
    }

    /// @brief Updates the duty cycle % and theorectical V_avg labels.
    /// @param iDuty Duty cycle 0-100
    void UpdateDutyLabels(int iDuty) {
      // Duty cycle %
      wchar_t arrBuf[16];
      wsprintf(arrBuf, L"%d%%", iDuty);
      lbl_trk_duty->SetText(arrBuf);

      // Theoretical V_avg = (duty / 100.0) * 5.0
      float fVtheory = (iDuty / 100.0f) * 5.0f;
      int iW = (int)fVtheory;
      int iF = (int)((fVtheory - iW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iW, iF);
      lbl_vavg_theory->SetText(arrBuf);
    }

    void UpdateMeasuredLabel(float fV) {
      wchar_t arrBuf[16];
      int iW = (int)fV;
      int iF = (int)((fV - iW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iW, iF);
      lbl_vavg_measured->SetText(arrBuf);
    }

    /// @brief Enables or disables controls that should be locked
    ///   during a recording sequence.
    /// @param bEnable TRUE ro enable. FALSE to lock.
    void SetRecordingControls(bool bEnable) {
      if(bEnable) {
        btn_record_ripple->Enable();
        btn_step_response->Enable();
        btn_reset->Enable();
        trk_duty->Enable();
      } else {
        btn_record_ripple->Disable();
        btn_step_response->Disable();
        btn_reset->Disable();
        trk_duty->Disable();
      }
    }

    bool ParseVAvg(const wchar_t* szLine, float& fVOut) {
      if(!wcsstr(szLine, L"VAVG:")) return false;
      float fV = ParseFloat(szLine, L"VAVG:");
      if(fV < 0) return false;
      fVOut = fV;
      return true;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnConnect / OnDisconnect
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnConnect() {
      if(cmb_port->GetCount() == 0) {
        MessageBox(hwnd_self, L"No COM ports found.", L"Connect", MB_OK | MB_ICONWARNING);
        return;
      }

      wchar_t arrPortName[16];
      cmb_port->GetSelected(arrPortName, 16);

      if(!port.Open(arrPortName)) {
        wchar_t arrMsg[64];
        wsprintf(arrMsg, L"Failed to open %s.\nError: %lu", arrPortName, port.GetLastErrorCode());
        MessageBox(hwnd_self, arrMsg, L"Connect", MB_OK | MB_ICONERROR);
        return;
      }

      SetTimer(hwnd_self, ID_TIMER_POLL, POLL_MS, NULL);
      btn_connect->Disable();
      btn_disc->Enable();
      btn_reset->Enable();
      btn_record_ripple->Enable();
      btn_step_response->Enable();
      cmb_port->Disable();

      // Send initial 0% on connect
      SendDuty(0);
      UpdateDutyLabels(0);
    }

    void OnDisconnect() {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();

      btn_connect->Enable();
      btn_disc->Disable();
      btn_reset->Disable();
      btn_record_ripple->Disable();
      btn_step_response->Disable();
      btn_export->Disable();
      cmb_port->Enable();

      lbl_vavg_measured->SetText(L"---");
      lbl_status->SetText(L"");
      iLastDuty = -1;

      ScanComPorts(cmb_port);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────
    SerialPort port;
    PlotPanel plot;

    char arrLineBuf[READ_BUF * 4];
    int iLineBufLen = 0;

    RecordState eRecState;
    long lRecStart;
    long lPhaseStart;

    int iLastDuty;

    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    Button* btn_reset;
    Button* btn_record_ripple;
    Button* btn_step_response;
    Button* btn_export;
    Trackbar* trk_duty;
    Label* lbl_trk_duty;
    Label* lbl_vavg_theory;
    Label* lbl_vavg_measured;
    Label* lbl_status;
};

// ────── ⋆⋅☆⋅⋆ ────────
// WinMain
// ────── ⋆⋅☆⋅⋆ ────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  INITCOMMONCONTROLSEX icc = {}; // Required to initialise the common controls library
  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  PWMDimmerWindow win_main;
  win_main.Create(hInstance, L"PWM LED Dimmer", 640, 560);
  win_main.Show(nCmdShow);
  return win_main.Run();
}