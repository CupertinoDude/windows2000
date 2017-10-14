/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    debugger.c

Abstract:

    This file implements the debugger.

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User Mode

--*/

#include "precomp.h"
#pragma hdrstop

#include "win32dm.h"


UINT                CbSendBuf;
LPBYTE              LpSendBuf;
BYTE                DmMsg[4096];
DWORD               nextHtid;
HTID                htidBpt;
HANDLE              hEventAttach;
HANDLE              hEventEntryPoint;
GOP                 gop = {0};




/**** SENDREQUEST - Send a request to the DM                            ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request to the DM.                                      *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *      LpDmMsg - global buffer filled in with returned data               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequest(
    DMF    dmf,
    HPID   hpid,
    HTID   htid
    )
{
    DBB     dbb;
    XOSD    xosd;

    dbb.dmf  = dmf;
    dbb.hpid = hpid;
    dbb.htid = htid;

    xosd = TLFunc( tlfRequest, hpid, sizeof ( DBB ), (LPARAM) &dbb );

    return xosd;
}

/**** SENDREQUESTX - Send a request with parameters to the DM           ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request and its parameter info to the DM.               *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *      wLen - number of bytes in lpv                                      *
 *      lpv - pointer to additional info needed by the DM; contents are    *
 *          dependent on the DMF                                           *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequestX(
    DMF  dmf,
    HPID hpid,
    HTID htid,
    UINT wLen,
    LPV  lpv
    )
{
    LPDBB   pdbb;
    XOSD    xosd;

    if (wLen + sizeof(DBB) > CbSendBuf) {
        if (LpSendBuf) {
            free(LpSendBuf);
        }
        CbSendBuf = sizeof(DBB) + wLen;
        LpSendBuf = (PUCHAR) malloc(CbSendBuf);
    }

    if (!LpSendBuf) {
        return xosdOutOfMemory;
    }

    pdbb = (LPDBB)LpSendBuf;

    pdbb->dmf  = dmf;
    pdbb->hpid = hpid;
    pdbb->htid = htid;
    _fmemcpy ( pdbb->rgbVar, lpv, wLen );

    xosd = TLFunc( tlfRequest, hpid, sizeof ( DBB ) + wLen, (LPARAM) pdbb );

    return xosd;
}

XOSD
PASCAL
TLCallBack (
    HPID hpid,
    UINT cb,
    LPV lpv
    )
/*++

Routine Description:

    Call the native execution model for the process hpid with the
    package sent from the transport layer.  This is how the DM
    sends things to its native EM.

Arguments:

    hpid - Supplies handle to the process

    cb   - Supplies size in bytes of the packet

    lpv  - Supplies the packet

Return Value:

    xosdNone - Success

    Any return value that can be generated by a native execution model.

--*/
{
    LPRTP  lprtp = (LPRTP) lpv;
    BYTE   b;
    XOSD   xosd;
    HTID   htid;


    DEBUG_OUT3( "tlcallback %x [hpid=(%p) htid=(%p)]\n", lprtp->dbc, hpid, lprtp->htid );

    switch(lprtp->dbc) {
        case dbcModLoad:
            b = 1;
            TLFunc( tlfReply, hpid, 1, (LPARAM) &b );
            break;

        case dbcCreateThread:
            htid = (HTID) ++nextHtid;
            TLFunc( tlfReply, hpid, sizeof(htid), (LPARAM) &htid );
            xosd = SendRequestX( dmfGo, hpid, htid, sizeof(gop), &gop );
            break;

        case dbcBpt:
            htidBpt = lprtp->htid;
            SetEvent( hEventAttach );
            break;

        case dbcLoadComplete:
            htidBpt = lprtp->htid;
            SetEvent( hEventAttach );
            break;

        case dbcEntryPoint:
            htidBpt = lprtp->htid;
            SetEvent( hEventEntryPoint );
            break;

        default:
            break;
     }

    return xosdNone;
}

