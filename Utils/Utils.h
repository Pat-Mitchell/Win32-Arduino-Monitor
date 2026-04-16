/// @file Utils.h
/// @brief Project-wide utility function.

#pragma once
#include "..\pch.h"

/// @function ScanComPorts
/// @brief Probs COM1-COM20 via CreateFile and populates a ComboBox
///        with available ports. Ports that exist but are busy are 
///        labeled "(in use)". Auto-selects the first entry found.
/// @param cmb_port Target ComboBox to populate 
/// @return void
void ScanComPorts(ComboBox* cmb_port);

/// @function ParseFloat
/// @brief Searches szSrc for szKey and returns the float value
///        immediately following it. Returns -1.0 on failure.
/// @param szSrc The full source string to search.
/// @param szKey Key to locate
/// @return Parsed float or -1.0 if the key is not found.
float ParseFloat(const wchar_t* szSrc, const wchar_t* szKey);

/// @function ShowSaveDialog
/// @brief Opens a Win32 Save As dialog and returns the chosen file path
///        Returns FALSE if the user cancels.
/// @param hwnd_owner Parent window handle
/// @param szFilter File type filter e.g. L"CSV Files\0*.csv\0All Files\0*.*\0"
/// @param szDefExt Default extension e.g. L"csv"
/// @param arrPathOut Buffer to receive the chosen path
/// @param iBufLen Buffer size in characters
/// @return TRUE if a path was chosen, FALSE if cancelled
BOOL ShowSaveDialog(HWND hwnd_owner, const wchar_t* szFilter, const wchar_t* szDefExt, wchar_t* arrPathOut, int iBufLen);