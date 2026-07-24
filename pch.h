/// @file pch.h
/// @brief Precompiled header. Includes stable, rarely-changing headers.
///        Compile once with build_pch.bat - g++ uses the .gch automatically.

#pragma once

// Force UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// Windows API - by far the heaviest include in this project
#include <windows.h>

// C standard headers used across the project
#include <stdlib.h>     // wcstof, wcstol
#include <string.h>     // wcslen, wcsstr, wcschr

// Wrapper classes — compiled separately but their headers are stable
#include "wrappers\win32Wrappers\UIElement.h"
#include "wrappers\win32Wrappers\Window.h"
#include "wrappers\win32Wrappers\Button.h"
#include "wrappers\win32Wrappers\Label.h"
#include "wrappers\win32Wrappers\PlotPanel.h"
#include "wrappers\win32Wrappers\TextInput.h"
#include "wrappers\win32Wrappers\ComboBox.h"
#include "wrappers\win32Wrappers\Trackbar.h"
#include "wrappers\serialPortWrappers\SerialPort.h"
#include "wrappers\xinputWrappers\XInputController.h"