/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    vdm.c

Abstract:

    This module implements Win32 APIs for VDMs

Author:

    Sudeepb Bharati (sudeepb) 04-Sep-1991

Revision History:

--*/

#include "basedll.h"
#include "apcompat.h"
#pragma hdrstop

BOOL
APIENTRY
GetBinaryTypeA(
    IN  LPCSTR   lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

/*++

Routine Description: ANSI version of GetBinaryTypeW.
    This API returns the binary type of lpApplicationName.

Arguments:
    lpApplicationName - Full pathname of the binary
    lpBinaryType - pointer where binary type will be returned.

Return Value:
    TRUE - if SUCCESS; lpBinaryType has following
                SCS_32BIT_BINARY    - Win32 Binary (NT or Chicago)
                SCS_DOS_BINARY      - DOS Binary
                SCS_WOW_BINARY      - Windows 3.X Binary
                SCS_PIF_BINARY      - PIF file
                SCS_POSIX_BINARY    - POSIX Binary
                SCS_OS216_BINARY    - OS/2 Binary
    FALSE - if file not found or of unknown type. More info with GetLastError
--*/
{
    NTSTATUS Status;
    PUNICODE_STRING CommandLine;
    ANSI_STRING AnsiString;
    UNICODE_STRING DynamicCommandLine;
    BOOLEAN bReturn = FALSE;

    CommandLine = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpApplicationName);
    if ( (ULONG)AnsiString.Length<<1 < (ULONG)NtCurrentTeb()->StaticUnicodeString.MaximumLength ) {
        DynamicCommandLine.Buffer = NULL;
        Status = RtlAnsiStringToUnicodeString(CommandLine,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        Status = RtlAnsiStringToUnicodeString(&DynamicCommandLine,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }

    bReturn = (BOOLEAN)GetBinaryTypeW(
             DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
             lpBinaryType);

    RtlFreeUnicodeString(&DynamicCommandLine);

    return((BOOL)bReturn);

}


BOOL
WINAPI
GetBinaryTypeW(
    IN  LPCWSTR  lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

/*++

Routine Description: Unicode version.
    This API returns the binary type of lpApplicationName.

Arguments:
    lpApplicationName - Full pathname of the binary
    lpBinaryType - pointer where binary type will be returned.

Return Value:
    TRUE - if SUCCESS; lpBinaryType has following
                SCS_32BIT_BINARY    - Win32 Binary (NT or Chicago)
                SCS_DOS_BINARY      - DOS Binary
                SCS_WOW_BINARY      - Windows 3.X Binary
                SCS_PIF_BINARY      - PIF file
                SCS_POSIX_BINARY    - POSIX Binary
                SCS_OS216_BINARY    - OS/2 Binary
    FALSE - if file not found or of unknown type. More info with GetLastError
--*/

{
    NTSTATUS Status;
    UNICODE_STRING PathName;
    RTL_RELATIVE_NAME RelativeName;
    BOOLEAN TranslationStatus;
    OBJECT_ATTRIBUTES Obja;
    PVOID FreeBuffer = NULL;
    HANDLE FileHandle, SectionHandle=NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    LONG fBinaryType = SCS_32BIT_BINARY;
    BOOLEAN bReturn = FALSE;
    SECTION_IMAGE_INFORMATION ImageInformation;


    try {
        //
        // Translate to an NT name.
        //

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                // DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
                                lpApplicationName,
                                &PathName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
            goto GBTtryexit;
            }

        FreeBuffer = PathName.Buffer;

        if ( RelativeName.RelativeName.Length ) {
            PathName = *(PUNICODE_STRING)&RelativeName.RelativeName;
            }
        else {
            RelativeName.ContainingDirectory = NULL;
            }

        InitializeObjectAttributes(
            &Obja,
            &PathName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

        //
        // Open the file for execute access
        //

        Status = NtOpenFile(
                    &FileHandle,
                    SYNCHRONIZE | FILE_EXECUTE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                    );
        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            goto GBTtryexit;
            }

        //
        // Create a section object backed by the file
        //

        Status = NtCreateSection(
                    &SectionHandle,
                    SECTION_ALL_ACCESS,
                    NULL,
                    NULL,
                    PAGE_EXECUTE,
                    SEC_IMAGE,
                    FileHandle
                    );
        NtClose(FileHandle);

        if (!NT_SUCCESS(Status) ) {

            SectionHandle = NULL;

            switch (Status) {
                case STATUS_INVALID_IMAGE_NE_FORMAT:
#ifdef i386
                    fBinaryType = SCS_OS216_BINARY;
                    break;
#endif

                case STATUS_INVALID_IMAGE_PROTECT:
                    fBinaryType = SCS_DOS_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_WIN_16:
                    fBinaryType = SCS_WOW_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_NOT_MZ:
                    fBinaryType = BaseIsDosApplication(&PathName, Status);
                    if (!fBinaryType){
                        BaseSetLastNTError(Status);
                        goto GBTtryexit;
                    }
                    fBinaryType = (fBinaryType  == BINARY_TYPE_DOS_PIF) ?
                                  SCS_PIF_BINARY : SCS_DOS_BINARY;
                    break;

                default:
                    BaseSetLastNTError(Status);
                    goto GBTtryexit;
                }
            }
        else {
            //
            // Query the section
            //

            Status = NtQuerySection(
                        SectionHandle,
                        SectionImageInformation,
                        &ImageInformation,
                        sizeof( ImageInformation ),
                        NULL
                        );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError(Status);
                goto GBTtryexit;
            }

            if (ImageInformation.ImageCharacteristics & IMAGE_FILE_DLL) {
                SetLastError(ERROR_BAD_EXE_FORMAT);
                goto GBTtryexit;
            }

            if (ImageInformation.Machine !=
                    RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress)->FileHeader.Machine) {

#ifdef MIPS
                if ( ImageInformation.Machine == IMAGE_FILE_MACHINE_R3000 ||
                     ImageInformation.Machine == IMAGE_FILE_MACHINE_R4000 ) {
                    ;
                }
                else {
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    goto GBTtryexit;
                }
#else
                SetLastError(ERROR_BAD_EXE_FORMAT);
                goto GBTtryexit;
#endif // MIPS
            }

            if ( ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
                ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI ) {


                if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_POSIX_CUI ) {
                    fBinaryType = SCS_POSIX_BINARY;
                }
            }


        }

        *lpBinaryType = fBinaryType;

        bReturn = TRUE;

GBTtryexit:;
        }
    finally {

        if (SectionHandle)
            NtClose(SectionHandle);

        if (FreeBuffer)
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
    return bReturn;
}


VOID
APIENTRY
VDMOperationStarted
(
    BOOL    IsWowCaller
    )

/*++

Routine Description:
    This routine is used by MVDM to tell base that it has hooked
    ctrl-c handler with console. If the cmd window is killed
    before VDM could hook ctrl-c, then we wont get a chance to
    cleanup our data structures. The absence of this call tells
    base that it has to clean up the resources next time a
    call is made to create a VDM.

Arguments:
    IsWowCaller - TRUE if the caller is WOWVDM

Return Value:

    None
--*/

{
    BaseUpdateVDMEntry(UPDATE_VDM_HOOKED_CTRLC,
                       NULL,
                       0,
                       IsWowCaller);
    return;
}


BOOL
APIENTRY
GetNextVDMCommand(
    PVDMINFO lpVDMInfo
    )

/*++

Routine Description:
    This routine is used by MVDM to get a new command to execute. The
    VDM is blocked untill a DOS/WOW binary is encountered.


Arguments:
    lpVDMInfo - pointer to VDMINFO where new DOS command and other
                enviornment information is returned.

    if lpVDMInfo is NULL, then the caller is
    asking whether its the first VDM in the system.

Return Value:

    TRUE - The operation was successful. lpVDMInfo is filled in.

    FALSE/NULL - The operation failed.

--*/

{

    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_GET_NEXT_VDM_COMMAND_MSG a = (PBASE_GET_NEXT_VDM_COMMAND_MSG)&m.u.GetNextVDMCommand;
    PBASE_EXIT_VDM_MSG c= (PBASE_EXIT_VDM_MSG)&m.u.ExitVDM;
    PBASE_IS_FIRST_VDM_MSG d= (PBASE_IS_FIRST_VDM_MSG)&m.u.IsFirstVDM;
    PBASE_SET_REENTER_COUNT_MSG e = (PBASE_SET_REENTER_COUNT_MSG)&m.u.SetReenterCount;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG Len,nPointers;
    USHORT VDMStateSave;

    // Special case to query the first VDM In the system.
    if(lpVDMInfo == NULL){
        Status = CsrClientCallServer(
                          (PCSR_API_MSG)&m,
                          NULL,
                          CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                              BasepIsFirstVDM
                                              ),
                          sizeof( *d )
                          );

        if (NT_SUCCESS(Status)) {
            return(d->FirstVDM);
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }

    // Special case to increment/decrement the re-enterancy count

    if (lpVDMInfo->VDMState == INCREMENT_REENTER_COUNT ||
        lpVDMInfo->VDMState == DECREMENT_REENTER_COUNT) {

        e->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
        e->fIncDec = lpVDMInfo->VDMState;
        Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
                        NULL,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                             BasepSetReenterCount
                                           ),
                        sizeof( *e )
                       );
        if (NT_SUCCESS(Status)) {
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
    }

    VDMStateSave = lpVDMInfo->VDMState;

    // console handle is always passed on in this case
    // wow is differentiated by a parameter a->VDMState
    // a->VDMState & ASKING_FOR_WOW_BINARY indicates wow

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (lpVDMInfo->VDMState & ASKING_FOR_PIF) {
       a->iTask = lpVDMInfo->iTask;
    }
    else {
       a->iTask = 0;
    }

    a->AppLen = lpVDMInfo->AppLen;
    a->PifLen = lpVDMInfo->PifLen;
    a->CmdLen = lpVDMInfo->CmdSize;
    a->EnvLen = lpVDMInfo->EnviornmentSize;
    a->ExitCode = lpVDMInfo->ErrorCode;
    a->VDMState = VDMStateSave;
    a->WaitObjectForVDM = 0;
    a->DesktopLen = lpVDMInfo->DesktopLen;
    a->TitleLen = lpVDMInfo->TitleLen;
    a->ReservedLen = lpVDMInfo->ReservedLen;
    a->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;

    // Find the total space for capture buffer

      // startup info
    Len = ROUND_UP(sizeof(STARTUPINFOA),4);
    nPointers = 1;

    if (lpVDMInfo->CmdSize) {
        Len += ROUND_UP(a->CmdLen,4);
        nPointers++;
        }

    if (lpVDMInfo->AppLen) {
        Len +=ROUND_UP(a->AppLen,4);
        nPointers++;
        }

    if (lpVDMInfo->PifLen) {
        Len +=ROUND_UP(a->PifLen,4);
        nPointers++;
        }

    if (lpVDMInfo->Enviornment) {
        nPointers++;
        Len+= (lpVDMInfo->EnviornmentSize) ?
                     ROUND_UP(lpVDMInfo->EnviornmentSize, 4) : 4;
        }

    if (lpVDMInfo->CurDirectoryLen == 0)
        a->CurDirectory = NULL;
    else{
        Len += ROUND_UP(lpVDMInfo->CurDirectoryLen,4);
        nPointers++;
        }

    if (lpVDMInfo->DesktopLen == 0)
        a->Desktop = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->DesktopLen,4);
        nPointers++;
        }

    if (lpVDMInfo->TitleLen == 0)
        a->Title = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->TitleLen,4);
        nPointers++;
        }

    if (lpVDMInfo->ReservedLen == 0)
        a->Reserved = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->ReservedLen,4);
        nPointers++;
        }

    CaptureBuffer = CsrAllocateCaptureBuffer(nPointers, Len);
    if (CaptureBuffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return FALSE;
        }

    if (lpVDMInfo->CmdLine) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->CmdSize,
                                   (PVOID *)&a->CmdLine
                                 );
        }
    else {
        a->CmdLine = NULL;
        }


    if (lpVDMInfo->AppLen) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->AppLen,
                                   (PVOID *)&a->AppName
                                 );
        }
    else {
        a->AppName = NULL;
        }

    if (lpVDMInfo->PifLen) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->PifLen,
                                   (PVOID *)&a->PifFile
                                 );
        }
    else {
        a->PifFile = NULL;
        }


    if (lpVDMInfo->EnviornmentSize) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->EnviornmentSize,
                                   (PVOID *)&a->Env
                                 );
        }
    else {
        a->Env = NULL;
        }

    if (lpVDMInfo->CurDirectoryLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->CurDirectoryLen,
                                   (PVOID *)&a->CurDirectory
                                 );
    else
        a->CurDirectory = NULL;


    CsrAllocateMessagePointer( CaptureBuffer,
                               sizeof(STARTUPINFOA),
                               (PVOID *)&a->StartupInfo
                             );

    if (lpVDMInfo->DesktopLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->DesktopLen,
                                   (PVOID *)&a->Desktop
                                 );
    else
        a->Desktop = NULL;

    if (lpVDMInfo->TitleLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->TitleLen,
                                   (PVOID *)&a->Title
                                 );
    else
        a->Title = NULL;

    if (lpVDMInfo->ReservedLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->ReservedLen,
                                   (PVOID *)&a->Reserved
                                 );
    else
        a->Reserved = NULL;

