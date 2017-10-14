//
//  Browse.C
//
//  Copyright (C) Microsoft, 1994,1995 All Rights Reserved.
//
//  History:
//  ral 5/23/94 - First pass
//  3/20/95  [stevecat] - NT port & real clean up, unicode, etc.
//
//
#include "appwiz.h"
#ifdef WINNT
#include <uastrfnc.h>
#endif

// Copied from shelldll\ole2dup.h
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

//
//  Initialize the browse property sheet.  Limit the size of the edit control.
//

void BrowseInitPropSheet(HWND hDlg, LPARAM lParam)
{
    LPWIZDATA lpwd = InitWizSheet(hDlg, lParam, 0);

    Edit_LimitText(GetDlgItem(hDlg, IDC_COMMAND), ARRAYSIZE(lpwd->szExeName)-1);
}


//
//  Sets the appropriate wizard buttons.  If there's any text in the
//  edit control then Next is enabled.  Otherwise, Next and Back are both
//  grey.
//
void SetBrowseButtons(LPWIZDATA lpwd)
{
    BOOL fIsText = GetWindowTextLength(GetDlgItem(lpwd->hwnd, IDC_COMMAND)) > 0;
    BOOL fIsSetup = (lpwd->dwFlags & WDFLAG_SETUPWIZ);
    int iBtns = fIsSetup ? PSWIZB_BACK : 0;

    if (fIsSetup)
    {
        iBtns |= fIsText ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH;
    }
    else
    {
        if (fIsText)
        {
            iBtns |= PSWIZB_NEXT;
        }
    }
    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), iBtns);
}


//
//  NOTES: 1) This function assumes that lpwd->hwnd has already been set to
//           the dialogs hwnd.  2) This function is called from NextPushed
//           if the application specified can not be found.
//
//  BrowseSetActive enables the next button and sets the focus to the edit
//  control by posting a POKEFOCUS message.
//

void BrowseSetActive(LPWIZDATA lpwd)
{
    //
    // NOTE: We re-use the szProgDesc string since it will always be reset
    //       when this page is activated.  Use it to construct a command line.
    //

    #define   szCmdLine lpwd->szProgDesc

    lstrcpy(szCmdLine, lpwd->szExeName);

    PathQuoteSpaces(szCmdLine);

    if (lpwd->szParams[0] != 0)
    {
        lstrcat(szCmdLine, TEXT(" "));
        lstrcat(szCmdLine, lpwd->szParams);
    }

    Edit_SetText(GetDlgItem(lpwd->hwnd, IDC_COMMAND), szCmdLine);

    if (lpwd->dwFlags & WDFLAG_SETUPWIZ)
    {
        int   iHeader = szCmdLine[0] != 0 ? IDS_HAVESETUPPRG : IDS_NOSETUPPRG;
        TCHAR szInstruct[MAX_PATH];

        LoadString(hInstance, iHeader, szInstruct, ARRAYSIZE(szInstruct));

        Static_SetText(GetDlgItem(lpwd->hwnd, IDC_SETUPMSG), szInstruct);
    }

    SetBrowseButtons(lpwd);

    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);

    szCmdLine[0] = 0;            // Reset progdesc to empty string
    #undef szCmdLine
}


//
//  Returns TRUE if able to get properties for szExeName from PifMgr.  The
//  program properties will be read into lpwd->PropPrg.
//

BOOL ReadPifProps(LPWIZDATA lpwd)
{
    int hPifProp;
    LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

    hPifProp = PifMgr_OpenProperties(lpszName, NULL, 0, OPENPROPS_INHIBITPIF);
    WIZERRORIF((!hPifProp), TEXT("Unable to open properties for DOS exe."))

    if (hPifProp == 0)
        return(FALSE);

    PifMgr_GetProperties(hPifProp, (LPSTR)GROUP_PRG, &(lpwd->PropPrg),
                         sizeof(lpwd->PropPrg), GETPROPS_NONE);

    PifMgr_CloseProperties(hPifProp, CLOSEPROPS_DISCARD);

    return(TRUE);
}