BOOL
ConnectDebugger(
    CIndiv_TL_RM_WKSP * pCTl
    )
{
    extern AVS          Avs;
    DBGVERSIONPROC      pVerProc;
    LPAVS               pavs;
    CHAR                buf[256];
    DWORD               cb;

    
    Assert(pCTl);

    if ((hTransportDll = LoadLibrary( pCTl->m_pszDll )) == NULL) {
        return FALSE;
    }

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hTransportDll, DBGVERSIONPROCNAME);
    if (!pVerProc) {
        return FALSE;
    }

    pavs = (*pVerProc)();

    if (pavs->rgchType[0] != 'T' || pavs->rgchType[1] != 'L') {
        return FALSE;
    }

    if (Avs.rlvt != pavs->rlvt) {
        return FALSE;
    }

    if (Avs.iRmj != pavs->iRmj) {
        return FALSE;
    }

    if ((TLFunc = (TLFUNC)GetProcAddress(hTransportDll, "TLFunc")) == NULL) {
        return FALSE;
    }

    if (TLFunc( tlfInit, NULL, (WPARAM)(&Dbf), (LPARAM) TLCallBack ) != xosdNone) {
        return FALSE;
    }

    ///////////////////////////////////////////////////////////////////////////////

    cb = sizeof(buf);
    GetComputerName( buf, &cb );
    strcat( buf, " " );
    strcat( buf, pCTl->m_pszDll );

    if (TLFunc( tlfInit, NULL, NULL, (LPARAM) buf ) != xosdNone) {
        return FALSE;
    }

    if (TLFunc( tlfSetBuffer, NULL, sizeof(DmMsg), (LPARAM) &DmMsg ) != xosdNone) {
        return FALSE;
    }

    if (TLFunc( tlfConnect, NULL, 0, NULL ) != xosdNone) {
        return FALSE;
    }

    if (SendRequest( dmfInit, NULL, NULL ) != xosdNone) {
        return FALSE;
    }

    return TRUE;
}


BOOL
DisConnectDebugger(
    HPID hpid
    )
{
    if (TLFunc( tlfDisconnect, hpid, NULL, (LPARAM) htidBpt ) != xosdNone) {
        return FALSE;
    }

    return TRUE;
}


BOOL
AttachProcess(
    HPID    hpid,
    DWORD   dwProcessToAttach,
    HANDLE  hEventForAttach
    )
{
    DAP dap;
    BOOL rval = TRUE;

    hEventAttach  = CreateEvent( NULL, TRUE, FALSE, NULL );

    dap.dwProcessId = dwProcessToAttach;
    dap.hEventGo = hEventForAttach;

    if (SendRequest( dmfCreatePid, hpid, NULL ) != xosdNone) {
        rval = FALSE;
    }
    else if (SendRequestX(dmfDebugActive, hpid, NULL, sizeof(dap), &dap) != xosdNone) {
        rval = FALSE;
    }
    else {
        WaitForSingleObject( hEventAttach, INFINITE );
    }

    CloseHandle( hEventAttach );

    return rval;
}