retry:
    Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
                        CaptureBuffer,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                            BasepGetNextVDMCommand
                                           ),
                        sizeof( *a )
                        );

    if (a->WaitObjectForVDM) {
        Status = NtWaitForSingleObject(a->WaitObjectForVDM,FALSE,NULL);
        if (Status != STATUS_SUCCESS){
            BaseSetLastNTError(Status);
            return FALSE;
            }
        else {
            a->VDMState |= ASKING_FOR_SECOND_TIME;
            a->ExitCode = 0;
            goto retry;
            }
        }

    if (NT_SUCCESS(Status)) {
        Status = (NTSTATUS)m.ReturnValue;
        }


    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_INVALID_PARAMETER) {
            //This means one of the buffer size is less than required.
            lpVDMInfo->CmdSize = a->CmdLen;
            lpVDMInfo->AppLen = a->AppLen;
            lpVDMInfo->PifLen = a->PifLen;
            lpVDMInfo->EnviornmentSize = a->EnvLen;
            lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
            lpVDMInfo->DesktopLen      = a->DesktopLen;
            lpVDMInfo->TitleLen        = a->TitleLen;
            lpVDMInfo->ReservedLen     = a->ReservedLen;
            }
        else {
            lpVDMInfo->CmdSize = 0;
            lpVDMInfo->AppLen = 0;
            lpVDMInfo->PifLen = 0;
            lpVDMInfo->EnviornmentSize = 0;
            lpVDMInfo->CurDirectoryLen = 0;
            lpVDMInfo->DesktopLen      = 0;
            lpVDMInfo->TitleLen        = 0;
            lpVDMInfo->ReservedLen     = 0;
            }
        CsrFreeCaptureBuffer( CaptureBuffer );
        BaseSetLastNTError(Status);
        return FALSE;
    }


    try {

        if (lpVDMInfo->CmdSize)
            RtlMoveMemory(lpVDMInfo->CmdLine,
                          a->CmdLine,
                          a->CmdLen);


        if (lpVDMInfo->AppLen)
            RtlMoveMemory(lpVDMInfo->AppName,
                          a->AppName,
                          a->AppLen);

        if (lpVDMInfo->PifLen)
            RtlMoveMemory(lpVDMInfo->PifFile,
                          a->PifFile,
                          a->PifLen);


        if (lpVDMInfo->Enviornment)
            RtlMoveMemory(lpVDMInfo->Enviornment,
                          a->Env,
                          a->EnvLen);


        if (lpVDMInfo->CurDirectoryLen)
            RtlMoveMemory(lpVDMInfo->CurDirectory,
                          a->CurDirectory,
                          a->CurDirectoryLen);

        if (a->VDMState & STARTUP_INFO_RETURNED)
            RtlMoveMemory(&lpVDMInfo->StartupInfo,
                          a->StartupInfo,
                          sizeof(STARTUPINFOA));

        if (lpVDMInfo->DesktopLen){
            RtlMoveMemory(lpVDMInfo->Desktop,
                          a->Desktop,
                          a->DesktopLen);
            lpVDMInfo->StartupInfo.lpDesktop = lpVDMInfo->Desktop;
        }


        if (lpVDMInfo->TitleLen){
            RtlMoveMemory(lpVDMInfo->Title,
                          a->Title,
                          a->TitleLen);
            lpVDMInfo->StartupInfo.lpTitle = lpVDMInfo->Title;
        }

        if (lpVDMInfo->ReservedLen){
            RtlMoveMemory(lpVDMInfo->Reserved,
                          a->Reserved,
                          a->ReservedLen);
            lpVDMInfo->StartupInfo.lpReserved = lpVDMInfo->Reserved;
        }

        lpVDMInfo->CmdSize = a->CmdLen;
        lpVDMInfo->AppLen = a->AppLen;
        lpVDMInfo->PifLen = a->PifLen;
        lpVDMInfo->EnviornmentSize = a->EnvLen;
        if (a->VDMState & STARTUP_INFO_RETURNED)
            lpVDMInfo->VDMState = STARTUP_INFO_RETURNED;
        else
            lpVDMInfo->VDMState = 0;
        lpVDMInfo->CurDrive = a->CurrentDrive;
        lpVDMInfo->StdIn  = a->StdIn;
        lpVDMInfo->StdOut = a->StdOut;
        lpVDMInfo->StdErr = a->StdErr;
        lpVDMInfo->iTask = a->iTask;
        lpVDMInfo->CodePage = a->CodePage;
        lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
        lpVDMInfo->DesktopLen = a->DesktopLen;
        lpVDMInfo->TitleLen = a->TitleLen;
        lpVDMInfo->ReservedLen = a->ReservedLen;
        lpVDMInfo->dwCreationFlags = a->dwCreationFlags;
        lpVDMInfo->fComingFromBat = a->fComingFromBat;

        CsrFreeCaptureBuffer( CaptureBuffer );
        return TRUE;
        }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        BaseSetLastNTError(GetExceptionCode());
        CsrFreeCaptureBuffer( CaptureBuffer );
        return FALSE;
        }
}

VOID
APIENTRY
ExitVDM(
    BOOL IsWowCaller,
    ULONG iWowTask
    )

/*++

Routine Description:
    This routine is used by MVDM to exit.


Arguments:
    IsWowCaller - TRUE if the caller is WOWVDM.
                  FALSE if the caller is DOSVDM
                  This parameter is obsolete as basesrv knows about the kind
                  of vdm that is calling us


    iWowTask - if IsWowCaller == FALSE then Dont Care
             - if IsWowCaller == TRUE && iWowTask != -1 kill iWowTask task
             - if IsWowCaller == TRUE && iWowTask == -1 kill all wow task

Return Value:
    None

--*/

{

    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_EXIT_VDM_MSG c= (PBASE_EXIT_VDM_MSG)&m.u.ExitVDM;

    c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (IsWowCaller) {
       c->iWowTask = iWowTask;
    }
    else {
       c->iWowTask = 0;
    }

    // this parameter means
    c->WaitObjectForVDM =0;

    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepExitVDM
                                         ),
                      sizeof( *c )
                      );
    if (NT_SUCCESS(Status) && c->WaitObjectForVDM) {
        NtClose (c->WaitObjectForVDM);
        }

    return;
}

/*++

Routine Description:
    Set new VDM current directories

Arguments:
    cchCurDir - length of buffer in bytes
    lpszCurDir - buffer to return the current director of NTVDM

Return Value:
    TRUE if function succeed
    FALSE if function failed, GetLastError() has the error code
--*/


BOOL
APIENTRY
SetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m.u.GetSetVDMCurDirs;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    // caller must have a valid console(WOW will fail)
    if (a->ConsoleHandle == (HANDLE) -1) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    if (cchCurDirs && lpszzCurDirs) {
        // get capture buffer, one pointer in the message

        CaptureBuffer = CsrAllocateCaptureBuffer(1, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

        CsrAllocateMessagePointer( CaptureBuffer,
                                   cchCurDirs,
                                   (PVOID *)&a->lpszzCurDirs
                                   );

        a->cchCurDirs = cchCurDirs;
        try {
            RtlMoveMemory(a->lpszzCurDirs, lpszzCurDirs, cchCurDirs);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError(GetExceptionCode());
            CsrFreeCaptureBuffer(CaptureBuffer);
            return FALSE;
        }
        Status = CsrClientCallServer(
                            (PCSR_API_MSG)&m,
                            CaptureBuffer,
                            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                BasepSetVDMCurDirs
                                                ),
                            sizeof( *a )
                            );
        CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
    return TRUE;
}




/*++

Routine Description:
    To return current directory of NTVDM.
    This allows the parent process(CMD.EXE in most cases) to keep track the
    current directory after each VDM execution.
    NOTE: this function doesn't apply to wow

Arguments:
    cchCurDir - length of buffer in bytes
    lpszCurDir - buffer to return the current director of NTVDM

    Note: We don't require the process id to the running VDM because
          current directories are global to every VDMs under a single NTVDM
          control -- each console handle has its own current directories
Return Value:
    ULONG - (1). number of bytes written to the given buffer if succeed
            (2). lentgh of the current directory including NULL
                 if the provided buffer is not large enough
            (3). 0  then GetLastError() has the error code
--*/


ULONG
APIENTRY
GetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m.u.GetSetVDMCurDirs;


    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    if (a->ConsoleHandle == (HANDLE) -1) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0L;
    }
    if (cchCurDirs && lpszzCurDirs) {
        CaptureBuffer = CsrAllocateCaptureBuffer(1, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

        CsrAllocateMessagePointer( CaptureBuffer,
                                   cchCurDirs,
                                   (PVOID *)&a->lpszzCurDirs
                                   );

        a->cchCurDirs = cchCurDirs;
    }
    else {
        a->cchCurDirs = 0;
        a->lpszzCurDirs = NULL;
        CaptureBuffer = NULL;
    }

    m.ReturnValue = 0xffffffff;

    Status = CsrClientCallServer(
                         (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepGetVDMCurDirs
                                             ),
                         sizeof( *a )
                         );

    if (m.ReturnValue == 0xffffffff) {
        a->cchCurDirs = 0;
        }

    if (NT_SUCCESS(Status)) {
        Status = m.ReturnValue;
        }

    if (NT_SUCCESS(Status)) {

        try {
            RtlMoveMemory(lpszzCurDirs, a->lpszzCurDirs, a->cchCurDirs);
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            a->cchCurDirs = 0;
            }
        }
    else {
        BaseSetLastNTError(Status);
        }

    if (CaptureBuffer) {
        CsrFreeCaptureBuffer(CaptureBuffer);
        }

    return a->cchCurDirs;
}


VOID
APIENTRY
CmdBatNotification(
    IN  ULONG   fBeginEnd
    )

/*++

Routine Description:
    This API lets base know about .bat processing from cmd. This is
    required by VDM, so that it can decided correctly when to  put
    command.com prompt on TSRs. If the command came from .bat file
    then VDM should'nt put its prompt. This is important for
    ventura publisher and civilization apps.

Arguments:
    fBeginEnd - CMD_BAT_OPERATION_STARTING  -> .BAT processing is starting
                CMD_BAT_OPERATION_TERMINATING -> .BAT processing is ending

Return Value:
    None
--*/

{
    BASE_API_MSG m;
    PBASE_BAT_NOTIFICATION_MSG a = (PBASE_BAT_NOTIFICATION_MSG)&m.u.BatNotification;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (a->ConsoleHandle == (HANDLE) -1)
        return;

    a->fBeginEnd = fBeginEnd;

    CsrClientCallServer((PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepBatNotification
                                             ),
                         sizeof( *a )
                         );

    return;
}


NTSTATUS
APIENTRY
RegisterWowExec(
    IN  HANDLE   hwndWowExec
    )

/*++

Routine Description:
    This API gives basesrv the window handle for the shared WowExec so
    it can send WM_WOWEXECSTARTAPP messages to WowExec.  This
    saves having a thread in WOW dedicated to GetNextVDMCommand.

Arguments:
    hwndWowExec - Win32 window handle for WowExec in shared WOW VDM.
                  Separate WOW VDMs don't register their WowExec handle
                  because they never get commands from base.
                  NULL is passed to de-register any given wowexec

Return Value:
   If hwndWowExec != NULL then returns success if wow had been registered successfully
   if hwndWowExec == NULL then returns success if no tasks are pending to be executed
--*/

{
    BASE_API_MSG m;
    PBASE_REGISTER_WOWEXEC_MSG a = &m.u.RegisterWowExec;
    NTSTATUS Status;

    a->hwndWowExec   = hwndWowExec;
    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    Status = CsrClientCallServer((PCSR_API_MSG)&m,
                                  NULL,
                                  CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                      BasepRegisterWowExec
                                                      ),
                                  sizeof( *a )
                                 );
    return Status;
}


/*++

Routine Description:
    This routine is used to close standard IO handles before returning to the
    caller


Arguments:
    pVDMInfo - VDM Info record containing stdio handles

Return Value:
    None

--*/
VOID
BaseCloseStandardHandle(
    IN PVDMINFO pVDMInfo
    )
{
    if (pVDMInfo->StdIn)
        NtClose (pVDMInfo->StdIn);

    if (pVDMInfo->StdOut)
        NtClose (pVDMInfo->StdOut);

    if (pVDMInfo->StdErr)
        NtClose (pVDMInfo->StdErr);

    pVDMInfo->StdIn  = 0;
    pVDMInfo->StdOut = 0;
    pVDMInfo->StdErr = 0;
}

#ifdef OLD_CFG_BASED