//
//  Returns TRUE if lpwd->szExeName points to a valid exe type.  It also sets
//  the appropriate flags, such as APPKNOWN and DOSAPP in the wizdata structure
//  if the exe is valid.
//

void DetermineExeType(LPWIZDATA lpwd)
{

    DWORD   dwExeType;
    LPTSTR  lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

    lpwd->dwFlags &= ~(WDFLAG_APPKNOWN | WDFLAG_DOSAPP | WDFLAG_SINGLEAPP);

    dwExeType = SHGetFileInfo(lpszName, 0, NULL, 0, SHGFI_EXETYPE);

    if (LOWORD(dwExeType) != ('M' | ('Z' << 8)))
    {
        lpwd->dwFlags |= WDFLAG_APPKNOWN;

        if (lstrcmpi(PathFindExtension(lpszName), c_szPIF) == 0)
        {
            lpwd->dwFlags |= WDFLAG_DOSAPP;
        }
    }
    else
    {
        lpwd->dwFlags |= WDFLAG_DOSAPP;

        if (ReadPifProps(lpwd))
        {
            if ((lpwd->PropPrg.flPrgInit & PRGINIT_INFSETTINGS) ||
                ((lpwd->PropPrg.flPrgInit &
                     (PRGINIT_NOPIF | PRGINIT_DEFAULTPIF)) == 0))
            {
                lpwd->dwFlags |= WDFLAG_APPKNOWN;

                if (lpwd->PropPrg.flPrgInit & PRGINIT_REALMODE)
                {
                    lpwd->dwFlags |= WDFLAG_SINGLEAPP;
                }
            }
        }
    }
}


//
//  Removes the filename extension (if any) from the string.
//

void StripExt(LPTSTR lpsz)
{
    LPTSTR pExt = PathFindExtension(lpsz);

    if (*pExt)
        *pExt = 0;    // null out the "."
}



//
//  Sets the working directory as appropriate for the file type.
//

void FindWorkingDir(LPWIZDATA lpwd)
{
    LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;
#ifdef WINNT
    TCHAR szWindir[ MAX_PATH ];
    DWORD dwLen;
#endif

    if (PathIsUNC(lpszName) || PathIsDirectory(lpszName))
    {
        lpwd->szWorkingDir[0] = 0;
    }
    else
    {
        lstrcpy(lpwd->szWorkingDir, lpszName);
        PathRemoveFileSpec(lpwd->szWorkingDir);
    }

#ifdef WINNT
    //
    // Okay, at this point we should have the absolute path for the
    // working directory of the link.  On NT, if the working dir happens to be for
    // something in the %Windir% directory (or a subdir of %windir%),
    // then store the path as %windir%\blah\blah\blah instead of as an
    // absolute path.  This will help with interoperability of shortcuts
    // across different machines, etc.  But only do this for shortcuts that
    // are already marked as having expandable env strings...
    //

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
    {
        dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                          szWindir,
                                          ARRAYSIZE(szWindir)
                                         );
        if (dwLen &&
            dwLen < ARRAYSIZE(szWindir) &&
            lstrlen(szWindir) <= lstrlen(lpwd->szWorkingDir)
           )
        {
            //
            // we use dwLen-1 because dwLen includes the '\0' character
            //
            if (CompareString( LOCALE_SYSTEM_DEFAULT,
                               NORM_IGNORECASE,
                               szWindir, dwLen-1 ,
                               lpwd->szWorkingDir, dwLen-1
                              ) == 2)
            {
                TCHAR szWorkingDir[ MAX_PATH ];
                //
                // We should substitute the env variable for the
                // actual string here...
                //
                ualstrcpy( szWorkingDir, lpwd->szWorkingDir );
                ualstrcpy( lpwd->szWorkingDir, TEXT("%windir%") );

                // 8 == lstrlen("%windir%")
                ualstrcpy( lpwd->szWorkingDir + 12, szWorkingDir+dwLen-1 );

            }
        }
    }
#endif // winnt
}


