/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WNMAN.H
 *  WOW32 16-bit Winnls API support (manually-coded thunks)
 *
 *  History:
 *  Created 19-Feb-1992 by Junichi Okubo (junichio)
--*/
#ifdef FE_IME
ULONG FASTCALL	WN32SendIMEMessage(PVDMFRAME pFrame);
ULONG FASTCALL	WN32SendIMEMessageEx(PVDMFRAME pFrame);

ULONG FASTCALL	WN32WINNLSGetIMEHotkey(PVDMFRAME pFrame);
ULONG FASTCALL	WN32WINNLSEnableIME(PVDMFRAME pFrame);
ULONG FASTCALL	WN32WINNLSGetEnableStatus(PVDMFRAME pFrame);

ULONG FASTCALL	WN32IMPQueryIME(PVDMFRAME pFrame);
ULONG FASTCALL	WN32IMPGetIME(PVDMFRAME pFrame);
ULONG FASTCALL	WN32IMPSetIME(PVDMFRAME pFrame);

VOID GETIMEPRO16(PIMEPRO pimepro32, PIMEPRO16 pime16);
VOID SETIMEPRO16(PIMEPRO16 pime16, PIMEPRO pimepro32);

#endif // FE_IME