BOOL
BaseGetVDMKeyword(
    PCHAR KeywordLine,
    PCONFIG_KEYWORD *pKeywordLine,
    PCHAR KeywordSize,
    PULONG VdmSize
    )
{
    NTSTATUS Status;
    PCONFIG_FILE ConfigFile;
    PCONFIG_SECTION Section;
    STRING SectionName, KeywordName;
    PCONFIG_KEYWORD pKeywordSize;

    //
    // Retrieve the VDM configuration information from the config file
    //

    Status = RtlOpenConfigFile( NULL, &ConfigFile );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    //
    // Find WOW section of config file
    //

    RtlInitString( &SectionName, "WOW" );
    Section = RtlLocateSectionConfigFile( ConfigFile, &SectionName );
    if (Section == NULL) {
        RtlCloseConfigFile( ConfigFile );
        return FALSE;
    }

    //
    // Get command line
    //

    RtlInitString( &KeywordName, KeywordLine );
    *pKeywordLine = RtlLocateKeywordConfigFile( Section, &KeywordName );
    if (*pKeywordLine == NULL) {
        RtlCloseConfigFile( ConfigFile );
        return FALSE;
    }
    //
    // Get Vdm size
    //

    RtlInitString( &KeywordName, KeywordSize );
    pKeywordSize = RtlLocateKeywordConfigFile( Section, &KeywordName );
    if (pKeywordSize == NULL) {
        *VdmSize = 1024L * 1024L * 16L;
    } else {
        Status = RtlCharToInteger( pKeywordSize->Value.Buffer, 0, VdmSize );
        if (!NT_SUCCESS( Status )) {
            *VdmSize = 1024L * 1024L * 16L;
        } else {
            *VdmSize *= 1024L * 1024L;   // convert to MB
        }
    }
    return TRUE;
}

#endif

BOOL
BaseGetVDMKeyword(
    LPWSTR  KeywordLine,
    LPSTR   KeywordLineValue,
    LPDWORD KeywordLineSize,
    LPWSTR  KeywordSize,
    LPDWORD VdmSize
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString,UnicodeTemp;
    UNICODE_STRING KeyName;
    ANSI_STRING AnsiString;
    LPWSTR UnicodeBuffer,Temp;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation;

    //
    // Allocate Work buffer
    //

    UnicodeBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ), FULL_INFO_BUFFER_SIZE);
    if (!UnicodeBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    // Open the WOW key

    RtlInitUnicodeString (&KeyName, WOW_ROOT);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        BaseSetLastNTError(NtStatus);
        return FALSE;
    }

    if (!GetVDMConfigValue(hKey,KeywordLine,UnicodeBuffer)) {
        NtClose (hKey);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return(FALSE);
    }

    //
    // Now convert back to ANSI for the caller after doing all the substitution
    //
    pKeyValueInformation = (PVOID)UnicodeBuffer;
    Temp = (LPWSTR)((PBYTE) pKeyValueInformation + pKeyValueInformation->DataOffset);
    RtlInitUnicodeString( &UnicodeString, Temp );
    UnicodeTemp.Buffer =  (LPWSTR)KeywordLineValue;
    UnicodeTemp.Length =  0;
    UnicodeTemp.MaximumLength = MAX_VDM_CFG_LINE;
    NtStatus = RtlExpandEnvironmentStrings_U    (NULL,&UnicodeString, &UnicodeTemp, NULL);
    if (!NT_SUCCESS( NtStatus )){
        NtClose (hKey);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return FALSE;
    }
    wcscpy(UnicodeString.Buffer,UnicodeTemp.Buffer);
    UnicodeString.Length = UnicodeTemp.Length;

    //
    // Set up an ANSI_STRING that points to the user's buffer
    //

    AnsiString.MaximumLength = (USHORT) *KeywordLineSize;
    AnsiString.Length = 0;
    AnsiString.Buffer = KeywordLineValue;


    RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

    *KeywordLineSize = AnsiString.Length;

    // Always set the VDMSize to 16Mb. (This is for reservation only)
    // Actual commit is done by SAS_INIT.

    *VdmSize = 16L;             //default value is 16
    *VdmSize *= 1024L * 1024L;  // convert From MB

    NtClose (hKey);
    RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
    return(TRUE);

}

BOOL
GetVDMConfigValue(
    HANDLE hKey,
    LPWSTR Keyword,
    LPWSTR UnicodeBuffer
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING ValueName;
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation = (PVOID) UnicodeBuffer;
    ULONG ValueLength;

    RtlInitUnicodeString(&ValueName, Keyword);
    NtStatus = NtQueryValueKey(hKey,
                               &ValueName,
                               KeyValueFullInformation,
                               pKeyValueInformation,
                               FULL_INFO_BUFFER_SIZE,
                               &ValueLength);

    if (NT_SUCCESS(NtStatus))
            return TRUE;
    else {
         BaseSetLastNTError (NtStatus);
         return FALSE;
    }
}

BOOL
BaseCheckVDM(
    IN  ULONG BinaryType,
    IN  PCWCH lpApplicationName,
    IN  PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN  ANSI_STRING *pAnsiStringEnv,
    IN  PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN  DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo
    )
