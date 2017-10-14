/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dialogs.h

Abstract:

    This module contains the definitions for the console dialog boxes

Author:

    Therese Stowell (thereses) Feb-3-1992 (swiped from Win3.1)

Revision History:

--*/

#define DID_SETTINGS                100
#define IDD_WINDOWED                101
#define IDD_FULLSCREEN              102
#define IDD_QUICKEDIT               103
#define IDD_INSERT                  104
#define IDD_CURSOR_SMALL            105
#define IDD_CURSOR_MEDIUM           106
#define IDD_CURSOR_LARGE            107
#define IDD_HISTORY_SIZE            108
#define IDD_HISTORY_SIZESCROLL      109
#define IDD_HISTORY_NUM             110
#define IDD_HISTORY_NUMSCROLL       111
#define IDD_HISTORY_NODUP           112
#if defined(FE_SB)
#define IDD_LANGUAGELIST            113
// v-HirShi Nov.2.1996
#define DID_SETTINGS2               114
#define IDD_LANGUAGE                115
#define IDD_LANGUAGE_GROUPBOX       116
#endif
#define IDD_DISPLAY_GROUPBOX        117

#define DID_FONTDLG                 200
#define IDD_STATIC                  201
#define IDD_FACENAME                202
#define IDD_BOLDFONT                203
#define IDD_STATIC2                 204
#define IDD_PREVIEWLABEL            206
#define IDD_GROUP                   207
#define IDD_STATIC3                 208
#define IDD_STATIC4                 209
#define IDD_FONTWIDTH               210
#define IDD_FONTHEIGHT              211
#define IDD_FONTSIZE                212
#define IDD_POINTSLIST              213
#define IDD_PIXELSLIST              214
#define IDD_PREVIEWWINDOW           215
#define IDD_FONTWINDOW              216

#define DID_SCRBUFSIZE              300
#define IDD_SCRBUF_WIDTH            301
#define IDD_SCRBUF_WIDTHSCROLL      302
#define IDD_SCRBUF_HEIGHT           303
#define IDD_SCRBUF_HEIGHTSCROLL     304
#define IDD_WINDOW_WIDTH            305
#define IDD_WINDOW_WIDTHSCROLL      306
#define IDD_WINDOW_HEIGHT           307
#define IDD_WINDOW_HEIGHTSCROLL     308
#define IDD_WINDOW_POSX             309
#define IDD_WINDOW_POSXSCROLL       310
#define IDD_WINDOW_POSY             311
#define IDD_WINDOW_POSYSCROLL       312
#define IDD_AUTO_POSITION           313
#define IDD_WARNING                 314

#define DID_COLOR                   400
#define IDD_COLOR_SCREEN_TEXT       401
#define IDD_COLOR_SCREEN_BKGND      402
#define IDD_COLOR_POPUP_TEXT        403
#define IDD_COLOR_POPUP_BKGND       404
#define IDD_COLOR_1                 405
#define IDD_COLOR_2                 406
#define IDD_COLOR_3                 407
#define IDD_COLOR_4                 408
#define IDD_COLOR_5                 409
#define IDD_COLOR_6                 410
#define IDD_COLOR_7                 411
#define IDD_COLOR_8                 412
#define IDD_COLOR_9                 413
#define IDD_COLOR_10                414
#define IDD_COLOR_11                415
#define IDD_COLOR_12                416
#define IDD_COLOR_13                417
#define IDD_COLOR_14                418
#define IDD_COLOR_15                419
#define IDD_COLOR_16                420
#define IDD_COLOR_SCREEN_COLORS     421
#define IDD_COLOR_POPUP_COLORS      422
#define IDD_COLOR_RED               423
#define IDD_COLOR_REDSCROLL         424
#define IDD_COLOR_GREEN             425
#define IDD_COLOR_GREENSCROLL       426
#define IDD_COLOR_BLUE              427
#define IDD_COLOR_BLUESCROLL        428

#define DID_SAVE_QUERY              500
#define DID_SAVE_QUERY_LINK         501
#define IDD_APPLY                   502
#define IDD_SAVE                    503
