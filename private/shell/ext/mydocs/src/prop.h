#ifndef __props_h
#define __props_h

BOOL CALLBACK TargetDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
#define MAX_NAME_LEN 20
#define FILENAME_LEN_WITH_SLASH_AND_NULL 14
#define MAX_DIR_PATH (MAX_PATH-FILENAME_LEN_WITH_SLASH_AND_NULL)

#ifndef IDH_MYDOCS_TARGET
#define IDH_MYDOCS_TARGET 1101
#endif

#ifndef IDH_MYDOCS_BROWSE
#define IDH_MYDOCS_BROWSE 1102
#endif

#ifndef IDH_MYDOCS_FIND_TARGET
#define IDH_MYDOCS_FIND_TARGET 1103
#endif

#endif