/*++

Routine Description:

    This routine calls the windows server to find out if the VDM for the
    current session is already present. If so, a new process is'nt created
    instead the DOS binary is dispatched to the existing VDM. Otherwise,
    a new VDM process is created. This routine also passes the app name
    and command line to the server in DOS int21/0ah style which is later
    passed by the server to the VDM.

Arguments:

    BinaryType - DOS/WOW binary
    lpApplicationName -- pointer to the full path name of the executable.
    lpCommandLine -- command line
    lpCurrentDirectory - Current directory
    lpEnvironment,     - Envirinment strings
    m - pointer to the base api message.
    iTask - taskid for win16 apps, and no-console dos apps
    dwCreationFlags - creation flags as passed to createprocess
    lpStartupInfo =- pointer to startupinfo as passed to createprocess


Return Value:

    OEM vs. ANSI:
    The command line, Application Name, title are converted to OEM strings,
    suitable for the VDM. All other strings are returned as ANSI.

    TRUE -- Operation successful, VDM state and other relevant information
            is in base api message.
    FALSE -- Operation failed.

--*/
{

    NTSTATUS Status;
    PPEB Peb;
    PBASE_CHECKVDM_MSG b= (PBASE_CHECKVDM_MSG)&m->u.CheckVDM;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ANSI_STRING AnsiStringCurrentDir,AnsiStringDesktop;
    ANSI_STRING AnsiStringReserved, AnsiStringPif;
    OEM_STRING OemStringCmd, OemStringAppName, OemStringTitle;
    UNICODE_STRING UnicodeString;
    PCHAR pch, Buffer = NULL;
    ULONG Len;
    ULONG bufPointers;
    LPWSTR wsBuffer;
    LPWSTR wsAppName;
    LPWSTR wsPifName;
    LPWSTR wsCmdLine;
    LPWSTR wsPif=(PWSTR)".\0p\0i\0f\0\0";    // L".pif"
    LPWSTR wsSharedWowPif=L"wowexec.pif";
    PWCHAR pwch;
    BOOLEAN bNewConsole;
    BOOLEAN bReturn = FALSE;
    DWORD   dw, dwTotal, Length;
    WCHAR   wchBuffer[MAX_PATH + 1];
    ULONG BinarySubType;
    LPWSTR lpAllocatedReserved = NULL;
    DWORD   HandleFlags;

    // does a trivial test of the environment
    if (!ARGUMENT_PRESENT(pAnsiStringEnv) ||
        pAnsiStringEnv->Length > MAXIMUM_VDM_ENVIORNMENT) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    wsCmdLine = wsAppName = NULL;
    OemStringCmd.Buffer = NULL;
    OemStringAppName.Buffer = NULL;
    AnsiStringCurrentDir.Buffer = NULL;
    AnsiStringDesktop.Buffer = NULL;
    AnsiStringPif.Buffer = NULL;
    OemStringTitle.Buffer = NULL;
    AnsiStringReserved.Buffer = NULL;
    wsBuffer = NULL;
    wsPifName = NULL;

    BinarySubType = BinaryType & BINARY_SUBTYPE_MASK;
    BinaryType = BinaryType & ~BINARY_SUBTYPE_MASK;
    bNewConsole = !NtCurrentPeb()->ProcessParameters->ConsoleHandle ||
                  (dwCreationFlags & CREATE_NEW_CONSOLE);

    try {

        if (BinaryType == BINARY_TYPE_DOS) {

            Peb = NtCurrentPeb();
            if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) {
                b->StdIn = lpStartupInfo->hStdInput;
                b->StdOut = lpStartupInfo->hStdOutput;
                b->StdErr = lpStartupInfo->hStdError;

                }
            else {
                b->StdIn = Peb->ProcessParameters->StandardInput;
                b->StdOut = Peb->ProcessParameters->StandardOutput;
                b->StdErr = Peb->ProcessParameters->StandardError;

                //
                // Verify that the standard handles ntvdm process will inherit
                // from the calling process are real handles. They are not
                // handles if the calling process was created with
                // STARTF_USEHOTKEY | STARTF_HASSHELLDATA.
                // Note that CreateProcess clears STARTF_USESTANDHANDLES
                // if either STARTF_USEHOTKEY or STARTF_HASSHELLDATA is set.
                //
                if (Peb->ProcessParameters->WindowFlags &
                    (STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {

                    if (b->StdIn && !CONSOLE_HANDLE(b->StdIn) &&
                        !GetHandleInformation(b->StdIn, &HandleFlags))
                        b->StdIn = 0;
                    if (b->StdOut && !CONSOLE_HANDLE(b->StdOut) &&
                        !GetHandleInformation(b->StdOut, &HandleFlags)) {
                        if (b->StdErr == b->StdOut)
                            b->StdErr = 0;
                        b->StdOut = 0;
                        }
                    if (b->StdErr && b->StdErr != b->StdOut &&
                        !CONSOLE_HANDLE(b->StdErr) &&
                        !GetHandleInformation(b->StdErr, &HandleFlags))
                        b->StdErr = 0;
                    }
                }
            if (CONSOLE_HANDLE((b->StdIn)))
                b->StdIn = 0;

            if (CONSOLE_HANDLE((b->StdOut)))
                b->StdOut = 0;

            if (CONSOLE_HANDLE((b->StdErr)))
                b->StdErr = 0;
            }


        if (BinaryType == BINARY_TYPE_SEPWOW) {
            bNewConsole = TRUE;
            }

        //
        // Convert Unicode Application Name to Oem short name
        //
             // skiping leading white space
        while(*lpApplicationName == (WCHAR)' ' || *lpApplicationName == (WCHAR)'\t' ) {
              lpApplicationName++;
              }

             // space for short AppName
        Len = wcslen(lpApplicationName);
        dwTotal = Len + 1 + MAX_PATH;
        wsAppName =  RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(VDM_TAG),
                                    dwTotal * sizeof(WCHAR)
                                    );
        if (wsAppName == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        dw = GetShortPathNameW(lpApplicationName, wsAppName, dwTotal);
        // If getting the short name is impossible, stop right here.
        // We can not execute a 16bits biranry if we can not find
        // its appropriate short name alias. Sorry HPFS, Sorry NFS

        if (0 == dw || dw > dwTotal) {
            SetLastError(ERROR_BAD_PATHNAME);
            goto BCVTryExit;
            }

        RtlInitUnicodeString(&UnicodeString, wsAppName);
        Status = RtlUnicodeStringToOemString(&OemStringAppName,
                                             &UnicodeString,
                                             TRUE
                                             );
        if (!NT_SUCCESS(Status) ){
            BaseSetLastNTError(Status);
            goto BCVTryExit;
            }


        //
        // Find len of basename excluding extension,
        // for CommandTail max len check.
        //
        dw = OemStringAppName.Length;
        pch = OemStringAppName.Buffer;
        Length = 1;        // start at one for space between cmdname & cmdtail
        while (dw-- && *pch != '.') {
            if (*pch == '\\') {
                Length = 1;
                }
            else {
                Length++;
                }
            pch++;
            }


        //
        // Find the beg of the command tail to pass as the CmdLine
        //

        Len = wcslen(lpApplicationName);

        if (L'"' == lpCommandLine[0]) {

            //
            // Application name is quoted, skip the quoted text
            // to get command tail.
            //

            pwch = (LPWSTR)&lpCommandLine[1];
            while (*pwch && L'"' != *pwch++) {
                ;
            }

        } else if (Len <= wcslen(lpCommandLine) &&
            0 == _wcsnicmp(lpApplicationName, lpCommandLine, Len)) {

            //
            // Application path is also on the command line, skip past
            // that to reach the command tail instead of looking for
            // the first white space.
            //

            pwch = (LPWSTR)lpCommandLine + Len;

        } else {

            //
            // We assume first token is exename (argv[0]).
            //

            pwch = (LPWSTR)lpCommandLine;

               // skip leading white characters
            while (*pwch != UNICODE_NULL &&
                   (*pwch == (WCHAR) ' ' || *pwch == (WCHAR) '\t')) {
                pwch++;
                }

               // skip first token
            if (*pwch == (WCHAR) '\"') {    // quotes as delimiter
                pwch++;
                while (*pwch && *pwch++ != '\"') {
                      ;
                      }
                }
            else {                         // white space as delimiter
                while (*pwch && *pwch != ' ' && *pwch != '\t') {
                       pwch++;
                       }
                }
        }

        //
        // pwch points past the application name, now skip any trailing
        // whitespace.
        //

        while (*pwch && (L' ' == *pwch || L'\t' == *pwch)) {
            pwch++;
        }

        wsCmdLine = pwch;
        dw = wcslen(wsCmdLine);

        // convert to oem
        UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
        UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
        UnicodeString.Buffer = wsCmdLine;
        Status = RtlUnicodeStringToOemString(
                    &OemStringCmd,
                    &UnicodeString,
                    TRUE);

        if (!NT_SUCCESS(Status) ){
            BaseSetLastNTError(Status);
            goto BCVTryExit;
            }

        //
        // check len of command line for dos compatibility
        //
        if (OemStringCmd.Length >= MAXIMUM_VDM_COMMAND_LENGTH - Length) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto BCVTryExit;
            }


        //
        // Search for matching pif file. Search order is AppName dir,
        // followed by win32 default search path. For the shared wow, pif
        // is wowexec.pif if it exists.
        //
        wsBuffer = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsBuffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        wsPifName = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsPifName) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        if (BinaryType == BINARY_TYPE_WIN16) {
            wcscpy(wsBuffer, wsSharedWowPif);
            Len = 0;
            }
        else {
            // start with fully qualified app name
            wcscpy(wsBuffer, lpApplicationName);

             // strip extension if any
            pwch = wcsrchr(wsBuffer, (WCHAR)'.');
            // dos application must have an extention
            if (pwch == NULL) {
                 SetLastError(ERROR_INVALID_PARAMETER);
                 goto BCVTryExit;
                }
            wcscpy(pwch, wsPif);
            Len = GetFileAttributesW(wsBuffer);
            if (Len == (DWORD)(-1) || (Len & FILE_ATTRIBUTE_DIRECTORY)) {
                Len = 0;
                }
            else {
                Len = wcslen(wsBuffer) + 1;
                wcsncpy(wsPifName, wsBuffer, Len);
                }
            }

        if (!Len)  {  // try basename

               // find beg of basename
            pwch = wcsrchr(wsBuffer, (WCHAR)'\\');
            if (!pwch ) {
                 pwch = wcsrchr(wsBuffer, (WCHAR)':');
                 }

               // move basename to beg of wsBuffer
            if (pwch++) {
                 while (*pwch != UNICODE_NULL &&
                        *pwch != (WCHAR)' '   && *pwch != (WCHAR)'\t' )
                       {
                        wsBuffer[Len++] = *pwch++;
                        }
                 wsBuffer[Len] = UNICODE_NULL;
                 }

            if (Len)  {
                Len = SearchPathW(
                            NULL,
                            wsBuffer,
                            wsPif,              // L".pif"
                            MAX_PATH,
                            wsPifName,
                            NULL
                            );
                if (Len >= MAX_PATH) {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto BCVTryExit;
                    }
                }
            }

        if (!Len)
            *wsPifName = UNICODE_NULL;



        if (!ARGUMENT_PRESENT( lpCurrentDirectory )) {

            dw = RtlGetCurrentDirectory_U(sizeof (wchBuffer), wchBuffer);

            wchBuffer[dw / sizeof(WCHAR)] = UNICODE_NULL;
            dw = GetShortPathNameW(wchBuffer,
                                   wchBuffer,
                                   sizeof(wchBuffer) / sizeof(WCHAR)
                                   );
            if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
                goto BCVTryExit;

            else if (dw == 0) {
                RtlInitUnicodeString(&UnicodeString, wchBuffer);
                dw = UnicodeString.Length / sizeof(WCHAR);
                }
            else {
                UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
                UnicodeString.Buffer = wchBuffer;
                UnicodeString.MaximumLength = (USHORT)sizeof(wchBuffer);
                }
            // DOS limit of 64 includes the final NULL but not the leading
            // drive and slash. So here we should be checking the ansi length
            // of current directory + 1 (for NULL) - 3 (for c:\).
            if ( dw - 2 <= MAXIMUM_VDM_CURRENT_DIR ) {
                Status = RtlUnicodeStringToAnsiString(
                                                      &AnsiStringCurrentDir,
                                                      &UnicodeString,
                                                      TRUE
                                                     );
                }
            else {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto BCVTryExit;
                }

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                goto BCVTryExit;
                }
            }
        else {

            // first get a full path name
            dw = GetFullPathNameW(lpCurrentDirectory,
                                   sizeof(wchBuffer) / sizeof(WCHAR),
                                   wchBuffer,
                                   NULL);
            if (0 != dw && dw <= sizeof(wchBuffer) / sizeof(WCHAR)) {
               dw = GetShortPathNameW(wchBuffer,
                                      wchBuffer,
                                      sizeof(wchBuffer) / sizeof(WCHAR));
            }
            if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
                goto BCVTryExit;

            if (dw != 0) {
                UnicodeString.Buffer = wchBuffer;
                UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
                UnicodeString.MaximumLength = sizeof(wchBuffer);
                }
            else
                RtlInitUnicodeString(&UnicodeString, lpCurrentDirectory);

            Status = RtlUnicodeStringToAnsiString(
                &AnsiStringCurrentDir,
                &UnicodeString,
                TRUE);

            if ( !NT_SUCCESS(Status) ){
                BaseSetLastNTError(Status);
                goto BCVTryExit;
               }

            // DOS limit of 64 includes the final NULL but not the leading
            // drive and slash. So here we should be checking the ansi length
            // of current directory + 1 (for NULL) - 3 (for c:\).
            if((AnsiStringCurrentDir.Length - 2) > MAXIMUM_VDM_CURRENT_DIR) {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto BCVTryExit;
                }
            }

        // NT allows applications to use UNC name as their current directory.
        // while NTVDM can't do that. We will end up a weird drive number
        // like '\' - 'a') here ????????????????????????????????
        // BUGBUG
        // Place Current Drive
        if(AnsiStringCurrentDir.Buffer[0] <= 'Z')
            b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'A';
        else
            b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'a';

        //
        // Hotkey info in NT traditionally is specified in the
        // startupinfo.lpReserved field, but Win95 added a
        // duplicate mechanism.  If the Win95 method was used,
        // map it to the NT method here so the rest of the
        // VDM code only has to deal with one method.
        //
        // If the caller was stupid enough to specify a hotkey
        // in lpReserved as well as using STARTF_USEHOTKEY,
        // the STARTF_USEHOTKEY hotkey will take precedence.
        //

        if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USEHOTKEY) {

            DWORD cbAlloc = sizeof(WCHAR) *
                            (20 +                            // "hotkey.4294967295 " (MAXULONG)
                             (lpStartupInfo->lpReserved      // length of prev lpReserved
                              ? wcslen(lpStartupInfo->lpReserved)
                              : 0
                             ) +
                             1                               // NULL terminator
                            );


            lpAllocatedReserved = RtlAllocateHeap(RtlProcessHeap(),
                                                  MAKE_TAG( VDM_TAG ),
                                                  cbAlloc
                                                 );
            if (lpAllocatedReserved) {

                swprintf(lpAllocatedReserved,
                         L"hotkey.%u %s",
                         HandleToUlong(lpStartupInfo->hStdInput),
                         lpStartupInfo->lpReserved ? lpStartupInfo->lpReserved : L""
                         );

                lpStartupInfo->dwFlags &= ~STARTF_USEHOTKEY;
                lpStartupInfo->hStdInput = 0;
                lpStartupInfo->lpReserved = lpAllocatedReserved;

            }

        }


        //
        // Allocate Capture Buffer
        //
        //
        bufPointers = 2;  // CmdLine, AppName

        //
        // CmdLine for capture buffer, 3 for 0xd,0xa and NULL
        //
        Len = ROUND_UP((OemStringCmd.Length + 3),4);

        // AppName, 1 for NULL
        Len += ROUND_UP((OemStringAppName.Length + 1),4);

        // Env
        if (pAnsiStringEnv->Length) {
            bufPointers++;
            Len += ROUND_UP(pAnsiStringEnv->Length, 4);
            }

        // CurrentDir
        if (AnsiStringCurrentDir.Length){
            bufPointers++;
            Len += ROUND_UP((AnsiStringCurrentDir.Length +1),4); // 1 for NULL
            }


        // pif file name, 1 for NULL
        if (wsPifName && *wsPifName != UNICODE_NULL) {
            bufPointers++;
            RtlInitUnicodeString(&UnicodeString,wsPifName);
            Status = RtlUnicodeStringToAnsiString(&AnsiStringPif,
                                                  &UnicodeString,
                                                  TRUE
                                                  );
            if ( !NT_SUCCESS(Status) ){
                BaseSetLastNTError(Status);
                goto BCVTryExit;
                }

            Len += ROUND_UP((AnsiStringPif.Length+1),4);
            }

        //
        // startupinfo space
        //
        if (lpStartupInfo) {
            Len += ROUND_UP(sizeof(STARTUPINFOA),4);
            bufPointers++;
            if (lpStartupInfo->lpDesktop) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpDesktop);
                Status = RtlUnicodeStringToAnsiString(
                            &AnsiStringDesktop,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    BaseSetLastNTError(Status);
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((AnsiStringDesktop.Length+1),4);
                }

            if (lpStartupInfo->lpTitle) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpTitle);
                Status = RtlUnicodeStringToOemString(
                            &OemStringTitle,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    BaseSetLastNTError(Status);
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((OemStringTitle.Length+1),4);
                }

            if (lpStartupInfo->lpReserved) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpReserved);
                Status = RtlUnicodeStringToAnsiString(
                            &AnsiStringReserved,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    BaseSetLastNTError(Status);
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((AnsiStringReserved.Length+1),4);
                }
            }


        // capture message buffer
        CaptureBuffer = CsrAllocateCaptureBuffer(bufPointers, Len);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            goto BCVTryExit;
            }

        // Allocate CmdLine pointer
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringCmd.Length + 3),4),
                                   (PVOID *)&b->CmdLine
                                 );

        // Copy Command Line
        RtlMoveMemory (b->CmdLine, OemStringCmd.Buffer, OemStringCmd.Length);
        b->CmdLine[OemStringCmd.Length] = 0xd;
        b->CmdLine[OemStringCmd.Length+1] = 0xa;
        b->CmdLine[OemStringCmd.Length+2] = 0;
        b->CmdLen = (USHORT)(OemStringCmd.Length + 3);

        // Allocate AppName pointer
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringAppName.Length + 1),4),
                                   (PVOID *)&b->AppName
                                 );

        // Copy AppName
        RtlMoveMemory (b->AppName,
                       OemStringAppName.Buffer,
                       OemStringAppName.Length
                       );
        b->AppName[OemStringAppName.Length] = 0;
        b->AppLen = OemStringAppName.Length + 1;




        // Allocate PifFile pointer, Copy PifFile name
        if(AnsiStringPif.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringPif.Length + 1),4),
                                       (PVOID *)&b->PifFile
                                     );

            RtlMoveMemory(b->PifFile,
                          AnsiStringPif.Buffer,
                          AnsiStringPif.Length);

            b->PifFile[AnsiStringPif.Length] = 0;
            b->PifLen = AnsiStringPif.Length + 1;

            }
        else {
            b->PifLen = 0;
            b->PifFile = NULL;
            }



        // Allocate Env pointer, Copy Env strings
        if(pAnsiStringEnv->Length) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((pAnsiStringEnv->Length),4),
                                       (PVOID *)&b->Env
                                     );

            RtlMoveMemory(b->Env,
                          pAnsiStringEnv->Buffer,
                          pAnsiStringEnv->Length);

            b->EnvLen = pAnsiStringEnv->Length;

            }
        else {
            b->EnvLen = 0;
            b->Env = NULL;
            }


        if(AnsiStringCurrentDir.Length) {
            // Allocate Curdir pointer
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringCurrentDir.Length + 1),4),
                                       (PVOID *)&b->CurDirectory
                                       );
            // copy cur directory
            RtlMoveMemory (b->CurDirectory,
                           AnsiStringCurrentDir.Buffer,
                           AnsiStringCurrentDir.Length+1);

            b->CurDirectoryLen = AnsiStringCurrentDir.Length+1;
            }
        else {
            b->CurDirectory = NULL;
            b->CurDirectoryLen = 0;
            }

        // Allocate startupinfo pointer
        if (lpStartupInfo) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP(sizeof(STARTUPINFOA),4),
                                       (PVOID *)&b->StartupInfo
                                     );
            // Copy startupinfo
            b->StartupInfo->dwX  =  lpStartupInfo->dwX;
            b->StartupInfo->dwY  =  lpStartupInfo->dwY;
            b->StartupInfo->dwXSize      =  lpStartupInfo->dwXSize;
            b->StartupInfo->dwYSize      =  lpStartupInfo->dwYSize;
            b->StartupInfo->dwXCountChars=      lpStartupInfo->dwXCountChars;
            b->StartupInfo->dwYCountChars=      lpStartupInfo->dwYCountChars;
            b->StartupInfo->dwFillAttribute=lpStartupInfo->dwFillAttribute;
            b->StartupInfo->dwFlags      =  lpStartupInfo->dwFlags;
            b->StartupInfo->wShowWindow =       lpStartupInfo->wShowWindow;
            b->StartupInfo->cb           =  sizeof(STARTUPINFOA);
            }
        else {
            b->StartupInfo = NULL;
            }

        // Allocate pointer for Desktop info if needed
        if (AnsiStringDesktop.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringDesktop.Length + 1),4),
                                       (PVOID *)&b->Desktop
                                     );
            // Copy desktop string
            RtlMoveMemory (b->Desktop,
                           AnsiStringDesktop.Buffer,
                           AnsiStringDesktop.Length+1);
            b->DesktopLen =AnsiStringDesktop.Length+1;
            }
        else {
            b->Desktop = NULL;
            b->DesktopLen =0;
            }

        // Allocate pointer for Title info if needed
        if (OemStringTitle.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((OemStringTitle.Length + 1),4),
                                       (PVOID *)&b->Title
                                     );
            // Copy title string
            RtlMoveMemory (b->Title,
                           OemStringTitle.Buffer,
                           OemStringTitle.Length+1);
            b->TitleLen = OemStringTitle.Length+1;
            }
        else {
            b->Title = NULL;
            b->TitleLen = 0;
            }

        // Allocate pointer for Reserved field if needed
        if (AnsiStringReserved.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringReserved.Length + 1),4),
                                       (PVOID *)&b->Reserved
                                     );
            // Copy reserved string
            RtlMoveMemory (b->Reserved,
                           AnsiStringReserved.Buffer,
                           AnsiStringReserved.Length+1);
            b->ReservedLen = AnsiStringReserved.Length+1;
            }
        else {
            b->Reserved = NULL;
            b->ReservedLen = 0;
            }

        // VadimB: this code is of no consequence to our marvelous new
        // architecture for tracking shared wows.
        // Reason: the checkvdm command is executed within the context of
        // a parent process thus at this point ConsoleHandle is of any
        // interest only to DOS apps.

        if (BinaryType == BINARY_TYPE_WIN16)
            b->ConsoleHandle = (HANDLE)-1;
        else if (bNewConsole)
            b->ConsoleHandle = 0;
        else
            b->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

        b->VDMState = FALSE;
        b->BinaryType = BinaryType;
        b->CodePage = (ULONG) GetConsoleCP ();
        b->dwCreationFlags = dwCreationFlags;


        Status = CsrClientCallServer(
                          (PCSR_API_MSG)m,
                          CaptureBuffer,
                          CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                               BasepCheckVDM
                                             ),
                          sizeof( *b )
                          );

        CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m->ReturnValue)) {
            BaseSetLastNTError((NTSTATUS)m->ReturnValue);
            goto BCVTryExit;
            }

        // VadimB: This iTask could be :
        //   (*) If not wow task - then dos task id (items below are not
        //       relevant for this case)
        //   (*) Shared wow exists and ready - this is a wow task id
        //       that is unique across all the shared wows

        *iTask = b->iTask;
        bReturn = TRUE;