#ifndef NO_NEW_SHORTCUT_HOOK

//
// Returns:
//    Hook result or error.
//
// S_OK:
//    *pnshhk is the INewShortcutHook of the object to use to save the new Shortcut.
//    szProgDesc[] and szExt[] are filled in.
//    szExeName[] may be translated.
// otherwise:
//    *pnshhk is NULL.
//    szProgDesc[] and szExt[] are empty strings.
//

HRESULT QueryNewLinkHandler(LPWIZDATA lpwd, LPCLSID pclsidHook)
{
   HRESULT   hr;
   IUnknown *punk;
   LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

   lpwd->pnshhk = NULL;
#ifdef UNICODE
   lpwd->pnshhkA = NULL;
#endif

   *(lpwd->szProgDesc) = TEXT('\0');
   *(lpwd->szExt) = TEXT('\0');

   hr = CoCreateInstance(pclsidHook, NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, &punk);

   if (hr == S_OK)
   {
      INewShortcutHook *pnshhk;

      hr = punk->lpVtbl->QueryInterface(punk, &IID_INewShortcutHook, &pnshhk);

      if (hr == S_OK)
      {
         hr = pnshhk->lpVtbl->SetReferent(pnshhk, lpszName, lpwd->hwnd);

         if (hr == S_OK)
         {
            hr = pnshhk->lpVtbl->SetFolder(pnshhk, lpwd->lpszFolder);

            if (hr == S_OK)
            {
               hr = pnshhk->lpVtbl->GetName(pnshhk, lpwd->szProgDesc,
                                            ARRAYSIZE(lpwd->szProgDesc));

               if (hr == S_OK)
               {
                  hr = pnshhk->lpVtbl->GetExtension(pnshhk, lpwd->szExt,
                                                    ARRAYSIZE(lpwd->szExt));

                  if (hr == S_OK)
                     hr = pnshhk->lpVtbl->GetReferent(pnshhk, lpszName,
                                                      ARRAYSIZE(lpwd->szExeName));
               }
            }
         }

         if (hr == S_OK)
            lpwd->pnshhk = pnshhk;
         else
            pnshhk->lpVtbl->Release(pnshhk);
      }
#ifdef UNICODE
      else
      {
          INewShortcutHookA *pnshhkA;
          hr = punk->lpVtbl->QueryInterface(punk, &IID_INewShortcutHookA, &pnshhkA);

          if (hr == S_OK)
          {
             UINT   cFolderA = WideCharToMultiByte(CP_ACP,0,lpwd->lpszFolder,-1,NULL,0,0,0)+1;
             LPSTR  lpszFolderA = (LPSTR)LocalAlloc(LPTR,cFolderA*SIZEOF(CHAR));
             CHAR   szNameA[MAX_PATH];
             CHAR   szProgDescA[MAX_PATH];
             CHAR   szExtA[MAX_PATH];

             WideCharToMultiByte(CP_ACP, 0,
                                 lpszName, -1,
                                 szNameA, ARRAYSIZE(szNameA),
                                 0, 0);

             WideCharToMultiByte(CP_ACP, 0,
                                 lpwd->lpszFolder, -1,
                                 lpszFolderA, cFolderA,
                                 0, 0);


             if (lpszFolderA != NULL)
             {
                 hr = pnshhkA->lpVtbl->SetReferent(pnshhkA, szNameA, lpwd->hwnd);

                 if (hr == S_OK)
                 {
                    hr = pnshhkA->lpVtbl->SetFolder(pnshhkA, lpszFolderA);

                    if (hr == S_OK)
                    {
                       hr = pnshhkA->lpVtbl->GetName(pnshhkA, szProgDescA,
                                                    ARRAYSIZE(szProgDescA));

                       if (hr == S_OK)
                       {
                          MultiByteToWideChar(CP_ACP, 0,
                                              szProgDescA, -1,
                                              lpwd->szProgDesc, ARRAYSIZE(lpwd->szProgDesc));

                          hr = pnshhkA->lpVtbl->GetExtension(pnshhkA, szExtA,
                                                            ARRAYSIZE(szExtA));

                          if (hr == S_OK)
                          {
                             MultiByteToWideChar(CP_ACP, 0,
                                                 szExtA, -1,
                                                 lpwd->szExt, ARRAYSIZE(lpwd->szExt));

                             hr = pnshhkA->lpVtbl->GetReferent(pnshhkA, szNameA,
                                                              ARRAYSIZE(szNameA));

                             MultiByteToWideChar(CP_ACP, 0,
                                                 szExtA, -1,
                                                 lpszName, ARRAYSIZE(lpwd->szExeName));
                          }
                       }
                    }
                 }

                 if (hr == S_OK)
                    lpwd->pnshhkA = pnshhkA;
                 else
                    pnshhkA->lpVtbl->Release(pnshhkA);

                 if (lpszFolderA)
                    LocalFree(lpszFolderA);
             }
             else
                hr = E_OUTOFMEMORY;
          }

      }
#endif
      punk->lpVtbl->Release(punk);
   }

   return(hr);
}


