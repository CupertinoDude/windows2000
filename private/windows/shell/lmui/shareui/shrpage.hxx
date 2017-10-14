//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995 - 1995.
//
//  File:       shrpage.hxx
//
//  Contents:   "Sharing" shell property page extension
//
//  History:    6-Apr-95        BruceFo     Created
//
//--------------------------------------------------------------------------

#ifndef __SHRPAGE_HXX__
#define __SHRPAGE_HXX__

class CShareInfo;

class CSharingPropertyPage
{
    DECLARE_SIG;

public:

    //
    // Main page dialog procedure: static
    //

    static
    INT_PTR CALLBACK
    DlgProcPage(
        HWND hWnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
        );

    static
    LRESULT CALLBACK
    SizeWndProc(
        IN HWND hwnd,
        IN UINT wMsg,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    //
    // constructor, destructor, 2nd phase constructor
    //

    CSharingPropertyPage(
        IN HWND hwndPage,
        IN PWSTR pszMachine,
        IN PWSTR pszShare
        );

    ~CSharingPropertyPage();

    HRESULT
    InitInstance(
        VOID
        );

private:

    //
    // Main page dialog procedure: non-static
    //

    INT_PTR
    _PageProc(
        IN HWND hWnd,
        IN UINT msg,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    //
    // Window messages and notifications
    //

    BOOL
    _OnInitDialog(
        IN HWND hwnd,
        IN HWND hwndFocus,
        IN LPARAM lInitParam
        );

    BOOL
    _OnCommand(
        IN HWND hwnd,
        IN WORD wNotifyCode,
        IN WORD wID,
        IN HWND hwndCtl
        );

    BOOL
    _OnPermissions(
        IN HWND hwnd
        );

    BOOL
    _OnNotify(
        IN HWND hwnd,
        IN int idCtrl,
        IN LPNMHDR phdr
        );

    BOOL
    _OnPropertySheetNotify(
        IN HWND hwnd,
        IN LPNMHDR phdr
        );

    BOOL
    _OnNcDestroy(
        IN HWND hwnd
        );

    //
    // Other helper methods
    //

    VOID
    _InitializeControls(
        IN HWND hwnd
        );

    VOID
    _SetControlsToDefaults(
        IN HWND hwnd
        );

    VOID
    _CacheMaxUses(
        IN HWND hwnd
        );

    VOID
    _SetControlsFromData(
        IN HWND hwnd
        );

    BOOL
    _ValidatePage(
        IN HWND hwnd
        );

    BOOL
    _DoApply(
        IN HWND hwnd
        );

    BOOL
    _DoCancel(
        IN HWND hwnd
        );

    VOID
    _MarkItemDirty(
        VOID
        );

    HWND
    _GetFrameWindow(
        VOID
        )
    {
        return GetParent(_hwndPage);
    }

#if DBG == 1
    VOID
    Dump(
        IN PWSTR pszCaption
        );
#endif // DBG == 1

    //
    // Private class variables
    //

    PWSTR               _pszMachine;
    PWSTR               _pszShare;
    HWND                _hwndPage;          // HWND to the property page
    BOOL                _fInitializingPage;

    BOOL                _bDirty;            // Dirty flag: anything changed?
    BOOL                _bShareNameChanged;
    BOOL                _bPathChanged;
    BOOL                _bCommentChanged;
    BOOL                _bUserLimitChanged;
    BOOL                _bSecDescChanged;

    CShareInfo*         _pCurInfo;
    PWSTR               _pszReplacePath;
    WORD                _wMaxUsers;

    WNDPROC _pfnAllowProc;
};

#endif  // __SHRPAGE_HXX__