// Helper function: repack dmfProgLoad/dmfSpawnOrphan arguments into the form:
//              DWORD dwChildFlags
//              TCHAR rgtchExe[]
//              TCHAR rgtchCmdLine[]
//              TCHAR rgtchDir[]
//
//      You probably want to free the lplpvPacket when you're done.
//
XOSD
RepackProgLoad (
    CONST LPPRL lpprl,
    LPVOID      *lplpvPacket,
    UINT        *pcb
    )
{
    BYTE*   lpb;
    LPTSTR  lszRemoteDir = _T("");

    assert (lpprl);
    assert (lplpvPacket);
    assert (pcb);

    // lszRemoteDir is allowed to be NULL, in which case we pass ""
    if (lpprl -> lszRemoteDir != NULL) {
        lszRemoteDir = lpprl -> lszRemoteDir;
    }

    *pcb = sizeof(DWORD);
    *pcb += _ftcslen(lpprl -> lszRemoteExe) + 1;
    *pcb += lpprl->lszCmdLine? (_ftcslen(lpprl -> lszCmdLine) + 1) : 1;

    *pcb += _ftcslen(lszRemoteDir) + 1;
    *pcb += sizeof (SPAWNORPHAN);

#if defined(_UNICODE)
#pragma message("MHAlloc and *lplpvPacket+ctch need work")
#endif
    *lplpvPacket = malloc(*pcb);
    lpb = (BYTE*) *lplpvPacket;

    if (!*lplpvPacket) {
        return xosdOutOfMemory;
    }

//  REVIEW:  SwapEndian ( &dwChildFlags, sizeof ( dwChildFlags ) );

    memcpy (lpb, &(lpprl -> dwChildFlags), sizeof (lpprl -> dwChildFlags));
    lpb += sizeof(DWORD);

    _ftcscpy((CHAR*) lpb, lpprl -> lszRemoteExe);
    lpb += _ftcslen (lpprl -> lszRemoteExe) + 1;

    if (lpprl->lszCmdLine) {
        _ftcscpy((CHAR*) lpb, lpprl -> lszCmdLine);
        lpb += _ftcslen (lpprl -> lszCmdLine) + 1;
    } else {
        *lpb++ = 0;
    }

    _ftcscpy((CHAR*) lpb, lszRemoteDir);
    lpb += _ftcslen (lszRemoteDir) + 1;

    if (lpprl -> lpso) {
        memcpy ((CHAR*) lpb, lpprl -> lpso, sizeof (SPAWNORPHAN));
    } else {
        *lpb = 0;
    }

    return xosdNone;
}

BOOL
ProgramLoad(
    HPID   hpid,
    LPSTR  lpProgName
    )
{
    PRL     prl;
    BYTE    buf[MAX_PATH];
    SETPTH  *setpth = (SETPTH *)&buf[0];
    UINT    cb;
    LPVOID  lpb;
    BOOL    rval = TRUE;


    hEventAttach  = CreateEvent( NULL, TRUE, FALSE, NULL );
    hEventEntryPoint  = CreateEvent( NULL, TRUE, FALSE, NULL );

    if (SendRequest( dmfCreatePid, hpid, NULL ) != xosdNone) {
        CloseHandle( hEventAttach );
        CloseHandle( hEventEntryPoint );
        return FALSE;
    }

    setpth->Set = TRUE;
    setpth->Path[0] = '\0';

    if (SendRequestX( dmfSetPath, hpid, NULL, sizeof(SETPTH), setpth ) != xosdNone) {
        CloseHandle( hEventAttach );
        CloseHandle( hEventEntryPoint );
        return FALSE;
    }

    prl.dwChildFlags = ulfMultiProcess;
    prl.lszCmdLine = lpProgName;

    if (RepackProgLoad(&prl, &lpb, &cb) != xosdNone) {
        CloseHandle( hEventAttach );
        CloseHandle( hEventEntryPoint );
        return FALSE;
    }

    if (SendRequestX(dmfProgLoad, hpid, NULL, cb, lpb) != xosdNone) {
        CloseHandle( hEventAttach );
        CloseHandle( hEventEntryPoint );
        free(lpb);
        return FALSE;
    }

    //
    // wait for the loader breakpoint
    //
    WaitForSingleObject( hEventAttach, INFINITE );
    SendRequestX( dmfGo, hpid, htidBpt, sizeof(gop), &gop );

    //
    // wait for the entrypoint breakpoint
    //
    WaitForSingleObject( hEventEntryPoint, INFINITE );
    SendRequestX( dmfGo, hpid, htidBpt, sizeof(gop), &gop );

    //
    // cleanup
    //
    CloseHandle( hEventAttach );
    CloseHandle( hEventEntryPoint );
    free(lpb);

    return TRUE;
}