BCVTryExit:;
        }

    finally {
        if(Buffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Buffer);

        if(wsBuffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsBuffer);

        if(wsPifName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsPifName);

        if(OemStringCmd.Buffer != NULL)
            RtlFreeOemString(&OemStringCmd);

        if(OemStringAppName.Buffer != NULL)
            RtlFreeOemString(&OemStringAppName);

        if(AnsiStringPif.Buffer != NULL)
           RtlFreeAnsiString(&AnsiStringPif);

        if(AnsiStringCurrentDir.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringCurrentDir);

        if(AnsiStringDesktop.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringDesktop);

        if(OemStringTitle.Buffer != NULL)
            RtlFreeAnsiString(&OemStringTitle);

        if(AnsiStringReserved.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringReserved);

        if (wsAppName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, wsAppName);

        if (lpAllocatedReserved != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, lpAllocatedReserved);

        }

    return bReturn;
}

BOOL
BaseUpdateVDMEntry(
    IN ULONG UpdateIndex,
    IN OUT HANDLE *WaitHandle,
    IN ULONG IndexInfo,
    IN ULONG BinaryType
    )
{
    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_UPDATE_VDM_ENTRY_MSG c= (PBASE_UPDATE_VDM_ENTRY_MSG)&m.u.UpdateVDMEntry;

    switch (UpdateIndex) {
        case UPDATE_VDM_UNDO_CREATION:
            c->iTask = HandleToUlong(*WaitHandle);
            c->VDMCreationState = (USHORT)IndexInfo;
            break;
        case UPDATE_VDM_PROCESS_HANDLE:
            c->VDMProcessHandle = *WaitHandle;  // Actually this is VDM handle
            c->iTask = IndexInfo;
            break;
        }

    // VadimB: this ConsoleHandle is of no consequence to the
    // shared wow tracking mechanism

    if(BinaryType == BINARY_TYPE_WIN16)
        c->ConsoleHandle = (HANDLE)-1;
    else if (c->iTask)
        c->ConsoleHandle = 0;
    else
        c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    c->EntryIndex = (WORD)UpdateIndex;
    c->BinaryType = BinaryType;


    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepUpdateVDMEntry
                                         ),
                      sizeof( *c )
                      );

    if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    switch (UpdateIndex) {
        case UPDATE_VDM_UNDO_CREATION:
            break;
        case UPDATE_VDM_PROCESS_HANDLE:
            *WaitHandle = c->WaitObjectForParent;
            break;
    }
    return TRUE;
}




ULONG
BaseIsDosApplication(
    IN PUNICODE_STRING PathName,
    IN NTSTATUS Status
    )
/*++

Routine Description:

    Determines if app is a ".com" or a ".pif" type of app
    by looking at the extension, and the Status from NtCreateSection
    for PAGE_EXECUTE.

Arguments:

    PathName    -- Supplies a pointer to the path string
    Status      -- Status code from CreateSection call
    bNewConsole -- Pif can exec only from a new console

Return Value:

    file is a com\pif dos application
    SCS_DOS_BINARY - ".com", may also be a .exe extension
    SCS_PIF_BINARY - ".pif"


    0 -- file is not a dos application, may be a .bat or .cmd file

--*/
{
    UNICODE_STRING String;

         // check for .com extension
    String.Length = BaseDotComSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotComSuffixName, TRUE))
        return BINARY_TYPE_DOS_COM;


        // check for .pif extension
    String.Length = BaseDotPifSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotPifSuffixName, TRUE))
        return BINARY_TYPE_DOS_PIF;


        // check for .exe extension
    String.Length = BaseDotExeSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
        sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotExeSuffixName, TRUE))
        return BINARY_TYPE_DOS_EXE;

    return 0;
}



BOOL
BaseGetVdmConfigInfo(
    IN  LPCWSTR CommandLine,
    IN  ULONG   DosSeqId,
    IN  ULONG   BinaryType,
    IN  PUNICODE_STRING CmdLineString,
    OUT PULONG VdmSize
    )
/*++

Routine Description:

    This routine locates the VDM configuration information for Wow vdms in
    the system configuration file.  It also reconstructs the commandline so
    that we can start the VDM.  The new command line is composed from the
    information in the configuration file + the old command line.

Arguments:

    CommandLine -- pointer to a string pointer that is used to pass the
        command line string

    DosSeqId - new console session id. This parameter is also valid for
               shared wow as it is passed to ntvdm as -i parameter. Another
               parameter to identify shared wow is passed to ntvdm as
               '-ws' where 'w' stands for wow app, 's' stands for separate
               In response to this 's' parameter ntvdm launches a
               separate wow (one-time shot). By default, ntvdm starts a shared
               wow.

    VdmSize -- Returns the size in bytes of the VDM to be created

    BinaryType - dos, sharedwow, sepwow


Return Value:

    TRUE -- VDM configuration information was available
    FALSE -- VDM configuration information was not available

Notes:

--*/
{
    NTSTATUS Status;
    BOOL bRet;
    DWORD dw;
    ANSI_STRING AnsiString;
    LPSTR NewCmdLine=NULL;
    PCH   pSrc, pDst, pch;
    ULONG Len;
    char CmdLine[MAX_VDM_CFG_LINE];

    CmdLineString->Buffer = NULL;

    Len = MAX_VDM_CFG_LINE;


    if (BinaryType == BINARY_TYPE_DOS) {
        bRet = BaseGetVDMKeyword(CMDLINE, CmdLine, &Len, DOSSIZE, VdmSize);
        }
    else {
        bRet = BaseGetVDMKeyword(WOWCMDLINE, CmdLine, &Len, WOWSIZE, VdmSize);
        }

    if (!bRet) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }

    //
    // Allocate memory to replace the CommandLine
    // extra space is needed for long->short name conversion,
    // separate wow, and extension.
    //

    NewCmdLine = RtlAllocateHeap(RtlProcessHeap(),
                                 MAKE_TAG( VDM_TAG ),
                                 MAX_PATH + MAX_VDM_CFG_LINE
                                 );
    if (!NewCmdLine) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }


    //
    // Copy over the cmdline checking for special args
    // and locating the beg of wowkernel
    //
    pSrc = CmdLine;
    pDst = NewCmdLine;


    //
    // first token must be "\\%SystemRoot%\\system32\\ntvdm", search
    // for the tail of the pathname to traverse possible long file name
    // safely.
    //
    pch = strstr(pSrc, "\\system32\\ntvdm");
    if (!pch) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }

    // mov pch to trailing space in "ntvdm "
    while (*pch && *pch != ' ') {
        pch++;
        }

    //
    // copy first token (ntvdm path name), surrounded by quotes for
    // possible long file name
    //
   *pDst++ = '\"';
    while (pSrc < pch) {
       *pDst++ = *pSrc++;
       }
    *pDst++ = '\"';


    //
    // Add -f arg, so ntvdm knows it wasn't invoked directly
    //
    *pDst++ = ' ';
    *pDst++ = '-';
    *pDst++ = 'f';

    //
    // Add DosSeqId for new console
    //
    if (DosSeqId) {
        sprintf(pDst, " -i%lx", DosSeqId);
        pDst += strlen(pDst);
        }

    //
    // Copy over everything up to the " -a " (exclusive)
    // CAVEAT: we assume -a is last
    //
    pch = strstr(pSrc, " -a ");
    if (pch) {
        while (pSrc < pch) {
           *pDst++ = *pSrc++;
           }
        }
    else {
        while (*pSrc) {
           *pDst++ = *pSrc++;
           }
        }

    *pDst = '\0';


    //
    // for wow -a is mandatory to specify win16 krnl, and is expected
    // to be the last cmdline parameter
    //
    if (BinaryType != BINARY_TYPE_DOS) { // shared wow, sep wow
        PCH pWowKernel;

        if (!*pSrc) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }

        //
        // Add -w to tell ntvdm its wow (mandatory)
        //
        *pDst++ = ' ';
        *pDst++ = '-';
        *pDst++ = 'w';

        //
        // identify that this wow is a separate wow by -ws
        //

        if (BINARY_TYPE_SEPWOW == BinaryType) {
           *pDst++ = 's';
        }

        //
        // copy over the " -a WowKernelPathname" argument
        // and locate beg of WowKernelPathname in destination
        //
        pWowKernel = pDst;
        while (*pSrc) {
           *pDst++ = *pSrc++;
           }
        pWowKernel += 4;      // find beg of WowKernelPathaname
        while (*pWowKernel == ' ') {
           pWowKernel++;
           }

        //
        // Append file extension to destination
        //
        strcpy(pDst, ".exe");

        //
        // convert wowkernel to short name
        //
        Len = MAX_PATH + MAX_VDM_CFG_LINE - (ULONG)((ULONG_PTR)pWowKernel - (ULONG_PTR)NewCmdLine) -1;
        dw = GetShortPathNameA(pWowKernel, pWowKernel, Len);
        if (dw > Len) {
            RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
            return FALSE;
            }
        }

    RtlInitAnsiString(&AnsiString, NewCmdLine);
    Status = RtlAnsiStringToUnicodeString(CmdLineString, &AnsiString, TRUE);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
        CmdLineString->Buffer = NULL;
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
    return TRUE;
}





BOOL
BaseCheckForVDM(
    IN HANDLE hProcess,
    OUT LPDWORD lpExitCode
    )
{
    NTSTATUS Status;
    EVENT_BASIC_INFORMATION ebi;
    BASE_API_MSG m;
    PBASE_GET_VDM_EXIT_CODE_MSG a = (PBASE_GET_VDM_EXIT_CODE_MSG)&m.u.GetVDMExitCode;

    Status = NtQueryEvent (
                hProcess,
                EventBasicInformation,
                &ebi,
                sizeof(ebi),
                NULL);

    if(!NT_SUCCESS(Status))
        return FALSE;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    a->hParent = hProcess;
    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                      BasepGetVDMExitCode),
                      sizeof( *a )
                      );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
        }

    *lpExitCode = (DWORD)a->ExitCode;

    return TRUE;
}