const TCHAR c_szNewLinkHandlers[] = REGSTR_PATH_EXPLORER TEXT("\\NewShortcutHandlers");


//
// Sets lpwd->pnshhk to NULL for CLSID_ShellLink (default) or to the
// INewShortcutHook of the object to be used.
//
// If lpwd->pnshhk is returned non-NULL, szProgDesc[] and szExt[] are also
// filled in.
//

void DetermineLinkHandler(LPWIZDATA lpwd)
{
   HKEY hkeyHooks;

   // Lose any previously saved external new Shortcut handler.

   if (lpwd->pnshhk)
   {
      lpwd->pnshhk->lpVtbl->Release(lpwd->pnshhk);
      lpwd->pnshhk = NULL;
   }
#ifdef UNICODE
   if (lpwd->pnshhkA)
   {
      lpwd->pnshhkA->lpVtbl->Release(lpwd->pnshhkA);
      lpwd->pnshhkA = NULL;
   }
#endif

   //
   // Enumerate the list of new link handlers.  Each new link handler is
   // registered as a GUID value under c_szNewLinkHandlers.
   //

   if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szNewLinkHandlers, &hkeyHooks)
       == ERROR_SUCCESS)
   {
      DWORD dwiValue;
      TCHAR szCLSID[GUIDSTR_MAX];
      DWORD dwcbCLSIDLen;

      //
      // Invoke each hook.  A hook returns S_FALSE if it does not wish to
      // handle the new link.  Stop if a hook returns S_OK.
      //

      for (dwcbCLSIDLen = ARRAYSIZE(szCLSID), dwiValue = 0;
           RegEnumValue(hkeyHooks, dwiValue, szCLSID, &dwcbCLSIDLen, NULL,
                        NULL, NULL, NULL) == ERROR_SUCCESS;
           dwcbCLSIDLen = ARRAYSIZE(szCLSID), dwiValue++)
      {
         CLSID clsidHook;

         if (SHCLSIDFromString(szCLSID, &clsidHook) == S_OK &&
             QueryNewLinkHandler(lpwd, &clsidHook) == S_OK)
            break;
      }

      RegCloseKey(hkeyHooks);
   }

   return;
}

#endif


void _inline PathRemoveArgs(LPTSTR pszPath)
{
    LPTSTR pArgs = PathGetArgs(pszPath);

    if (*pArgs)
    {
        *(pArgs - 1) = TEXT('\0');   // clobber the ' '
    }
    else
    {
        //
        // Handle trailing space.
        //
        pArgs = CharPrev(pszPath, pArgs);

        if (*pArgs == TEXT(' '))
            *pArgs = TEXT('\0');
    }
}


//
//  Returns TRUE if it's OK to go to the next wizard dialog.
//

