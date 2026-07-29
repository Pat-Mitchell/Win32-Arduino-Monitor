/// @file Utils.h
/// @brief Project-wide utility function.

#pragma once
#include "..\pch.h"

#ifndef M_PI
#define M_PI  3.14159265
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD  M_PI / 180.f
#endif

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

/// @brief Formats a label, value, and unit int a display string.
///   e.g. FormatReadout(buf, 32, L'V", 4.87f, 2, L"V") -> L"V": 4.87 v"
///   Used by ReadoutPanel-style displays to keep formatting consistent
///   across all projects
/// @param arrBuf Desitination buffer
/// @param iBufLen Buffer size in characters
/// @param szLabel Field label e.g. L"V"
/// @param fValue value to display
/// @param iDecimalPlaces Number of decimal places to show
/// @param szUnit Unit suffix e.g. L"V", L"mA", L"C"
void FormatReadout(wchar_t* arrBuf, int iBufLen, const wchar_t* szLabel, float fValue, int iDecimalPlaces, const wchar_t* szUnit);

/// @brief Linearly maps fVal from one range to another
///   Equivalent to Arduino's map() but for floats and with no
///   integer truncation. Does not clamp. Call ClampFloat first
///   if the input may exceed [fInMin, fInMax].
/// @param fVal Input value map.
/// @param fInMin Lower bound of the input range
/// @param fInMax Upper bound of the input range
/// @param fOutMin Lower bound of the output range
/// @param fOutMax Upper bound of the output range
/// @return Mapped value in the output range
float MapFloat(float fVal, float fInMin, float fInMax, float fOutMin, float fOutMax);

/// @brief Clamps fVal to the range [fMin, fMax]
/// @param fVal Value to clamp
/// @param fMin Minimum allowed value
/// @param fMax Maximum allowed value
/// @return Clamped value
float ClampFloat(float fVal, float fMin, float fMax);

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

/// @brief Draws a straight arrow from (iX1, iY1) to (iX2, iY2).
///   The arowhead is a filled triangle at the tip, with two barbs
///   offset +/-30 degrees from the stem direction.
/// @param hdc Device context to draw into.
/// @param iX1 Stem start X in pixels
/// @param iY1 Stem start Y in pixels
/// @param iX2 Arrow tip X in pixels
/// @param iY2 Arrow tip Y in pixels
/// @param iHeadSize Length of the arrowhead barbs in pixels
/// @param clr Arrow color
void DrawArrow(HDC hdc, int iX1, int iY1, int iX2, int iY2, int iHeadSize, COLORREF clr);

/// @brief Draws a circular arc with an arrowhead at the end.
///   The head is tangent to the arc at the endpoint so it reads
///   naturally as a rotational direction indicator.
/// @param hdc Device context to draw into.
/// @param iCX Center X of the arc circle in pixels
/// @param iCY Center Y of the arc circle in pixels
/// @param iRadius Radius of the arc in pixels
/// @param fStartAngle Start angle in degrees. 0 = right, clockwise positive
/// @param fSwepAngle Sweep in degrees. Positive = clockwise, negative = counter-clockwise
/// @param iHeadSize Length of the arrowhead barbs in pixels
/// @param clr Arrow color
void DrawArcArrow(HDC hdc, int iCX, int iCY, int iRadius, float fStartAngle, float fSwepAngle, int iHeadSize, COLORREF clr);