DWORD
APIENTRY
GetShortPathNameA(
    IN  LPCSTR  lpszLongPath,
    IN  LPSTR   lpShortPath,
    IN  DWORD   cchBuffer
    )
{
    UNICODE_STRING  UString, UStringRet;
    ANSI_STRING     AString;
    NTSTATUS        Status;
    WCHAR           TempPathW[MAX_PATH];
    LPWSTR          lpShortPathW = NULL;
    DWORD           ReturnValue;
    DWORD           ReturnValueW;

    if (lpszLongPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }
    // We have to initialize it before the "try" statement
    AString.Buffer = NULL;
    UString.Buffer = NULL;

    ReturnValue = 0;
    ReturnValueW = 0;

    try {
        if (!Basep8BitStringToDynamicUnicodeString(&UString, lpszLongPath )) {
            goto gspTryExit;
            }

        // we have to get the real converted path in order to find out
        // the required length. An UNICODE char does not necessarily convert
        // to one ANSI char(A DBCS is basically TWO ANSI char!!!!!).
        // First, we use the buffer allocated from the stack. If the buffer
        // is too small, we then allocate it from heap.
        // A check of (lpShortPathW && TempPathW != lpShortPathW) will reveal
        // if we have allocated a buffer from heap and need to release it.
        lpShortPathW = TempPathW;
        ReturnValueW = GetShortPathNameW(UString.Buffer, lpShortPathW, sizeof(TempPathW) / sizeof(WCHAR));
        if (ReturnValueW >= sizeof(TempPathW) / sizeof(WCHAR))
            {
            // the stack-based buffer is too small. Allocate a new buffer
            // from heap.
            lpShortPathW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                        ReturnValueW * sizeof(WCHAR)
                                        );
            if (lpShortPathW) {
                ReturnValueW = GetShortPathNameW(UString.Buffer, lpShortPathW, ReturnValueW);
                }
            else {
                ReturnValueW = 0;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }

        if (ReturnValueW)
            {
            // we are here because we have something interesting left to do.
            // Convert the UNICODE path name to ANSI(or OEM).
            UString.MaximumLength = (USHORT)((ReturnValueW + 1) * sizeof(WCHAR));
            UStringRet.Buffer = lpShortPathW;
            UStringRet.Length = (USHORT)(ReturnValueW * sizeof(WCHAR));
            Status = BasepUnicodeStringTo8BitString(&AString,
                                                    &UStringRet,
                                                    TRUE
                                                    );

            if (!NT_SUCCESS(Status))
                {
                BaseSetLastNTError(Status);
                ReturnValue=0;
                goto gspTryExit;
                }
            // now AString.Length contains the size of the converted path
            // name. If the caller provides enough buffer, copy the
            // path name.
            ReturnValue = AString.Length;
            if (ARGUMENT_PRESENT(lpShortPath) && cchBuffer > ReturnValue)
                {
                RtlMoveMemory(lpShortPath, AString.Buffer, ReturnValue);
                // terminate the string with NULL char
                lpShortPath[ReturnValue] = '\0';
                }
            else
                {
                // either the caller does not provide a buffer or
                // the provided buffer is too small return the required size,
                // including the terminated null char
                ReturnValue++;
                }
            }
gspTryExit:;
        }

    finally {
            if (UString.Buffer)
                RtlFreeUnicodeString(&UString);
            if (AString.Buffer)
                RtlFreeAnsiString(&AString);
            if (lpShortPathW && lpShortPathW != TempPathW)
                RtlFreeHeap(RtlProcessHeap(), 0, lpShortPathW);
        }
    return ReturnValue;
}
/****
GetShortPathName

Description:
    This function converts the given path name to its short form if
     needed. The conversion  may not be necessary and in that case,
     this function simply copies down the given name to the return buffer.
    The caller can have the return buffer set equal to the given path name
     address.

Parameters:
    lpszLongPath -  Points to a NULL terminated string.
    lpszShortPath - Buffer address to return the short name.
    cchBuffer - Buffer size in char of lpszShortPath.

Return Value
    If the GetShortPathName function succeeds, the return value is the length,
    in characters, of the string copied to lpszShortPath,
    not including the terminating
    null character.

    If the lpszShortPath is too small, the return value is
    the size of the buffer, in
    characters, required to hold the path.

    If the function fails, the return value is zero. To get
    extended error information, use
    the GetLastError function.

Remarks:
    The "short name" can be longer than its "long name". lpszLongPath doesn't
    have to be a fully qualified path name or a long path name.

****/

DWORD
APIENTRY
GetShortPathNameW(
    IN  LPCWSTR lpszLongPath,
    IN  LPWSTR  lpszShortPath,
    IN  DWORD   cchBuffer
    )
{

    LPCWSTR         pcs;
    LPWSTR          pSrcCopy, pSrc, pFirst, pLast, pDst;
    WCHAR           wch;
    HANDLE          FindHandle;
    WIN32_FIND_DATAW        FindData;
    LPWSTR          Buffer;
    DWORD           ReturnLen, Length;
    UINT PrevErrorMode;
    if (!ARGUMENT_PRESENT(lpszLongPath)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }

    //
    // override the error mode since we will be touching the media.
    // This is to prevent file system's pop-up when the given path does not
    // exist or the media is not available.
    // we are doing this because we can not depend on the caller's current
    // error mode. NOTE: the old error mode must be restored.
    PrevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    try {
        Buffer = NULL;
        pSrcCopy = NULL;
        // first, make sure the given path exist
        if (0xFFFFFFFF == GetFileAttributesW(lpszLongPath))
        {
            //
            // (bjm - 3/17/99)
            // This behavior (failing if the file does not exist) is new (to NT) with NT 5.  
            // (It's the Win9x behavior.)
            // Give an exception to Norton AntiVirus Uninstall.
            // If we fail this call, there will be a registry value left behind in VDD that'll cause
            // undeserved ugly messages for a user.  Norton AV Uninstall counts on NT 4 behavior
            // which did not care if the file existed to do this conversion.  This was changed in
            // NT 5.0 to match Win9x behavior.
            //
            if ( !NtCurrentPeb() || !(NtCurrentPeb()->AppCompatInfo) ||
                !(((PAPP_COMPAT_INFO)(NtCurrentPeb()->AppCompatInfo))->CompatibilityFlags.QuadPart & KACF_OLDGETSHORTPATHNAME ) )
            {
                // last error has been set by GetFileAttributes
                ReturnLen = 0;
                goto gsnTryExit;
            }
        }
        
        pcs = SkipPathTypeIndicator_U(lpszLongPath);
        if (!pcs || *pcs == UNICODE_NULL || !FindLFNorSFN_U((LPWSTR)pcs, &pFirst, &pLast, TRUE))
            {
            // nothing to convert, copy down the source string
            // to the buffer if necessary

            ReturnLen = wcslen(lpszLongPath);
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                if (lpszShortPath != lpszLongPath)
                    RtlMoveMemory(lpszShortPath, lpszLongPath,
                                  (ReturnLen + 1) * sizeof(WCHAR)
                                  );
                }
            else {
                // the caller does not provide enough buffer, return
                // necessary string length plus the terminated null char
                ReturnLen++;
                }
            goto gsnTryExit;
            }

        // conversions  are necessary, make a local copy of the string
        // because we have to party on it.

        ASSERT(!pSrcCopy);

        // get the source string length
        Length  = wcslen(lpszLongPath) + 1;

        pSrcCopy = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                   Length * sizeof(WCHAR)
                                   );
        if (!pSrcCopy) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto gsnTryExit;
            }
        wcsncpy(pSrcCopy, lpszLongPath, Length);
        // pFirst points to the first char of the very first LFN in the path
        // pLast points to the char right after the last char of the very
        // first LFN in the path. *pLast could be UNICODE_NULL
        pFirst = pSrcCopy + (pFirst - lpszLongPath);
        pLast = pSrcCopy  + (pLast - lpszLongPath);
        //
        // We allow lpszShortPath be overlapped with lpszLongPath so
        // allocate a local buffer.

        pDst = lpszShortPath;
        if (cchBuffer > 0 && ARGUMENT_PRESENT(lpszShortPath) &&
            (lpszShortPath >= lpszLongPath &&lpszShortPath < lpszLongPath + Length ||
             lpszShortPath < lpszLongPath && lpszShortPath + cchBuffer >= lpszLongPath))
            {
            ASSERT(!Buffer);

            Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                           cchBuffer * sizeof(WCHAR));
            if (!Buffer){
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto gsnTryExit;
            }
            pDst = Buffer;
        }

        pSrc = pSrcCopy;

        ReturnLen = 0;
        do {
            // there are three pointers involve in the conversion loop:
            // pSrc, pFirst and pLast. Their relationship
            // is:
            //
            // "c:\long~1.1\\foo.bar\\long~2.2\\bar"
            //  ^          ^          ^       ^
            //  |          |          |       |
            //  |          pSrc       pFirst  pLast
            //  pSrcCopy
            //
            // pSrcCopy always points to the very first char of the entire
            // path.
            //
            // chars between pSrc(included) and pFirst(not included)
            // do not need conversion so we simply copy them.
            // chars between pFirst(included) and pLast(not included)
            // need conversion.
            //
            Length = (ULONG)(pFirst - pSrc);
            if (Length) {
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath)) {
                    RtlMoveMemory(pDst, pSrc, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                }
            wch = *pLast;
            *pLast = UNICODE_NULL;
            FindHandle = FindFirstFileW(pSrcCopy, &FindData);
            *pLast = wch;
            if (INVALID_HANDLE_VALUE != FindHandle) {
                FindClose(FindHandle);
                // if no short name could be found, copy the original name.
                // the origian name starts with pFirst(included) and ends
                // with pLast(excluded).
                if (!(Length = wcslen(FindData.cAlternateFileName)))
                    Length = (ULONG)(pLast - pFirst);
                else
                    pFirst = FindData.cAlternateFileName;
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                    {
                    RtlMoveMemory(pDst, pFirst, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                 }
            else {
                // part of the path does not exist, fail the function
                //
                ReturnLen = 0;
                break;
                }
            // move to next path name
            pSrc = pLast;
            if (*pLast == UNICODE_NULL)
                break;
            }while (FindLFNorSFN_U(pSrc, &pFirst, &pLast, TRUE));

        // if ReturnLen == 0, we fail somewhere inside while loop.
        if (ReturnLen) {
            // (*pSrc == UNICODE_NULL) means the last pathname is a LFN which
            // has been dealt with. otherwise, the substring pointed by
            // pSrc is a legal short path name and we have to copy it
            //Length could be zero
            Length = wcslen(pSrc);
            ReturnLen += Length;
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                //include the terminated null char
                RtlMoveMemory(pDst, pSrc, (Length + 1)* sizeof(WCHAR));
                if (Buffer)
                    RtlMoveMemory(lpszShortPath, Buffer, (ReturnLen + 1) * sizeof(WCHAR));
                }
            else
                // not enough buffer, the return value counts the terminated NULL
                ReturnLen++;
            }
gsnTryExit:;
        }
    finally {
         if (Buffer)
            RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
         if (pSrcCopy)
            RtlFreeHeap(RtlProcessHeap(), 0, pSrcCopy);
        }

    // restore the error mode
    SetErrorMode(PrevErrorMode);

    return ReturnLen;
}