BOOL NextPushed(LPWIZDATA lpwd)
{

    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));
    PathRemoveBlanks(lpwd->szExeName);

    if (lpwd->szExeName[0] != 0)
    {
        BOOL    bUNC;
        LPTSTR  lpszTarget = NULL;
        HCURSOR hcurOld  = SetCursor(LoadCursor(NULL, IDC_WAIT));
        LPTSTR  lpszArgs = PathGetArgs(lpwd->szExeName);

        lstrcpy(lpwd->szParams, lpszArgs);

        if (*lpszArgs)
        {
            *(lpszArgs - 1) = 0;   // clobber the ' ' in the exe name field
        }

        ExpandEnvironmentStrings( lpwd->szExeName,
                                  lpwd->szExpExeName,
                                  ARRAYSIZE(lpwd->szExpExeName)
                                 );
        lpwd->szExpExeName[ MAX_PATH-1 ] = TEXT('\0');
        if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
            lpwd->dwFlags |= WDFLAG_EXPSZ;


        lpszTarget = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;


        PathUnquoteSpaces(lpszTarget);
        if (lpwd->dwFlags & WDFLAG_EXPSZ)
            PathUnquoteSpaces(lpwd->szExeName);

        lpwd->dwFlags &= ~WDFLAG_COPYLINK;

#ifndef NO_NEW_SHORTCUT_HOOK

        //
        // Figure out who wants to handle this string as a link referent.
        //

        DetermineLinkHandler(lpwd);

        if (lpwd->pnshhk)
        {
            //
            // We are using an external link handler.  Skip file system
            // validation.
            //

            lpwd->dwFlags |= WDFLAG_APPKNOWN;
            SetCursor(hcurOld);
            return(TRUE);
        }
#ifdef UNICODE
        if (lpwd->pnshhkA)
        {
            //
            // We are using an external link handler.  Skip file system
            // validation.
            //

            lpwd->dwFlags |= WDFLAG_APPKNOWN;
            SetCursor(hcurOld);
            return(TRUE);
        }
#endif

#endif

        bUNC = PathIsUNC(lpszTarget);

        if (bUNC && !SHValidateUNC(lpwd->hwnd, lpszTarget, FALSE))
            goto Done;

        //
        //  If the user tries to make a link to A:\ and there's no disk
        //  in the drive, PathResolve would fail.  So, for drive roots, we
        //  don't try to resolve it.
        //

        if ((PathIsRoot(lpszTarget) && !bUNC &&
             DriveType(DRIVEID(lpszTarget))) ||
             PathResolve(lpszTarget, NULL,
                         PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
        {
            //
            // If we found a PIF file then we'll try to convert it to the
            // name of the file it points to.
            //

            if (lstrcmpi(PathFindExtension(lpszTarget), c_szPIF) == 0)
            {
                if (!ReadPifProps(lpwd))
                {
                    goto Done;
                }

#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0, lpwd->PropPrg.achCmdLine, -1,
                                    lpszTarget, ARRAYSIZE(lpwd->szExeName));
#else
                lstrcpy(lpszTarget, lpwd->PropPrg.achCmdLine);
#endif // UNICODE

                PathRemoveArgs(lpszTarget);

                if (!PathResolve(lpszTarget, NULL,
                                 PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
                {
                    goto Done;
                }
            }

#ifdef WINNT
            //
            // Okay, at this point we should have the absolute path for the
            // target of the link.  On NT, if the target happens to be for
            // something in the %Windir% directory (or a subdir of %Windir%),
            // AND the user didn't type in an expandable path already, then
            // store the path as %windir%\blah\blah\blah instead of as an
            // absolute path.  This will help with interoperability of shortcuts
            // across different machines, etc.
            //

            if (!(lpwd->dwFlags & WDFLAG_EXPSZ))
            {
                TCHAR szWindir[ MAX_PATH ];
                DWORD dwLen;

                //
                // What did the user type in?
                //
                GetDlgItemText(lpwd->hwnd, IDC_COMMAND, szWindir, ARRAYSIZE(szWindir));
                if (ualstrcmpi(szWindir, lpwd->szExeName)==0)
                {
                    //
                    // If we didn't change it, it means the user typed in an
                    // exact path.  In that case, don't try to map anyting.
                    //
                    goto LinkToALinkCase;
                }
                dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                                  szWindir,
                                                  ARRAYSIZE(szWindir)
                                                 );
                if (dwLen &&
                    dwLen < ARRAYSIZE(szWindir) &&
                    lstrlen(szWindir) <= lstrlen(lpszTarget)
                   )
                {
                    //
                    // we use dwLen-1 because dwLen includes the '\0' character
                    //
                    if (CompareString( LOCALE_SYSTEM_DEFAULT,
                                       NORM_IGNORECASE,
                                       szWindir, dwLen-1 ,
                                       lpszTarget, dwLen-1
                                      ) == 2)
                    {
                        //
                        // We should substitute the env variable for the
                        // actual string here...
                        //
                        lstrcpy( lpwd->szExpExeName, lpwd->szExeName );
                        lstrcpy( lpwd->szExeName, TEXT("%windir%") );

                        // 8 == lstrlen("%windir%")
                        ualstrcpy( lpwd->szExeName + 8, lpwd->szExpExeName+dwLen-1 );
                        lpwd->dwFlags |= WDFLAG_EXPSZ;
                        lpszTarget = lpwd->szExpExeName;

                    }
                }
            }
#endif // winnt

#ifdef WINNT
            //
            // Okay, at this point we should have the absolute path for the
            // target of the link.  On NT, if the target happens to be for
            // something in the %Windir% directory (or a subdir of %Windir%),
            // AND the user didn't type in an expandable path already, then
            // store the path as %windir%\blah\blah\blah instead of as an
            // absolute path.  This will help with interoperability of shortcuts
            // across different machines, etc.
            //

            if (!(lpwd->dwFlags & WDFLAG_EXPSZ))
            {
                TCHAR szWindir[ MAX_PATH ];
                DWORD dwLen;

                //
                // What did the user type in?
                //
                GetDlgItemText(lpwd->hwnd, IDC_COMMAND, szWindir, ARRAYSIZE(szWindir));
                if (ualstrcmpi(szWindir, lpwd->szExeName)==0)
                {
                    //
                    // If we didn't change it, it means the user typed in an
                    // exact path.  In that case, don't try to map anyting.
                    //
                    goto LinkToALinkCase;
                }
                dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                                  szWindir,
                                                  ARRAYSIZE(szWindir)
                                                 );
                if (dwLen &&
                    dwLen < ARRAYSIZE(szWindir) &&
                    lstrlen(szWindir) <= lstrlen(lpszTarget)
                   )
                {
                    //
                    // we use dwLen-1 because dwLen includes the '\0' character
                    //
                    if (CompareString( LOCALE_SYSTEM_DEFAULT,
                                       NORM_IGNORECASE,
                                       szWindir, dwLen-1 ,
                                       lpszTarget, dwLen-1
                                      ) == 2)
                    {
                        //
                        // We should substitute the env variable for the
                        // actual string here...
                        //
                        lstrcpy( lpwd->szExpExeName, lpwd->szExeName );
                        lstrcpy( lpwd->szExeName, TEXT("%windir%") );

                        // 8 == lstrlen("%windir%")
                        ualstrcpy( lpwd->szExeName + 8, lpwd->szExpExeName+dwLen-1 );
                        lpwd->dwFlags |= WDFLAG_EXPSZ;
                        lpszTarget = lpwd->szExpExeName;

                    }
                }
            }
#endif // winnt

LinkToALinkCase:
            //
            //  Really, really obscure case. The user creates "New Shortcut" and
            //  tries to point it to itself. Don't allow it.  We'd be confused
            //  later. Since it's so obscure, just give a generic error about
            //  "Can't find this file"
            //

            if (!(lpwd->lpszOriginalName &&
                  lstrcmpi(lpwd->lpszOriginalName, lpszTarget) == 0))
            {
                DetermineExeType(lpwd);
                FindWorkingDir(lpwd);

                lpwd->szProgDesc[0] = 0;  // Reset description
                                          // EVEN IF WE DON'T RECREATE IT HERE!

                if (lpwd->lpszFolder && lpwd->lpszFolder[0] != 0 &&
                    !DetermineDefaultTitle(lpwd))
                {
                    goto Done;
                }

                if (lpwd->dwFlags & WDFLAG_EXPSZ)
                {
                    LPTSTR lpszExt = PathFindExtension( lpwd->szExeName );

                    if (!(*lpszExt))
                    {
                        // do simple check to make sure there was a file name
                        // at the end of the original entry.  we assume that
                        // if we got this far, lpszExt points to the end of
                        // the string pointed to by lpwd->szExeName, and that
                        // lpwd->szExeName has at least one character in it.
                        if (lpwd->szExeName &&
                            (*lpwd->szExeName) &&
                            (*(lpszExt-1)!=TEXT('%'))
                            )
                        {
                            lstrcpy( lpszExt, PathFindExtension( lpszTarget ) );
                        }
                    }
                }

                SetCursor(hcurOld);
                return(TRUE);
            }

        }
Done:

        SetCursor(hcurOld);
        ShellMessageBox(hInstance, lpwd->hwnd, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, lpwd->szExeName);
    }

    BrowseSetActive(lpwd);
    return(FALSE);
}


