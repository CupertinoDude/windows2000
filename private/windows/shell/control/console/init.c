/*************************************************************\
 *  File name:    INIT.C
 *
 *  Description:  Initialization code for Console control panel
 *                applet
 *
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1992-1994
 *  All rights reserved
 *
\*************************************************************/
#include "precomp.h"
#include <cpl.h>


HINSTANCE ghInstance;


/*************************************************************\
 *
 *  DllInitialize()
 *
 *  Purpose:    Main entry point
 *
 *
 *  Parameters: HINSTANCE hInstDLL    - Instance handle of DLL
 *              DWORD     dwReason    - Reason DLL was called
 *              LPVOID    lpvReserved - NULL
 *
 *
 *  Return:     BOOL
 *
\*************************************************************/

BOOL DllInitialize(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    ghInstance = hInstDLL;

    DisableThreadLibraryCalls(hInstDLL);

    return TRUE;
}


/*************************************************************\
 *
 *  CPlApplet()
 *
 *  Purpose:    Control Panel entry point
 *
 *
 *  Parameters: HWND hwnd      - Window handle
 *              WORD wMsg      - Control Panel message
 *              LPARAM lParam1 - Long parameter
 *              LPARAM lParam2 - Long parameter
 *
 *
 *  Return:     LONG
 *
\*************************************************************/

LONG CPlApplet( HWND hwnd, WORD wMsg, LPARAM lParam1, LPARAM lParam2)
{
    LPCPLINFO lpOldCPlInfo;
    LPNEWCPLINFO lpCPlInfo;
    INITCOMMONCONTROLSEX iccex;

    switch (wMsg) {

    case CPL_INIT:

        iccex.dwSize = sizeof(iccex);
        iccex.dwICC  = ICC_WIN95_CLASSES;
        InitCommonControlsEx( &iccex );
        //InitCommonControls();

        if (!RegisterClasses(ghInstance)) {
            return FALSE;
        }
        OEMCP = GetOEMCP();
        gfFESystem = IsFarEastCP(OEMCP);
        if (!NT_SUCCESS(InitializeDbcsMisc())) {
            return FALSE;
        }
        return TRUE;

    case CPL_GETCOUNT:
        return 1;

    case CPL_INQUIRE:

        lpOldCPlInfo = (LPCPLINFO)lParam2;

        lpOldCPlInfo->idIcon = IDI_CONSOLE;
        lpOldCPlInfo->idName = IDS_NAME;
        lpOldCPlInfo->idInfo = IDS_INFO;
        lpOldCPlInfo->lData  = 0;
        return TRUE;

    case CPL_NEWINQUIRE:

        lpCPlInfo = (LPNEWCPLINFO)lParam2;

        lpCPlInfo->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_CONSOLE));

        if (!LoadString(ghInstance, IDS_NAME, lpCPlInfo->szName,
                        NELEM(lpCPlInfo->szName))) {
            lpCPlInfo->szName[0] = TEXT('\0');
        }

        if (!LoadString(ghInstance, IDS_INFO, lpCPlInfo->szInfo,
                        NELEM(lpCPlInfo->szInfo))) {
            lpCPlInfo->szInfo[0] = TEXT('\0');
        }

        lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
        lpCPlInfo->dwHelpContext = 0;
        lpCPlInfo->szHelpFile[0] = TEXT('\0');

        return (LONG)TRUE;

    case CPL_DBLCLK:
        ConsolePropertySheet(hwnd);
        break;

    case CPL_EXIT:
        DestroyDbcsMisc();
        UnregisterClasses(ghInstance);
        break;
    }
    return (LONG)0;
}