/**
   function to create VDM environment for the new executable.
   Input:   lpEnvironmen = optinal environment strings prototype in UNICODE.
                           If it is NULL, this function use the environment
                           block attached to the process
            pAStringEnv  = pointer to a ANSI_STRING to receive the
                           new environment strings.
            pUStringEnv  = pointer to a UNICODE_STRING to receive the
                           new environment strings.
    Output: FALSE if the creattion failed.
            TRUE  creation successful, pAStringEnv has been setup.

    This function was provided so that BaseCheckVdm can have correct
    environment(includes the newly create NTVDM process). This was done
    because before command.com gets the next command, users can have
    tons of things specified in config.sys and autoexec.bat which
    may rely on current directory of each drive.
**/
BOOL BaseCreateVDMEnvironment(
    PWCHAR lpEnvironment,
    ANSI_STRING * pAStringEnv,
    UNICODE_STRING  *pUStringEnv
    )
{
    WCHAR  *pEnv, *pDst, *EnvStrings,* pTmp, *pNewEnv;
    DWORD   cchEnv, dw, Length, dwRemain;
    NTSTATUS    Status;
    UINT        NameType;
    BOOL        bRet = FALSE;
    SIZE_T      EnvSize;

    if (!ARGUMENT_PRESENT(pAStringEnv) || !ARGUMENT_PRESENT(pUStringEnv)){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }
    try {
        // the environment strings are shared by every thread of the same
        // process. Since we have no idea of what the caller process
        // is, we have to grab the entire environment to our local buffer in one
        // shot then we can walk through the strings.
        // Note that if another thread makes call to RtlSetEnvironmentVariable
        // then we are out of sync. It is a problem of process structure and
        // I don't want to think about it now.
        // The funny thing is that we have to assume the environment
        // is a block of strings(otherwise, how can we do it?)t, nothing more and
        // nothing less. If someday and somebody dares to change it, he will be
        // the one to blame. If the caller(CreateProcess)
        // provides the environment, we assume it is safe to walk through it.
        //

        if (lpEnvironment == NULL) {
            // create a new environment and inherit the current process env
            Status = RtlCreateEnvironment(TRUE, (PVOID *)&EnvStrings);
            if (!NT_SUCCESS(Status))
                goto bveTryExit;
            }
        else
            EnvStrings = lpEnvironment;

        if (EnvStrings == NULL) {
            SetLastError(ERROR_BAD_ENVIRONMENT);
            goto bveTryExit;
            }
        // figure out how long the environment is
        // why can Rtl just provides such a function for us?
        //
        cchEnv = 0;
        pEnv = EnvStrings;
        // environment is double-null terminated
        while (!(*pEnv++ == UNICODE_NULL && *pEnv == UNICODE_NULL))
            cchEnv++;
        // count the last two NULLs
        cchEnv += 2;
        // we don't want to change the original environment, so
        // make a local buffer for it.
        EnvSize = (cchEnv + MAX_PATH) * sizeof(WCHAR);
        pNewEnv = NULL;
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pNewEnv,
                                          0,
                                          &EnvSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (!NT_SUCCESS(Status) ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto bveTryExit;
            }
        // give the last two for null
        dwRemain = MAX_PATH - 2;
        // now walk through the environment string
        pEnv = EnvStrings;
        // the new environmet will be
        pDst = pNewEnv;
        while (*pEnv != UNICODE_NULL) {
            // current directory environment has the form as:
            // "=d:=d:\pathname" where d: is the drive designator.
            if (pEnv[0] == L'=')
                {
                if ((pEnv[1] >= L'A' && pEnv[1] <= L'Z' || pEnv[1] >= L'a' && pEnv[1] <= L'z') &&
                     pEnv[2] == L':' && pEnv[3] == L'=' && wcslen(pEnv) >= 7)
                    {
                    // hack hack!!!!
                    // if the path points to the root directory,
                    // bypass the conversion. Dos or Wow keeps current directory
                    // for every valid drive. If we do the conversion for
                    // every current directory, it could take several
                    // seconds on removable drives, especially, on
                    // floppy drives.
                    if (pEnv[7] == UNICODE_NULL &&
                        (pEnv[6] == L'\\' || pEnv[6] == L'/') &&
                        pEnv[5] == L':' &&
                        (pEnv[4] >= L'A' && pEnv[4] <= L'Z' ||
                         pEnv[4] >= L'a' && pEnv[4] <= L'z'))
                        {
                        NameType = ENV_NAME_TYPE_NO_PATH;
                        }
                    else
                        {
                        // copy "=N:=", where N is the drive letter
                        *pDst++ = *pEnv++;*pDst++ = *pEnv++;
                        *pDst++ = *pEnv++;*pDst++ = *pEnv++;
                        NameType = ENV_NAME_TYPE_SINGLE_PATH;
                        }
                    }
                else {
                    // a weird environment was detected.
                    // treat it as no path
                    NameType = ENV_NAME_TYPE_NO_PATH;
                    }
                }
            else {
                pTmp = pEnv;
                // copy down the name and the '='
                while (*pEnv != UNICODE_NULL && (*pDst++ = *pEnv++) != L'=')
                    ;
                NameType = BaseGetEnvNameType_U(pTmp, (DWORD)(pEnv - pTmp) - 1);
                }

            if (NameType == ENV_NAME_TYPE_NO_PATH) {
                while ((*pDst++ = *pEnv++) != UNICODE_NULL)
                    ;
                }
            else if (NameType == ENV_NAME_TYPE_SINGLE_PATH) {
                    Length = wcslen(pEnv) + 1;
                    dw = GetShortPathNameW(pEnv, pDst, Length + dwRemain);
                    // if the conversion failed, we simply pass down the original
                    // one no matter what the reason is. This is done because we
                    // are doing the environment strings.
                    if (dw == 0 || dw >= Length + dwRemain){
                        RtlMoveMemory(pDst, pEnv, Length * sizeof(WCHAR));
                        dw = Length - 1;
                        }
                    pDst += dw + 1;
                    pEnv += Length;
                    if (dw > Length)
                        dwRemain -= dw - Length;
                    }
                 else {
                    // multiple path name found.
                    // the character ';' is used for seperator
                     pTmp = pEnv;
                     while(*pEnv != UNICODE_NULL) {
                        if (*pEnv == L';') {
                            // length not include the ';'
                            Length = (DWORD)(pEnv - pTmp);
                            if (Length > 0) {
                                *pEnv = UNICODE_NULL;
                                dw = GetShortPathNameW(pTmp, pDst, Length + 1 + dwRemain);
                                // again, if the conversion failed, use the original one
                                if (dw == 0 || dw > Length + dwRemain) {
                                    RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
                                    dw = Length;
                                    }
                                pDst += dw;
                                *pDst++ = *pEnv++ = L';';
                                if (dw > Length)
                                    dwRemain -= dw - Length;
                                }
                             // skip all consecutive ';'
                             while (*pEnv == L';')
                                *pDst++ = *pEnv++;
                             pTmp = pEnv;
                             }
                        else
                            pEnv++;
                        }
                    // convert the last one
                    if ((Length = (DWORD)(pEnv - pTmp)) != 0) {
                        dw = GetShortPathNameW(pTmp, pDst, Length+1 + dwRemain);
                        if (dw == 0 || dw > Length) {
                            RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
                            dw = Length;
                            }
                        pDst += dw;
                        if (dw > Length)
                            dwRemain -= dw - Length;
                        }
                    *pDst++ = *pEnv++;
                    }
            }
        *pDst++ = UNICODE_NULL;
        cchEnv = (ULONG)((ULONG_PTR)pDst - (ULONG_PTR)pNewEnv);
        pUStringEnv->MaximumLength = pUStringEnv->Length = (USHORT)cchEnv;
        pUStringEnv->Buffer = pNewEnv;
        Status = RtlUnicodeStringToAnsiString(pAStringEnv,
                                              pUStringEnv,
                                              TRUE
                                              );

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            }
        else
            bRet = TRUE;
bveTryExit:;
        }
    finally {
         if (lpEnvironment == NULL && EnvStrings != NULL)
            RtlDestroyEnvironment(EnvStrings);
        }
    return bRet;
}
/**
    Destroy the environment block created by BaseCreateVDMEnvironment
    Input: ANSI_STRING * pAnsiStringVDMEnv
                      Environment block in ANSI, should be freed via
                      RtlFreeAnsiString
           UNICODE_STRING * pUnicodeStringEnv
                      Environment block in UNICODE. The Buffer should
                      be freed with RtlFreeHeap.
    Output: should always be TRUE.

**/

BOOL
BaseDestroyVDMEnvironment(
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
    )
{
    if (pAStringEnv->Buffer)
        RtlFreeAnsiString(pAStringEnv);
    if (pUStringEnv->Buffer) {
        NTSTATUS Status;
        SIZE_T RegionSize;

        //
        // Free the specified environment variable block.
        //

        RegionSize = 0;
        Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                      &pUStringEnv->Buffer,
                                      &RegionSize,
                                      MEM_RELEASE
                                    );
    }
    return TRUE;

}

/**
    This function returns the name type of the given environment variable name
    The name type has three possibilities. Each one represents if the
    given name can have pathnames as its value.
     ENV_NAME_TYPE_NO_PATH:   no pathname can be its value
     ENV_NAME_TYPE_SINGLE_PATH: single pathname
     ENV_NAME_MULTIPLE_PATH: multiple path


    SIDE NOTE:
        Currently, nt can not installed on a long path and it seems
        that systemroot and windir are never be in long path.

**/
UINT
BaseGetEnvNameType_U(WCHAR * Name, DWORD NameLength)
{


// so far we only take care of five predefined names:
// PATH
// WINDIR and
// SYSTEMROOT.
// TEMP
// TMP
//
static ENV_INFO     EnvInfoTable[STD_ENV_NAME_COUNT] = {
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_PATH_LEN, ENV_NAME_PATH},
    {ENV_NAME_TYPE_SINGLE_PATH, ENV_NAME_WINDIR_LEN, ENV_NAME_WINDIR},
    {ENV_NAME_TYPE_SINGLE_PATH, ENV_NAME_SYSTEMROOT_LEN, ENV_NAME_SYSTEMROOT},
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_TEMP_LEN, ENV_NAME_TEMP},
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_TMP_LEN, ENV_NAME_TMP}
    };



   UINT NameType;
   int  i;


    NameType = ENV_NAME_TYPE_NO_PATH;
    for (i = 0; i < STD_ENV_NAME_COUNT; i++) {
        if (EnvInfoTable[i].NameLength == NameLength &&
            !_wcsnicmp(EnvInfoTable[i].Name, Name, NameLength)) {
            NameType = EnvInfoTable[i].NameType;
            break;
            }
        }
    return NameType;
}


DWORD
APIENTRY
GetLongPathNameA(
    IN  LPCSTR  lpszShortPath,
    IN  LPSTR   lpLongPath,
    IN  DWORD   cchBuffer
    )
{
    UNICODE_STRING  UString, UStringRet;
    ANSI_STRING     AString;
    NTSTATUS        Status;
    LPWSTR          lpLongPathW;
    WCHAR           TempPathW[MAX_PATH];
    DWORD           ReturnValue, ReturnValueW;


    if (lpszShortPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }

    AString.Buffer = NULL;
    UString.Buffer = NULL;

    ReturnValue = 0;
    ReturnValueW = 0;

    try {
        if (!Basep8BitStringToDynamicUnicodeString(&UString, lpszShortPath )) {
            goto glpTryExit;
            }

        // we have to get the real converted path in order to find out
        // the required length. An UNICODE char does not necessarily convert
        // to one ANSI char(A DBCS is basically TWO ANSI char!!!!!).
        // First, we use the buffer allocated from the stack. If the buffer
        // is too small, we then allocate it from heap.
        // A check of (lpLongPathW && TempPathW != lpLongPathW) will reveal
        // if we have allocated a buffer from heap and need to release it.
        lpLongPathW = TempPathW;
        ReturnValueW = GetLongPathNameW(UString.Buffer, lpLongPathW, sizeof(TempPathW) / sizeof(WCHAR));
        if (ReturnValueW >= sizeof(TempPathW) / sizeof(WCHAR))
            {
            // the stack-based buffer is too small. Allocate a new buffer
            // from heap.
            lpLongPathW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                        ReturnValueW * sizeof(WCHAR)
                                        );
            if (lpLongPathW) {
                ReturnValueW = GetLongPathNameW(UString.Buffer, lpLongPathW, ReturnValueW);
                }
            else {
                ReturnValueW = 0;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }

        if (ReturnValueW)
            {
            // we are here because we have something interesting left to do.
            // Convert the UNICODE path name to ANSI(or OEM).
            UString.MaximumLength = (USHORT)((ReturnValueW + 1) * sizeof(WCHAR));
            UStringRet.Buffer = lpLongPathW;
            UStringRet.Length = (USHORT)(ReturnValueW * sizeof(WCHAR));
            Status = BasepUnicodeStringTo8BitString(&AString,
                                                    &UStringRet,
                                                    TRUE
                                                    );

            if (!NT_SUCCESS(Status))
                {
                BaseSetLastNTError(Status);
                ReturnValue=0;
                goto glpTryExit;
                }
            // now AString.Length contains the size of the converted path
            // name. If the caller provides enough buffer, copy the
            // path name.
            ReturnValue = AString.Length;
            if (ARGUMENT_PRESENT(lpLongPath) && cchBuffer > ReturnValue)
                {
                RtlMoveMemory(lpLongPath, AString.Buffer, ReturnValue);
                // terminate the buffer with NULL char.
                lpLongPath[ReturnValue] = '\0';
                }
            else
                {
                // either the caller does not provide a buffer or
                // the provided buffer is too small, return the required size,
                // including the terminated null char.
                ReturnValue++;
                }
            }
glpTryExit:;
        }

    finally {
            if (UString.Buffer)
                RtlFreeUnicodeString(&UString);
            if (AString.Buffer)
                RtlFreeAnsiString(&AString);
            if (lpLongPathW && lpLongPathW != TempPathW)
                RtlFreeHeap(RtlProcessHeap(), 0, lpLongPathW);
        }
    return ReturnValue;
}