//
//  Returns TRUE if it's OK to run the setup program.
//

BOOL SetupFinishPushed(LPWIZDATA lpwd)
{
    BOOL fValidPrg = FALSE;

    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));
    PathRemoveBlanks(lpwd->szExeName);

    if (lpwd->szExeName[0] != 0)
    {
        LPTSTR lpszTarget = NULL;
        LPTSTR lpszArgs = NULL;
        HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));


        ExpandEnvironmentStrings( lpwd->szExeName,
                                  lpwd->szExpExeName,
                                  ARRAYSIZE(lpwd->szExpExeName)
                                 );
        if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
            lpwd->dwFlags |= WDFLAG_EXPSZ;

        lpszTarget = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

        lpszArgs = PathGetArgs(lpszTarget);
        lstrcpy(lpwd->szParams, lpszArgs);

        if (*lpszArgs)
        {
            *(lpszArgs - 1) = 0;   // clobber the ' ' in the exe name field
        }


        PathUnquoteSpaces(lpszTarget);
        if (lpwd->dwFlags & WDFLAG_EXPSZ)
            PathUnquoteSpaces(lpwd->szExeName);

        if (PathResolve(lpszTarget, NULL,
                        PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
        {
            fValidPrg = TRUE;
            FindWorkingDir(lpwd);
            if (lpwd->dwFlags & WDFLAG_EXPSZ)
            {
                LPTSTR lpszExt = PathFindExtension( lpwd->szExeName );

                if (!(*lpszExt))
                {
                    lstrcpy( lpszExt, PathFindExtension( lpszTarget ) );
                }
            }
        }
        SetCursor(hcurOld);
    }


    if (!fValidPrg)
    {
        ShellMessageBox(hInstance, lpwd->hwnd, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, lpwd->szExeName);
        BrowseSetActive(lpwd);
    }
    return(fValidPrg);
}


