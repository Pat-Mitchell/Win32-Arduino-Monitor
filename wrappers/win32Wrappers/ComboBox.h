/// @file ComboBox.h
/// @brief Wraps a Win32 COMBOBOX control (CBS_DROPDOWNLIST)

#pragma once
#include "UIElement.h"

class ComboBox : public UIElement {
  public:

    /// @function ComboBox
    /// @brief Creates a dropdown list 
    /// @param HWND_parent Constrol ID
    /// @param iId Control ID
    /// @param iX Left positin
    /// @param iY Top position
    /// @param iW Width in pixels
    /// @param iH Dropdown list height in pixels
    ComboBox(HWND hwnd_parent, int iId, int iX, int iY, int iW, int iH) {
      hwnd_self = CreateWindowEx(
        0, L"COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        iX, iY, iW, iH,
        hwnd_parent, (HMENU)(UINT_PTR)iId,
        NULL, NULL
      );
    }

    /// @function AddItem
    /// @brief Appends a string to the dropdown list
    /// @param szItem the display string to add
    /// @return void
    void AddItem(const wchar_t* szItem) {
      if(hwnd_self)
        SendMessage(hwnd_self, CB_ADDSTRING, 0, (LPARAM)szItem);
    }

    /// @function Clear
    /// @brief Removes all items from the list
    /// @return void
    void Clear() {
      if(hwnd_self)
        SendMessage(hwnd_self,CB_RESETCONTENT, 0, 0);
    }

    /// @function GetSelected
    /// @brief Copies the currently selected item's text into a buffer
    /// @param arrBuf Destination buffer
    /// @param iBufLen Buffer size in characters.
    /// @return void
    void GetSelected(wchar_t* arrBuf, int iBufLen) const {
      if(hwnd_self) 
        GetWindowText(hwnd_self, arrBuf, iBufLen);
    }

    /// @function SelectFirst
    /// @brief Selects the first item in the list if any exist
    /// @return void
    void SelectFirst() {
      if(hwnd_self && SendMessage(hwnd_self, CB_GETCOUNT, 0, 0) > 0) 
        SendMessage(hwnd_self, CB_SETCURSEL, 0, 0);
    }

    /// @function GetCount
    /// @brief Returns the number of Items in the list.
    /// @return Item count
    int GetCount() const {
      if(!hwnd_self) return 0;
      return (int)SendMessage(hwnd_self, CB_GETCOUNT, 0, 0);
    }
};