DWORD
APIENTRY
GetLongPathNameW(
    IN  LPCWSTR lpszShortPath,
    IN  LPWSTR  lpszLongPath,
    IN  DWORD   cchBuffer
)
{

    LPCWSTR pcs;
    DWORD ReturnLen, Length;
    LPWSTR pSrc, pSrcCopy, pFirst, pLast, Buffer, pDst;
    WCHAR   wch;
    HANDLE          FindHandle;
    WIN32_FIND_DATAW        FindData;
    UINT PrevErrorMode;

    if (!ARGUMENT_PRESENT(lpszShortPath)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }
    //
    // override the error mode since we will be touching the media.
    // This is to prevent file system's pop-up when the given path does not
    // exist or the media is not available.
    // we are doing this because we can not depend on the caller's current
    // error mode. NOTE: the old error mode must be restored.
    PrevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    try {

        Buffer = NULL;
        pSrcCopy = NULL;
        // first make sure the given path exist.
        //
        if (0xFFFFFFFF == GetFileAttributesW(lpszShortPath))
        {
            // last error has been set by GetFileAttributes
            ReturnLen = 0;
            goto glnTryExit;
        }
        pcs = SkipPathTypeIndicator_U(lpszShortPath);
        if (!pcs || *pcs == UNICODE_NULL || !FindLFNorSFN_U((LPWSTR)pcs, &pFirst, &pLast, FALSE))
            {
            // The path is ok and does not need conversion at all.
            // Check if we need to do copy
            ReturnLen = wcslen(lpszShortPath);
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                {
                if (lpszLongPath != lpszShortPath)
                    RtlMoveMemory(lpszLongPath, lpszShortPath,
                                      (ReturnLen + 1)* sizeof(WCHAR)
                                      );
                }
            else {
                // No buffer or buffer too small, the return size
                // has to count the terminated NULL char
                ReturnLen++;
                }
            goto glnTryExit;
            }


        // conversions  are necessary, make a local copy of the string
        // because we have to party on it.

        ASSERT(!pSrcCopy);

        Length = wcslen(lpszShortPath) + 1;
        pSrcCopy = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                   Length * sizeof(WCHAR)
                                   );
        if (!pSrcCopy) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto glnTryExit;
            }
        RtlMoveMemory(pSrcCopy, lpszShortPath, Length * sizeof(WCHAR));
        // pFirst points to the first char of the very first SFN in the path
        // pLast points to the char right after the last char of the very
        // first SFN in the path. *pLast could be UNICODE_NULL
        pFirst = pSrcCopy + (pFirst - lpszShortPath);
        pLast = pSrcCopy + (pLast - lpszShortPath);
        //
        // We allow lpszShortPath be overlapped with lpszLongPath so
        // allocate a local buffer if necessary:
        // (1) the caller does provide a legitimate buffer and
        // (2) the buffer overlaps with lpszShortName

        pDst = lpszLongPath;
        if (cchBuffer && ARGUMENT_PRESENT(lpszLongPath) &&
            (lpszLongPath >= lpszShortPath && lpszLongPath < lpszShortPath + Length ||
             lpszLongPath < lpszShortPath && lpszLongPath + cchBuffer >= lpszShortPath))
            {
            ASSERT(!Buffer);

            Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                           cchBuffer * sizeof(WCHAR));
            if (!Buffer){
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto glnTryExit;
                }
            pDst = Buffer;
            }

        pSrc = pSrcCopy;
        ReturnLen = 0;
        do {
            // there are three pointers involve in the conversion loop:
            // pSrc, pFirst and pLast. Their relationship
            // is:
            //
            // "c:\long~1.1\\foo.bar\\long~2.2\\bar"
            //  ^          ^          ^       ^
            //  |          |          |       |
            //  |          pSrc       pFirst  pLast
            //  pSrcCopy
            //
            // pSrcCopy always points to the very first char of the entire
            // path.
            //
            // chars between pSrc(included) and pFirst(not included)
            // do not need conversion so we simply copy them.
            // chars between pFirst(included) and pLast(not included)
            // need conversion.
            //
            Length = (ULONG)(pFirst - pSrc);
            ReturnLen += Length;
            if (Length && cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                RtlMoveMemory(pDst, pSrc, Length * sizeof(WCHAR));
                pDst += Length;
                }
            // now try to convert the name, chars between pFirst and (pLast - 1)
            wch = *pLast;
            *pLast = UNICODE_NULL;
            FindHandle = FindFirstFileW(pSrcCopy, &FindData);
            *pLast = wch;
            if (FindHandle != INVALID_HANDLE_VALUE){
                FindClose(FindHandle);
                // if no long name, copy the original name
                // starts with pFirst(included) and ends with pLast(excluded)
                if (!(Length = wcslen(FindData.cFileName)))
                    Length = (ULONG)(pLast - pFirst);
                else
                    pFirst = FindData.cFileName;
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                    {
                    RtlMoveMemory(pDst, pFirst, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                }
            else {
                // invalid path, reset the length, mark the error and
                // bail out of the loop. We will be copying the source
                // to destination later.
                //
                ReturnLen = 0;
                break;
                }
            pSrc = pLast;
            if (*pSrc == UNICODE_NULL)
                break;
            } while (FindLFNorSFN_U(pSrc, &pFirst, &pLast, FALSE));

        if (ReturnLen) {
            //copy the rest of the path from pSrc. This may only contain
            //a single NULL char
            Length = wcslen(pSrc);
            ReturnLen += Length;
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                {
                RtlMoveMemory(pDst, pSrc, (Length + 1) * sizeof(WCHAR));
                if (Buffer)
                    RtlMoveMemory(lpszLongPath, Buffer, (ReturnLen + 1) * sizeof(WCHAR));
                }
            else
                ReturnLen++;
            }

glnTryExit:
        ;
        }
        finally {
            if (pSrcCopy)
                RtlFreeHeap(RtlProcessHeap(), 0, pSrcCopy);
            if (Buffer)
                RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
            }

    // restore error mode.
    SetErrorMode(PrevErrorMode);
    return ReturnLen;
}

/**
    Search for SFN(Short File Name) or LFN(Long File Name) in the
    given path depends on FindLFN.

    Input: LPWSTR Path
                The given path name. Does not have to be fully qualified.
                However, path type separaters are not allowed.
           LPWSTR* ppFirst
                To return the pointer points to the first char
                of the name found.
           LPWSTR* ppLast
                To return the pointer points the char right after
                the last char of the name found.
           BOOL FindLFN
                TRUE to search for LFN, otherwise, search for SFN

    Output:
            TRUE
                if the target file name type is found, ppFirst and
                ppLast are filled with pointers.
            FALSE
                if the target file name type not found.

    Remark: "\\." and "\\.." are special cases. When encountered, they
            are ignored and the function continue to search


**/
BOOL
FindLFNorSFN_U(
    LPWSTR  Path,
    LPWSTR* ppFirst,
    LPWSTR* ppLast,
    BOOL    FindLFN
    )
{
    LPWSTR pFirst, pLast;
    BOOL TargetFound;

    ASSERT(Path);

    pFirst = Path;

    TargetFound = FALSE;

    while(TRUE) {
        //skip over leading path separator
        // it is legal to have multiple path separators in between
        // name such as "foobar\\\\\\multiplepathchar"
        while (*pFirst != UNICODE_NULL  && (*pFirst == L'\\' || *pFirst == L'/'))
            pFirst++;
        if (*pFirst == UNICODE_NULL)
            break;
        pLast = pFirst + 1;
        while (*pLast != UNICODE_NULL && *pLast != L'\\' && *pLast != L'/')
            pLast++;
        if (FindLFN)
            TargetFound = !IsShortName_U(pFirst, (int)(pLast - pFirst));
        else
            TargetFound = !IsLongName_U(pFirst, (int)(pLast - pFirst));
        if (TargetFound) {
            if(ppFirst && ppLast) {
                *ppFirst = pFirst;
                // pLast point to the last char of the path/file name
                *ppLast = pLast;
                }
            break;
            }
        if (*pLast == UNICODE_NULL)
            break;
        pFirst = pLast + 1;
        }
    return TargetFound;
}
LPCWSTR
SkipPathTypeIndicator_U(
    LPCWSTR Path
    )
{
    RTL_PATH_TYPE   RtlPathType;
    LPCWSTR         pFirst;
    DWORD           Count;

    RtlPathType = RtlDetermineDosPathNameType_U(Path);
    switch (RtlPathType) {
        // form: "\\server_name\share_name\rest_of_the_path"
        case RtlPathTypeUncAbsolute:
            pFirst = Path + 2;
            Count = 2;
            // guard for UNICODE_NULL is necessary because
            // RtlDetermineDosPathNameType_U doesn't really
            // verify an UNC name.
            while (Count && *pFirst != UNICODE_NULL) {
                if (*pFirst == L'\\' || *pFirst == L'/')
                    Count--;
                pFirst++;
                }
            break;

        // form: "\\.\rest_of_the_path"
        case RtlPathTypeLocalDevice:
            pFirst = Path + 4;
            break;

        // form: "\\."
        case RtlPathTypeRootLocalDevice:
            pFirst = NULL;
            break;

        // form: "D:\rest_of_the_path"
        case RtlPathTypeDriveAbsolute:
            pFirst = Path + 3;
            break;

        // form: "D:rest_of_the_path"
        case RtlPathTypeDriveRelative:
            pFirst = Path + 2;
            break;

        // form: "\rest_of_the_path"
        case RtlPathTypeRooted:
            pFirst = Path + 1;
            break;

        // form: "rest_of_the_path"
        case RtlPathTypeRelative:
            pFirst = Path;
            break;

        default:
            pFirst = NULL;
            break;
        }
    return pFirst;
}

/**
    This function determines if the given name is a valid short name.
    This function only does "obvious" testing since there are not precise
    ways to cover all the file systems(each file system has its own
    file name domain(for example, FAT allows all extended chars and space char
    while NTFS **may** not).
    The main purpose is to help the caller decide if a long to short name
    conversion is necessary. When in doubt, this function simply tells the
    caller that the given name is NOT a short name so that caller would
    do whatever it takes to convert the name.
    This function applies strict rules in deciding if the given name
    is a valid short name. For example, a name containing any extended chars
    is treated as invalid; a name with embedded space chars is also treated
    as invalid.
    A name is a valid short name if ALL the following conditions are met:
    (1). total length <= 13.
    (2). 0 < base name length <= 8.
    (3). extention name length <= 3.
    (4). only one '.' is allowed and must not be the first char.
    (5). every char must be legal defined by the IllegalMask array.

    null path, "." and ".." are treated valid.

    Input: LPCWSTR Name -  points to the name to be checked. It does not
                           have to be NULL terminated.

           int Length - Length of the name, not including teminated NULL char.

    output: TRUE - if the given name is a short file name.
            FALSE - if the given name is not a short file name
**/

// bit set -> char is illegal
DWORD   IllegalMask[] =

{
    // code 0x00 - 0x1F --> all illegal
    0xFFFFFFFF,
    // code 0x20 - 0x3f --> 0x20,0x22,0x2A-0x2C,0x2F and 0x3A-0x3F are illegal
    0xFC009C05,
    // code 0x40 - 0x5F --> 0x5B-0x5D are illegal
    0x38000000,
    // code 0x60 - 0x7F --> 0x7C is illegal
    0x10000000
};

BOOL
IsShortName_U(
    LPCWSTR Name,
    int     Length
    )
{
    int Index;
    BOOL ExtensionFound;
    DWORD      dwStatus;
    UNICODE_STRING UnicodeName;
    ANSI_STRING AnsiString;
    UCHAR      AnsiBuffer[MAX_PATH];
    UCHAR      Char;

    ASSERT(Name);

    // total length must less than 13(8.3 = 8 + 1 + 3 = 12)
    if (Length > 12)
        return FALSE;
    //  "" or "." or ".."
    if (!Length)
        return TRUE;
    if (L'.' == *Name)
    {
        // "." or ".."
        if (1 == Length || (2 == Length && L'.' == Name[1]))
            return TRUE;
        else
            // '.' can not be the first char(base name length is 0)
            return FALSE;
    }

    UnicodeName.Buffer = (LPWSTR)Name;
    UnicodeName.Length =
    UnicodeName.MaximumLength = (USHORT)(Length * sizeof(WCHAR));

    AnsiString.Buffer = AnsiBuffer;
    AnsiString.Length = 0;
    AnsiString.MaximumLength = MAX_PATH; // make a dangerous assumption

    dwStatus = BasepUnicodeStringTo8BitString(&AnsiString,
                                              &UnicodeName,
                                              FALSE);
    if (! NT_SUCCESS(dwStatus)) {
         return(FALSE);
    }

    // all trivial cases are tested, now we have to walk through the name
    ExtensionFound = FALSE;
    for (Index = 0; Index < AnsiString.Length; Index++)
    {
        Char = AnsiString.Buffer[Index];

        // Skip over and Dbcs characters
        if (IsDBCSLeadByte(Char)) {
            //
            //  1) if we're looking at base part ( !ExtensionPresent ) and the 8th byte
            //     is in the dbcs leading byte range, it's error ( Index == 7 ). If the
            //     length of base part is more than 8 ( Index > 7 ), it's definitely error.
            //
            //  2) if the last byte ( Index == DbcsName.Length - 1 ) is in the dbcs leading
            //     byte range, it's error
            //
            if ((!ExtensionFound && (Index >= 7)) ||
                (Index == AnsiString.Length - 1)) {
                return FALSE;
            }
            Index += 1;
            continue;
        }

        // make sure the char is legal
        if (Char > 0x7F || IllegalMask[Char / 32] & (1 << (Char % 32)))
            return FALSE;

        if ('.' == Char)
        {
            // (1) can have only one '.'
            // (2) can not have more than 3 chars following.
            if (ExtensionFound || Length - (Index + 1) > 3)
            {
                return FALSE;
            }
            ExtensionFound = TRUE;
        }
        // base length > 8 chars
        if (Index >= 8 && !ExtensionFound)
            return FALSE;
    }
    return TRUE;

}

/**
    This function determines if the given name is a valid long name.
    This function only does "obvious" testing since there are not precise
    ways to cover all the file systems(each file system has its own
    file name domain(for example, FAT allows all extended chars and space char
    while NTFS **may** not)
    This function helps the caller to determine if a short to long name
    conversion is necessary. When in doubt, this function simply tells the
    caller that the given name is NOT a long name so that caller would
    do whatever it takes to convert the name.
    A name is a valid long name if one of the following conditions is met:
    (1). total length >= 13.
    (2). 0 == base name length ||  base name length > 8.
    (3). extention name length > 3.
    (4). '.' is the first char.
    (5). muitlple '.'


    null path, "." and ".." are treat as valid long name.

    Input: LPCWSTR Name -  points to the name to be checked. It does not
                           have to be NULL terminated.

           int Length - Length of the name, not including teminated NULL char.

    output: TRUE - if the given name is a long file name.
            FALSE - if the given name is not a long file name
**/


BOOL
IsLongName_U(
    LPCWSTR Name,
    int Length
    )
{
    int Index;
    BOOL ExtensionFound;
    // (1) NULL path
    // (2) total length > 12
    // (3) . is the first char (cover "." and "..")
    if (!Length || Length > 12 || L'.' == *Name)
        return TRUE;
    ExtensionFound = FALSE;
    for (Index = 0; Index < Length; Index++)
    {
        if (L'.' == Name[Index])
        {
            // multiple . or extension longer than 3
            if (ExtensionFound || Length - (Index + 1) > 3)
                return TRUE;
            ExtensionFound = TRUE;
        }
        // base length longer than 8
        if (Index >= 8 && !ExtensionFound)
            return TRUE;
    }
    return FALSE;
}