BOOL DetermineDefaultTitle(LPWIZDATA lpwd)
{
    TCHAR   szFullName[MAX_PATH];
    BOOL    fCopy;
    LPTSTR  lpszName;

    lpwd->dwFlags &= ~WDFLAG_COPYLINK;

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
        lpszName = lpwd->szExpExeName;
    else
        lpszName = lpwd->szExeName;

    if (!SHGetNewLinkInfo(lpszName, lpwd->lpszFolder, szFullName,
                     &fCopy, 0))
    {
        //
        // failure...
        //

        return(FALSE);
    }

    lpszName = PathFindFileName(szFullName);

    StripExt(lpszName);

    lstrcpyn(lpwd->szProgDesc, lpszName, ARRAYSIZE(lpwd->szProgDesc));

    //
    // We will never copy PIF files since they often do not contain
    // the appropriate current directory.  This is becuase they are
    // automatically created when you run a DOS application from the
    // shell.
    //

    if ((lpwd->dwFlags & WDFLAG_DOSAPP) == 0)
    {
        if (fCopy)
        {
            lpwd->dwFlags |= WDFLAG_COPYLINK;
        }
#ifndef NO_NEW_SHORTCUT_HOOK
        lstrcpy(lpwd->szExt, c_szLNK);
    }
    else
    {
        lstrcpy(lpwd->szExt, c_szPIF);
#endif
    }

    return(TRUE);
}


