#include "LRCResonance.h"

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV
// ────── ⋆⋅☆⋅⋆ ────────

BOOL ExportCSV(const wchar_t* szPath, const float* arrFreq, const float* arrV, int iCount) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) return FALSE;

  DWORD dw = 0;
  const char* szHdr = "Frequency_Hz,VOltage_V\r\n";
  WriteFile(hFile, szHdr, lstrlenA(szHdr), &dw, NULL);

  for(int i = 0; i < iCount; i++) {
    char arrRow[64];
    int iW = (int)arrV[i];
    int iF = (int)((arrV[i] - iW) * 10000);
    int iLen = wsprintfA(arrRow, "%d,%d.%04d\r\n", (int)arrFreq[i], iW, iF);
    WriteFile(hFile, arrRow, iLen, &dw, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// LRCWindow
// ────── ⋆⋅☆⋅⋆ ────────

LRCWindow::LRCWindow()
  : cmb_port(nullptr) 
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , edit_fstart(nullptr)
  , edit_fend(nullptr)
  , edit_steps(nullptr)
  , btn_sweep(nullptr)
  , btn_stop(nullptr)
  , edit_lval(nullptr)
  , edit_cval(nullptr)
  , edit_vpin(nullptr)
  , btn_setvpin(nullptr)
  , prog_sweep(nullptr)
  , btn_export(nullptr)
  , lbl_f0_meas(nullptr)
  , lbl_f0_theory(nullptr)
  , lbl_error(nullptr)
  , lbl_status(nullptr)
  , lbl_vcc(nullptr)
  , bSweepActive(false)
  , iExpectedSteps(0)
  , iReceivedSteps(0)
  , fVpin(4.85f)
  , iLineBufLen(0)
{
  ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
}

LRCWindow::~LRCWindow() {
  delete cmb_port;
  delete btn_connect;
  delete btn_disc;
  delete edit_fstart;
  delete edit_fend;
  delete edit_steps;
  delete btn_sweep;
  delete btn_stop;
  delete edit_lval;
  delete edit_cval;
  delete edit_vpin;
  delete btn_setvpin;
  delete prog_sweep;
  delete btn_export;
  delete lbl_f0_meas;
  delete lbl_f0_theory;
  delete lbl_error;
  delete lbl_status;
  delete lbl_vcc;
}