//
//  Call the common dialog code for File Open
//
BOOL _inline BrowseForExe(LPWIZDATA lpwd, LPCTSTR lpszStartDir)
{
    TCHAR szExt[80];
    TCHAR szFilter[80];
    TCHAR szTitle[80];
    LPTSTR lpszName;

    LoadAndStrip(IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter));
    LoadString(hInstance, IDS_BROWSEEXT,    szExt,    ARRAYSIZE(szExt));
    LoadString(hInstance, IDS_BROWSETITLE,  szTitle,  ARRAYSIZE(szTitle));

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
        lpszName = lpwd->szExpExeName;
    else
        lpszName = lpwd->szExeName;

    return(GetFileNameFromBrowse(lpwd->hwnd,
                                 lpszName, ARRAYSIZE(lpwd->szExeName),
                                 lpszStartDir, szExt, szFilter, szTitle));
}


//
//  Use the common open dialog to browse for program.
//

void BrowsePushed(LPWIZDATA lpwd)
{
    TCHAR szBootDir[64];

    //
    // Must pass the buffer size to GetBootDir because that is what
    // the RegQueryValueEx function expects - not count of chars.
    //

    GetBootDir(szBootDir, sizeof(szBootDir));

    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));
    ExpandEnvironmentStrings( lpwd->szExeName, lpwd->szExpExeName, ARRAYSIZE(lpwd->szExpExeName) );
    if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
        lpwd->dwFlags |= WDFLAG_EXPSZ;

    if (BrowseForExe(lpwd, szBootDir))
    {
        lpwd->szParams[0] = 0;
        BrowseSetActive(lpwd);
    }
}


//
//  Main dialog procedure for first page of shortcut wizard.
//

BOOL CALLBACK BrowseDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong(hDlg, DWL_USER));
    LPWIZDATA lpwd;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
               case PSN_SETACTIVE:
                    lpwd->hwnd = hDlg;
                    if (lpwd->dwFlags & WDFLAG_NOBROWSEPAGE)
                    {
                        SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                    }
                    else
                    {
                        BrowseSetActive(lpwd);
                    }
                    break;

               case PSN_WIZNEXT:
                    if (!NextPushed(lpwd))
                    {
                        SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                    }
                    break;

               case PSN_WIZFINISH:
                  {
                    BOOL ForceWx86;

#ifdef WX86
                    ForceWx86 = bWx86Enabled && bForceX86Env;
#else
                    ForceWx86 = FALSE;
#endif

                    if (!SetupFinishPushed(lpwd) ||
                        !ExecSetupProg(lpwd, ForceWx86))
                    {
                        BrowseSetActive(lpwd);
                        SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                    }
                    break;
                 }

               case PSN_RESET:
                    CleanUpWizData(lpwd);
                    break;

               default:
                  return FALSE;
            }
            break;

        case WM_INITDIALOG:
            BrowseInitPropSheet(hDlg, lParam);
            break;

        case WMPRIV_POKEFOCUS:
            {
            HWND hCmd = GetDlgItem(hDlg, IDC_COMMAND);

            SetFocus(hCmd);

            Edit_SetSel(hCmd, 0, -1);

            break;
            }

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                        break;

                case IDC_COMMAND:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case EN_CHANGE:
                            SetBrowseButtons(lpwd);
                            break;
                    }
                    break;

                case IDC_BROWSE:
                    BrowsePushed(lpwd);
                    break;

            } // end of switch on WM_COMMAND
            break;

        default:
            return FALSE;

    } // end of switch on message

    return TRUE;
}  // BrowseDlgProc
