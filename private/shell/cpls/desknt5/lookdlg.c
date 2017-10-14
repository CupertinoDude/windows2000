/*  LOOKDLG.C
**
**  Copyright (C) Microsoft, 1993, All Rights Reserved.
**
**
**  History:
**
*/

#include "precomp.h"
#pragma hdrstop


#ifndef COLOR_HOTLIGHT
    #define COLOR_HOTLIGHT              26
    #define COLOR_GRADIENTACTIVECAPTION     27
    #define COLOR_GRADIENTINACTIVECAPTION   28
    #undef COLOR_MAX
    #define COLOR_MAX                   (COLOR_GRADIENTINACTIVECAPTION+1)
#endif


#define COLOR_MAX_40                (COLOR_INFOBK+1)
#define COLOR_MAX_41                (COLOR_GRADIENTINACTIVECAPTION+1)

#undef  COLOR_MAX
#define COLOR_MAX COLOR_MAX_41

#define RGB_PALETTE 0x02000000


void FAR SetMagicColors(HDC, DWORD, WORD);

#define CENTRIES_3D 3

int g_i3DShadowAdj      = -333;
int g_i3DHilightAdj     = 500;
int g_iWatermarkAdj     = -50;

BOOL g_fScale3DShadowAdj  = TRUE;
BOOL g_fScale3DHilightAdj = TRUE;
BOOL g_fScaleWatermarkAdj = TRUE;

HPALETTE g_hpal3D = NULL;               // only exist if palette device
HPALETTE g_hpalVGA = NULL;              // only exist if palette device
BOOL g_bPalette = FALSE;                // is this a palette device?
int cyBorder;
int cxBorder;
int cxEdge;
int cyEdge;

// The following are defined in winnls.h only for _WIN32_WINNT >=0x500
// We need these for win9x versions also!
#ifndef WINNT
WINBASEAPI
LANGID
WINAPI
GetSystemDefaultUILanguage(void);

WINBASEAPI
LANGID
WINAPI
GetUserDefaultUILanguage(void);
#endif /* WINNT */


#define MAX_CHARSETS    4
// The following array will hold the "unique" Charsets corresponding to System Locale, 
// User Locale, System UI lang and User UI Lang. Note: Only unique charsets are kept
// here. So, the variable g_iCountCharsets contains the number of valid items in this array.
UINT    g_uiUniqueCharsets[MAX_CHARSETS] = { DEFAULT_CHARSET, DEFAULT_CHARSET, DEFAULT_CHARSET, DEFAULT_CHARSET };
// The following variable will hold the number of charsets stored in the above array.
int     g_iCountCharsets = 0; //Minimum value is 1; Maximum is 4
// The following are the indices into the above array.
#define SYSTEM_LOCALE_CHARSET  0  //The first item in the array is always system locale charset.

LOOK_FONT g_fonts[NUM_FONTS];
COLORREF g_rgb[COLOR_MAX];
HBRUSH g_brushes[COLOR_MAX];

HBITMAP g_hbmGradient = NULL;

BOOL g_bInit = TRUE;
BOOL g_bCrappyColor = FALSE;

BOOL g_fProprtySheetExiting = FALSE;

//This value changes whenever user changes Font size thro "General" tab.
int  g_iNewDPI   = 0;
int  g_iAppliedNewDPI = 0;       // A new DPI value that is already applied to the font sizes.
BOOL g_fAppliedDPIchanged = FALSE; //Whether DPI value changed after it was saved.

#define METRIC_CHANGE 0x0001
#define COLOR_CHANGE  0x0002
#define DPI_CHANGE    0x0004
#define SCHEME_CHANGE 0x8000
UINT g_fChanged;

LOOK_SIZE g_sizes[NUM_SIZES] = {
/* SIZE_FRAME */        {0, 0, 50},
/* SIZE_SCROLL */       {0, 8, 100},
/* SIZE_CAPTION */      {0, 8, 100},
/* SIZE_SMCAPTION */    {0, 4, 100},
/* SIZE_MENU */         {0, 8, 100},
/* SIZE_DXICON */       {0, 0, 150},    // x spacing
/* SIZE_DYICON */       {0, 0, 150},    // y spacing
/* SIZE_ICON */         {0, 16, 72},    // shell icon size
/* SIZE_SMICON */       {0, 8, 36},     // shell small icon size
};

LOOK_SIZE g_elCurSize;

#define COLORFLAG_SOLID 0x0001
UINT g_colorFlags[COLOR_MAX] = {
/* COLOR_SCROLLBAR           */ 0,
/* COLOR_DESKTOP             */ 0,
/* COLOR_ACTIVECAPTION       */ COLORFLAG_SOLID,
/* COLOR_INACTIVECAPTION     */ COLORFLAG_SOLID,
/* COLOR_MENU                */ COLORFLAG_SOLID,
/* COLOR_WINDOW              */ COLORFLAG_SOLID,
/* COLOR_WINDOWFRAME         */ COLORFLAG_SOLID,
/* COLOR_MENUTEXT            */ COLORFLAG_SOLID,
/* COLOR_WINDOWTEXT          */ COLORFLAG_SOLID,
/* COLOR_CAPTIONTEXT         */ COLORFLAG_SOLID,
/* COLOR_ACTIVEBORDER        */ 0,
/* COLOR_INACTIVEBORDER      */ 0,
/* COLOR_APPWORKSPACE        */ 0,
/* COLOR_HIGHLIGHT           */ COLORFLAG_SOLID,
/* COLOR_HIGHLIGHTTEXT       */ COLORFLAG_SOLID,
/* COLOR_3DFACE              */ COLORFLAG_SOLID,
/* COLOR_3DSHADOW            */ COLORFLAG_SOLID,
/* COLOR_GRAYTEXT            */ COLORFLAG_SOLID,
/* COLOR_BTNTEXT             */ COLORFLAG_SOLID,
/* COLOR_INACTIVECAPTIONTEXT */ COLORFLAG_SOLID,
/* COLOR_3DHILIGHT           */ COLORFLAG_SOLID,
/* COLOR_3DDKSHADOW          */ COLORFLAG_SOLID,
/* COLOR_3DLIGHT             */ COLORFLAG_SOLID,
/* COLOR_INFOTEXT            */ COLORFLAG_SOLID,
/* COLOR_INFOBK              */ 0,
/* COLOR_3DALTFACE           */ 0,
/* COLOR_HOTLIGHT            */ COLORFLAG_SOLID,
/* COLOR_GRADIENTACTIVECAPTION   */ COLORFLAG_SOLID,
/* COLOR_GRADIENTINACTIVECAPTION */ COLORFLAG_SOLID
};

// strings for color names.
PTSTR s_pszColorNames[COLOR_MAX] = {
/* COLOR_SCROLLBAR           */ TEXT("Scrollbar"),
/* COLOR_DESKTOP             */ TEXT("Background"),
/* COLOR_ACTIVECAPTION       */ TEXT("ActiveTitle"),
/* COLOR_INACTIVECAPTION     */ TEXT("InactiveTitle"),
/* COLOR_MENU                */ TEXT("Menu"),
/* COLOR_WINDOW              */ TEXT("Window"),
/* COLOR_WINDOWFRAME         */ TEXT("WindowFrame"),
/* COLOR_MENUTEXT            */ TEXT("MenuText"),
/* COLOR_WINDOWTEXT          */ TEXT("WindowText"),
/* COLOR_CAPTIONTEXT         */ TEXT("TitleText"),
/* COLOR_ACTIVEBORDER        */ TEXT("ActiveBorder"),
/* COLOR_INACTIVEBORDER      */ TEXT("InactiveBorder"),
/* COLOR_APPWORKSPACE        */ TEXT("AppWorkspace"),
/* COLOR_HIGHLIGHT           */ TEXT("Hilight"),
/* COLOR_HIGHLIGHTTEXT       */ TEXT("HilightText"),
/* COLOR_3DFACE              */ TEXT("ButtonFace"),
/* COLOR_3DSHADOW            */ TEXT("ButtonShadow"),
/* COLOR_GRAYTEXT            */ TEXT("GrayText"),
/* COLOR_BTNTEXT             */ TEXT("ButtonText"),
/* COLOR_INACTIVECAPTIONTEXT */ TEXT("InactiveTitleText"),
/* COLOR_3DHILIGHT           */ TEXT("ButtonHilight"),
/* COLOR_3DDKSHADOW          */ TEXT("ButtonDkShadow"),
/* COLOR_3DLIGHT             */ TEXT("ButtonLight"),
/* COLOR_INFOTEXT            */ TEXT("InfoText"),
/* COLOR_INFOBK              */ TEXT("InfoWindow"),
/* COLOR_3DALTFACE           */ TEXT("ButtonAlternateFace"),
/* COLOR_HOTLIGHT            */ TEXT("HotTrackingColor"),
/* COLOR_GRADIENTACTIVECAPTION   */ TEXT("GradientActiveTitle"),
/* COLOR_GRADIENTINACTIVECAPTION */ TEXT("GradientInactiveTitle"),
};
TCHAR g_szColors[] = TEXT("colors");           // colors section name

// Location of the Colors subkey in Registry; Defined in RegStr.h
TCHAR szRegStr_Colors[] = REGSTR_PATH_COLORS;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//this order has to match the enum order in look.h
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
LOOK_ELEMENT g_elements[] = {
/* ELEMENT_APPSPACE        */   {COLOR_APPWORKSPACE,    SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_APPSPACE, -1,       COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_DESKTOP         */   {COLOR_BACKGROUND,      SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DESKTOP, -1,        COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_INACTIVEBORDER  */   {COLOR_INACTIVEBORDER,  SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_INACTIVEBORDER, -1, COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ACTIVEBORDER    */   {COLOR_ACTIVEBORDER,    SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_ACTIVEBORDER, -1,   COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_INACTIVECAPTION */   {COLOR_INACTIVECAPTION, SIZE_CAPTION,   TRUE,    COLOR_INACTIVECAPTIONTEXT,FONT_CAPTION,ELNAME_INACTIVECAPTION, -1,COLOR_GRADIENTINACTIVECAPTION, {-1,-1,-1,-1}},
/* ELEMENT_INACTIVESYSBUT1 */   {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_INACTIVESYSBUT2 */   {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ACTIVECAPTION   */   {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  ELNAME_ACTIVECAPTION, -1,  COLOR_GRADIENTACTIVECAPTION, {-1,-1,-1,-1}},
/* ELEMENT_ACTIVESYSBUT1   */   {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_CAPTIONBUTTON, -1,  COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ACTIVESYSBUT2   */   {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_MENUNORMAL      */   {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_MENUTEXT,         FONT_MENU,     ELNAME_MENU, -1,           COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_MENUSELECTED    */   {COLOR_HIGHLIGHT,       SIZE_MENU,      TRUE,    COLOR_HIGHLIGHTTEXT,    FONT_MENU,     ELNAME_MENUSELECTED, -1,   COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_MENUDISABLED    */   {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_NONE,             FONT_MENU,     -1, ELEMENT_MENUNORMAL,    COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_WINDOW          */   {COLOR_WINDOW,          SIZE_NONE,      FALSE,   COLOR_WINDOWTEXT,       FONT_NONE,     ELNAME_WINDOW, -1,         COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_MSGBOX          */   {COLOR_NONE,            SIZE_NONE,      TRUE,    COLOR_WINDOWTEXT,       FONT_MSGBOX,   ELNAME_MSGBOX, -1,         COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_MSGBOXCAPTION   */   {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  -1, ELEMENT_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, {-1,-1,-1,-1}},
/* ELEMENT_MSGBOXSYSBUT    */   {COLOR_3DFACE,          SIZE_CAPTION,   TRUE,    COLOR_BTNTEXT,          FONT_CAPTION,  -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
// do not even try to set a scrollbar color the system will ignore you
/* ELEMENT_SCROLLBAR       */   {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_SCROLLBAR, -1,      COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_SCROLLUP        */   {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_SCROLLDOWN      */   {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_BUTTON          */   {COLOR_3DFACE,          SIZE_NONE,      FALSE,   COLOR_BTNTEXT,          FONT_NONE,     ELNAME_BUTTON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_SMCAPTION       */   {COLOR_NONE,            SIZE_SMCAPTION, TRUE,    COLOR_NONE,             FONT_SMCAPTION,ELNAME_SMALLCAPTION, -1,   COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ICON            */   {COLOR_NONE,            SIZE_ICON,      FALSE,   COLOR_NONE,             FONT_ICONTITLE,ELNAME_ICON, -1,           COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ICONHORZSPACING */   {COLOR_NONE,            SIZE_DXICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DXICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_ICONVERTSPACING */   {COLOR_NONE,            SIZE_DYICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DYICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_INFO            */   {COLOR_INFOBK,          SIZE_NONE,      TRUE,    COLOR_INFOTEXT,         FONT_STATUS,   ELNAME_INFO, -1,           COLOR_NONE, {-1,-1,-1,-1}},
/* ELEMENT_HOTTRACKAREA    */   {COLOR_HOTLIGHT,        SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_HOTTRACKAREA, -1,   COLOR_NONE, {-1,-1,-1,-1}}
};
#if 0
// go fix look.h if you decide to add this back in
/* ELEMENT_SMICON          */   {COLOR_NONE,            SIZE_SMICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_SMICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
#endif

// used by ChooseColor dialog
COLORREF g_CustomColors[16];

// structure used to store a scheme in the registry
#ifdef UNICODE
#   define SCHEME_VERSION 2        // Ver 2 == Unicode
#else
#   define SCHEME_VERSION 3        // Ver 3 == Memphis ANSI
#endif

#define SCHEME_VERSION_WIN32 4     // Win32 version of schemes can be loaded from both NT and Win95

typedef struct {
    SHORT version;
    // for alignment
    WORD  wDummy;
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA;

#define SCHEME_VERSION_400      1
#define SCHEME_VERSION_NT_400   2
#define COLOR_MAX_400       (COLOR_INFOBK + 1)

HWND g_hDlg = NULL;             // nonzero if page is up
int g_iCurElement = -2;         // start off as not even "not set"
int g_LogDPI = 96;              // logical resolution of display
#define ELCUR           (g_elements[g_iCurElement])
#define ELCURFONT       (g_fonts[ELCUR.iFont])
// this one kept separately for range purposes
#define ELCURSIZE       g_elCurSize
int g_iPrevSize = SIZE_NONE;

#define MAXSCHEMENAME 100
TCHAR g_szCurScheme[MAXSCHEMENAME];      // current scheme name
TCHAR g_szLastScheme[MAXSCHEMENAME];     // last scheme they had

HBRUSH g_hbrMainColor = NULL;
HBRUSH g_hbrTextColor = NULL;
HBRUSH g_hbrGradientColor = NULL;

static const TCHAR c_szRegPathUserMetrics[] = TEXT("Control Panel\\Desktop\\WindowMetrics");
static const TCHAR c_szRegValIconSize[] = TEXT("Shell Icon Size");
static const TCHAR c_szRegValSmallIconSize[] = TEXT("Shell Small Icon Size");
static const TCHAR c_szRegDefFontPath[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel");
static const TCHAR c_szRegDefFontName[] = TEXT("DefaultFontName");
static const TCHAR c_szTahoma[] = TEXT("Tahoma");
static const TCHAR c_szRegPathAppearance[] = TEXT("Control Panel\\Appearance");
static const TCHAR c_szRegValCharsets[] = TEXT("RecentFourCharsets");

#ifndef WINNT
#define READ_3D_RULES_FROM_REGISTRY 1
#endif

void NEAR PASCAL Look_Get3DRules(void);
void NEAR PASCAL Look_Repaint(HWND hDlg, BOOL bRecalc);
BOOL NEAR PASCAL Look_ChangeColor(HWND hDlg, int iColor, COLORREF rgb);
INT_PTR CALLBACK SaveSchemeDlgProc(HWND, UINT, WPARAM, LPARAM);
void NEAR PASCAL Look_UpdateSizeBasedOnFont(HWND hDlg, BOOL fComputeIdeal);
void NEAR PASCAL Look_SetCurSizeAndRange(HWND hDlg);
void NEAR PASCAL Look_SyncSize(HWND hDlg);
void NEAR PASCAL Look_DoSizeStuff(HWND hDlg, BOOL fCanComputeIdeal);
void Look_UpdateDPIchange(HWND hDlg);
void Look_GetCurrentCharsets(UINT uiCharsets[], int iCount);
void NEAR PASCAL Scheme_SetCurrentScheme(LPTSTR szCurScheme);
BOOL Look_CheckFontsCharsets(LOGFONT lfUIFonts[], int iCountFonts, 
                        UINT uiCurUniqueCharsets[], int iCountCurUniqueCharsets, 
                        BOOL fSilent, BOOL *pfNewName, BOOL *pfDirty, LPCTSTR lpszName);
BOOL Look_CheckSchemeCharsets(SCHEMEDATA  *psd, LPCTSTR lpszName, 
                        UINT uiCurUniqueCharsets[], int iCountCurUniqueCharsets, 
                        BOOL fSilent, BOOL *pfNewName, BOOL *pfDirty);



//The following macro is used to apply a DPI change to the given scheme data!
#define ApplyDPIchangeToSchemeData(sd)     AdjustFontHeightsForDPIchange(&(sd.ncm), &(sd.lfIconTitle), 96, g_iAppliedNewDPI)
//The following macro restores back the original font heights (used before a DPI change was applied).
#define RemoveDPIchangeFromSchemeData(sd)  AdjustFontHeightsForDPIchange(&(sd.ncm), &(sd.lfIconTitle), g_iAppliedNewDPI, 96)
//
// bugbug, needs to be in inc16\windows.h
//
//int WINAPI EnumFontFamiliesEx(HDC, LPLOGFONT, FONTENUMPROC, LPARAM, DWORD);


COLORREF GetNearestPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    PALETTEENTRY pe;
    GetPaletteEntries(hpal, GetNearestPaletteIndex(hpal, rgb & 0x00FFFFFF), 1, &pe);
    return RGB(pe.peRed, pe.peGreen, pe.peBlue);
}

BOOL IsPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    return GetNearestPaletteColor(hpal, rgb) == (rgb & 0xFFFFFF);
}

//
//  make the color a solid color if it needs to be.
//  on a palette device make is a palette relative color, if we need to.
//
COLORREF NearestColor(int iColor, COLORREF rgb)
{
    rgb &= 0x00FFFFFF;

    //
    // if we are on a palette device, we need to do special stuff...
    //
    if (g_bPalette)
    {
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            if (IsPaletteColor(g_hpal3D, rgb))
                rgb |= RGB_PALETTE;
            else
                rgb = GetNearestPaletteColor(g_hpalVGA, rgb);
        }
        else
        {
            if (IsPaletteColor(g_hpal3D, rgb))
                rgb |= RGB_PALETTE;

            else if (IsPaletteColor(GetStockObject(DEFAULT_PALETTE), rgb))
                rgb ^= 0x000001;    // force a dither
        }
    }
    else
    {
        // map color to nearest color if we need to for this UI element.
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            HDC hdc = GetDC(NULL);
            rgb = GetNearestColor(hdc, rgb);
            ReleaseDC(NULL, hdc);
        }
    }

    return rgb;
}

// ------------------------ magic color utilities --------------------------
/*
** set a color in the 3D palette.
*/
void NEAR PASCAL Set3DPaletteColor(COLORREF rgb, int iColor)
{
    int iPalette;
    PALETTEENTRY pe;

    if (!g_hpal3D)
        return;

    switch (iColor)
    {
        case COLOR_3DFACE:
            iPalette = 16;
            break;
        case COLOR_3DSHADOW:
            iPalette = 17;
            break;
        case COLOR_3DHILIGHT:
            iPalette = 18;
            break;
        default:
            return;
    }

    pe.peRed    = GetRValue(rgb);
    pe.peGreen  = GetGValue(rgb);
    pe.peBlue   = GetBValue(rgb);
    pe.peFlags  = 0;
    SetPaletteEntries(g_hpal3D, iPalette, 1, (LPPALETTEENTRY)&pe);
}
// ------------end--------- magic color utilities --------------------------

// ------------------------ manage system settings --------------------------
/*
** Helper Routine since USER's new METRICS structures are all 32-bit
*/
#ifndef LF32toLF

void NEAR LF32toLF(LPLOGFONT_32 lplf32, LPLOGFONT lplf)
{
    lplf->lfHeight       = (int) lplf32->lfHeight;
    lplf->lfWidth        = (int) lplf32->lfWidth;
    lplf->lfEscapement   = (int) lplf32->lfEscapement;
    lplf->lfOrientation  = (int) lplf32->lfOrientation;
    lplf->lfWeight       = (int) lplf32->lfWeight;
    *((LPCOMMONFONT) &lplf->lfItalic) = lplf32->lfCommon;
}
#endif

#ifndef LFtoLF32
void NEAR LFtoLF32(LPLOGFONT lplf, LPLOGFONT_32 lplf32)
{
    lplf32->lfHeight = (LONG)lplf->lfHeight;
    lplf32->lfWidth = (LONG)lplf->lfWidth;
    lplf32->lfEscapement = (LONG)lplf->lfEscapement;
    lplf32->lfOrientation = (LONG)lplf->lfOrientation;
    lplf32->lfWeight = (LONG)lplf->lfWeight;
    lplf32->lfCommon = *((LPCOMMONFONT) &lplf->lfItalic);
}
#endif

void AdjustFontHeightsForDPIchange(LPNONCLIENTMETRICS lpncm, LPLOGFONT plfIconTitle, int iOldDPI, int iNewDPI)
{
    //Check if the DPI has changed; Otherwise, no need to modify the font sizes.
    if(iNewDPI != iOldDPI)
    {
        lpncm->lfCaptionFont.lfHeight   = MulDiv(lpncm->lfCaptionFont.lfHeight, iNewDPI, iOldDPI);
        lpncm->lfSmCaptionFont.lfHeight = MulDiv(lpncm->lfSmCaptionFont.lfHeight, iNewDPI, iOldDPI);
        lpncm->lfMenuFont.lfHeight      = MulDiv(lpncm->lfMenuFont.lfHeight, iNewDPI, iOldDPI);
        lpncm->lfStatusFont.lfHeight    = MulDiv(lpncm->lfStatusFont.lfHeight, iNewDPI, iOldDPI);
        lpncm->lfMessageFont.lfHeight   = MulDiv(lpncm->lfMessageFont.lfHeight, iNewDPI, iOldDPI);
        plfIconTitle->lfHeight          = MulDiv(plfIconTitle->lfHeight, iNewDPI, iOldDPI);
    }
}

/*
** Fill in a NONCLIENTMETRICS structure with latest preview stuff
*/
void NEAR GetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm)
{
    lpncm->iBorderWidth = (LONG)g_sizes[SIZE_FRAME].CurSize;
    lpncm->iScrollWidth = lpncm->iScrollHeight = (LONG)g_sizes[SIZE_SCROLL].CurSize;
    lpncm->iCaptionWidth = lpncm->iCaptionHeight = (LONG)g_sizes[SIZE_CAPTION].CurSize;
    lpncm->iSmCaptionWidth = lpncm->iSmCaptionHeight = (LONG)g_sizes[SIZE_SMCAPTION].CurSize;
    lpncm->iMenuWidth = lpncm->iMenuHeight = (LONG)g_sizes[SIZE_MENU].CurSize;
    LFtoLF32(&(g_fonts[FONT_CAPTION].lf), &(lpncm->lfCaptionFont));
    LFtoLF32(&(g_fonts[FONT_SMCAPTION].lf), &(lpncm->lfSmCaptionFont));
    LFtoLF32(&(g_fonts[FONT_MENU].lf), &(lpncm->lfMenuFont));
    LFtoLF32(&(g_fonts[FONT_STATUS].lf), &(lpncm->lfStatusFont));
    LFtoLF32(&(g_fonts[FONT_MSGBOX].lf), &(lpncm->lfMessageFont));
}
/*
** given a NONCLIENTMETRICS structure, make it preview's current setting
*/
void NEAR SetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm)
{
    g_sizes[SIZE_FRAME].CurSize = (int)lpncm->iBorderWidth;
    g_sizes[SIZE_SCROLL].CurSize = (int)lpncm->iScrollWidth;
    g_sizes[SIZE_CAPTION].CurSize = (int)lpncm->iCaptionHeight;
    g_sizes[SIZE_SMCAPTION].CurSize = (int)lpncm->iSmCaptionHeight;
    g_sizes[SIZE_MENU].CurSize = (int)lpncm->iMenuHeight;

    LF32toLF(&(lpncm->lfCaptionFont), &(g_fonts[FONT_CAPTION].lf));
    LF32toLF(&(lpncm->lfSmCaptionFont), &(g_fonts[FONT_SMCAPTION].lf));
    LF32toLF(&(lpncm->lfMenuFont), &(g_fonts[FONT_MENU].lf));
    LF32toLF(&(lpncm->lfStatusFont), &(g_fonts[FONT_STATUS].lf));
    LF32toLF(&(lpncm->lfMessageFont), &(g_fonts[FONT_MSGBOX].lf));
}

//
//  Given the Locale ID, this returns the corresponding charset
//
UINT  GetCharsetFromLCID(LCID   lcid)
{
    TCHAR szData[6+1]; // 6 chars are max allowed for this lctype
    UINT uiRet;
    if(GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, szData, ARRAYSIZE(szData)) > 0)
    {
        UINT uiCp = (UINT)MyStrToLong(szData);
        CHARSETINFO csinfo;

        TranslateCharsetInfo((DWORD *)uiCp, &csinfo, TCI_SRCCODEPAGE);
        uiRet = csinfo.ciCharset;
    }
    else
    {
        // at worst non penalty for charset
        uiRet = DEFAULT_CHARSET;
    }

    return uiRet;
}

//
// Given the Language ID, this gets the charset.
//
UINT  GetCharsetFromLang(LANGID   wLang)
{
    return(GetCharsetFromLCID(MAKELCID(wLang, SORT_DEFAULT)));
}

BOOL Look_ReadCharsets(UINT uiCharsets[], int iCount)
{
    HKEY    hkAppearance;
    BOOL    fSuccess = FALSE;

    if(RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegPathAppearance, 0, KEY_READ, &hkAppearance) == ERROR_SUCCESS)
    {
        DWORD   dwType = REG_BINARY;
        DWORD   dwSize = iCount * sizeof(UINT);
        
        if(RegQueryValueEx(hkAppearance, c_szRegValCharsets, NULL, &dwType, (LPBYTE)uiCharsets, &dwSize) == ERROR_SUCCESS)
            fSuccess = TRUE;

        RegCloseKey(hkAppearance);
    }

    return fSuccess;
}

BOOL Look_SaveCharsets(UINT uiCharsets[], int iCount)
{
    HKEY    hkAppearance;
    BOOL    fSuccess = FALSE;
    
    if(RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegPathAppearance, 0, TEXT(""), 0, KEY_WRITE, NULL, &hkAppearance, NULL) == ERROR_SUCCESS)
    {
        if(RegSetValueEx(hkAppearance, c_szRegValCharsets, 0, REG_BINARY, (LPBYTE)uiCharsets, iCount * sizeof(UINT)) == ERROR_SUCCESS)
            fSuccess = TRUE;
            
        RegCloseKey(hkAppearance);
    }

    return fSuccess;
}

void Look_GetUniqueCharsets(UINT uiCharsets[], UINT uiUniqueCharsets[], int iMaxCount, int *piCountUniqueCharsets)
{
    int i, j;
    
    // Find the unique Charsets;
    *piCountUniqueCharsets = 0;
    for(i = 0; i < iMaxCount; i++)
    {
        uiUniqueCharsets[i] = DEFAULT_CHARSET; //Initialize it to default charset.
        
        for(j = 0; j < *piCountUniqueCharsets; j++)
        {
            if(uiUniqueCharsets[j] == uiCharsets[i])
                break; // This Charset is already in the array
        }
        if(j == *piCountUniqueCharsets)
        {
            //Yes! It is a unique char set; Save it!
            uiUniqueCharsets[j] = uiCharsets[i];
            (*piCountUniqueCharsets)++; //One more unique char set found.
        }
    }
}

int Look_CompareUniqueCharsets(UINT uiCharset1[], int iCount1, UINT uiCharset2[], int iCount2)
{
    if(iCount1 == iCount2)
    {
        int i, j;
        
        // The first items in the array is SYSTEM CHAR SET; It must match because system locale's
        // charset is always used by the Icon Title font; Icon Title font's charset is used by 
        // comctl32 to do A/W conversion. In order that all ANSI applications run correctly, 
        // the icon charset must always match current system locale.
        if(uiCharset1[SYSTEM_LOCALE_CHARSET] != uiCharset2[SYSTEM_LOCALE_CHARSET])
            return -1;

        //Now see if the arrays have the same elements.
        ASSERT(SYSTEM_LOCALE_CHARSET == 0);
        
        for(i = SYSTEM_LOCALE_CHARSET+1; i < iCount1; i++)
        {
            for(j = SYSTEM_LOCALE_CHARSET+1; j < iCount2; j++)
            {
                if(uiCharset1[i] == uiCharset2[j])
                    break;
            }
            if( j == iCount2)
                return -1;   // uiCharset1[i] is not found in the second array.
        }
    }
    
    return (iCount1 - iCount2); //Both the arrays have the same Charsets
}

void Look_GetUIFonts(NONCLIENTMETRICS *pncm, LOGFONT lfUIFonts[])
{
    pncm->cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
                                (void far *)(LPNONCLIENTMETRICS)pncm, FALSE);

    // Read the icon title font directly into the font array.
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT),
                (void far *)(LPLOGFONT)&(lfUIFonts[FONT_ICONTITLE]), FALSE);
                
    //Make a copy of the ncm fonts into fonts array.
    LF32toLF(&(pncm->lfCaptionFont), &(lfUIFonts[FONT_CAPTION]));
    LF32toLF(&(pncm->lfSmCaptionFont), &(lfUIFonts[FONT_SMCAPTION]));
    LF32toLF(&(pncm->lfMenuFont), &(lfUIFonts[FONT_MENU]));
    LF32toLF(&(pncm->lfStatusFont), &(lfUIFonts[FONT_STATUS]));
    LF32toLF(&(pncm->lfMessageFont), &(lfUIFonts[FONT_MSGBOX]));
}

void Look_SetUIFonts(NONCLIENTMETRICS *pncm, LOGFONT lfUIFonts[])
{
    //Copy all fonts back into the ncm structure.
    LFtoLF32(&(lfUIFonts[FONT_CAPTION]), &(pncm->lfCaptionFont));
    LFtoLF32(&(lfUIFonts[FONT_SMCAPTION]), &(pncm->lfSmCaptionFont));
    LFtoLF32(&(lfUIFonts[FONT_MENU]), &(pncm->lfMenuFont));
    LFtoLF32(&(lfUIFonts[FONT_STATUS]), &(pncm->lfStatusFont));
    LFtoLF32(&(lfUIFonts[FONT_MSGBOX]), &(pncm->lfMessageFont));

    // BUGBUG: Do we want a WININICHANGE HERE?
    // NOTE: We want to set a SPIF_SENDWININICHANGE, because we want to refresh.
    TraceMsg(TF_GENERAL, "desk.cpl: Calling SPI_SETNONCLIENTMETRICS");
    SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
            (void far *)(LPNONCLIENTMETRICS)pncm,
            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);  
    TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_SETICONTITLELOGFONT");
    SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT),
            (void far *)(LPLOGFONT)&lfUIFonts[FONT_ICONTITLE],
            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
}


#ifndef WINNT
void ThunkSchemeDataFrom16To32(SCHEMEDATA * psd, DWORD dwSize)
{
    SCHEMEDATA sdTemp = {0};
    LPBYTE pCur = (LPBYTE)psd;
    SHORT * pShort;
    ASSERT(psd);
    
    sdTemp.version = psd->version;
    sdTemp.wDummy = 0;

    // Copy over NONCLIENTMETRICS ncm
    pCur += SIZEOF(SHORT);
    hmemcpy(&sdTemp.ncm, pCur, SIZEOF(NONCLIENTMETRICS));
    pCur += SIZEOF(NONCLIENTMETRICS);

    // Now Thunk lfIconTitle from 16-bit LOGFONT to 32-bit LOGFONT
    pShort = (SHORT *)pCur;
    sdTemp.lfIconTitle.lfHeight = (int) (* pShort++);
    sdTemp.lfIconTitle.lfWidth = (int) (* pShort++);
    sdTemp.lfIconTitle.lfEscapement = (int) (* pShort++);
    sdTemp.lfIconTitle.lfOrientation = (int) (* pShort++);
    sdTemp.lfIconTitle.lfWeight = (int) (* pShort++);

    pCur = (LPBYTE)pShort;
    // Copy the rest over
    hmemcpy(&(sdTemp.lfIconTitle.lfItalic), pCur, SIZEOF(LOGFONT) - SIZEOF(LONG) * 5);
    pCur += SIZEOF(LOGFONT) - SIZEOF(LONG) * 5;

    // Copy the Colors 
    hmemcpy(&(sdTemp.rgb[0]), pCur, dwSize - (pCur - (LPBYTE)psd));

    // Copy the Scheme data back to psd
    hmemcpy(psd, &sdTemp, SIZEOF(sdTemp));
}
#endif


//
//  This function reads the current scheme's name and associated scheme data from registry.
//
//  This function returns FALSE, if there is no current scheme.
// 
BOOL  Look_GetCurSchemeNameAndData(SCHEMEDATA *psd, LPTSTR lpszSchemeName, int iLen)
{
    HKEY    hkAppearance;
    BOOL    fCurrentSchemeValid = FALSE;
    
    //Get the current scheme.
    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, &hkAppearance) == ERROR_SUCCESS)
    {
        DWORD   dwSize;
        
        dwSize = iLen;
        if (RegQueryValueEx(hkAppearance, REGSTR_KEY_CURRENT, NULL, NULL, (LPBYTE)lpszSchemeName, &dwSize) == ERROR_SUCCESS)
        {
            HKEY    hkSchemes;
            //Open the schemes key!
            if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS)
            {
                DWORD   dwType = REG_BINARY;
                dwSize = sizeof(SCHEMEDATA);
                if (RegQueryValueEx(hkSchemes, lpszSchemeName, NULL, &dwType, (LPBYTE)psd, &dwSize) == ERROR_SUCCESS)
                {
#ifdef WINNT
                    if (psd->version == SCHEME_VERSION)
                    {
                        fCurrentSchemeValid = TRUE;
                    }
#else
                    // 16 -- bit version, need to convert. This is a Memphis only issue
                    if ((psd->version == SCHEME_VERSION) || (psd->version == SCHEME_VERSION_400))
                    {
                        ThunkSchemeDataFrom16To32(psd, dwSize);
                        fCurrentSchemeValid = TRUE;
                    }
                    else if (psd->version == SCHEME_VERSION_WIN32)
                    {
                        fCurrentSchemeValid = TRUE;
                    }
#endif      
                }

                RegCloseKey(hkSchemes);
            }
        }

        RegCloseKey(hkAppearance);
    }

    return fCurrentSchemeValid; //Yes there is a current scheme and there is valid data! 
}

BOOL NEAR PASCAL Look_SaveGivenSchemeData(LPCTSTR lpszName, SCHEMEDATA  *psd)
{
    BOOL bRet = FALSE;
    HKEY hkSchemes;

    if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hkSchemes, lpszName, 0L, REG_BINARY, (LPBYTE)psd, sizeof(*psd)) == ERROR_SUCCESS)
            bRet = TRUE;

        RegCloseKey(hkSchemes);
    }
    return bRet;
}

//------------------------------------------------------------------------------------------------
//
// This is the function that needs to be called everytime a locale changes (or could have changed).
//
// It does the following:
//  1. It checks if any of the four the charset settings has changed.
//  2. If some charset has changed, it makes the corresponding changes in the 6 UI fonts.
//  3. If a scheme is selected, it checks to see if the fonts support the new charsets and if not
//     changes the fonts and/or charsets and saves the scheme under a new name.
//  4. Saves the new charsets in the registry sothat we don't have to do the same everytime this
//     function is called.
//
//  NOTE: This is a private export from desk.cpl. This is called in two places:
//  1. from regional control panel whenever it is run AND whenever it changes some locale.
//  2. from desk.cpl itself whenever "Appearance" tab is created. This is required because it is
// possible that an admin changes a system locale and then someother user logs-in. For this user
// the locale changes will cause font changes only when he runs Regional options or Appearance tab.
// The only other alternative would be to call this entry point whenever a user logs on, which will
// be a boot time perf hit.
//
//-------------------------------------------------------------------------------------------------
STDAPI UpdateCharsetChanges()
{
    UINT    uiCurCharsets[MAX_CHARSETS];
    UINT    uiRegCharsets[MAX_CHARSETS];
    UINT    uiCurUniqueCharsets[MAX_CHARSETS];
    int     iCountCurUniqueCharsets;
    UINT    uiRegUniqueCharsets[MAX_CHARSETS];
    int     iCountRegUniqueCharsets;
    NONCLIENTMETRICS    ncm;
    LOGFONT             lfUIFonts[NUM_FONTS];
    BOOL    fNewName = FALSE;
    BOOL    fDirty = FALSE;
    TCHAR   szSchemeName[MAXSCHEMENAME];
    SCHEMEDATA  sd;
    BOOL    fRegCharsetsValid = FALSE;
    
    //Get the current four charsets from system.
    Look_GetCurrentCharsets(uiCurCharsets, MAX_CHARSETS);
    //Get the charsets saved in the registry.
    fRegCharsetsValid = Look_ReadCharsets(uiRegCharsets, MAX_CHARSETS);

    //Get rid of the duplicate charsets and get only the unique Charsets from these arrays.
    Look_GetUniqueCharsets(uiCurCharsets, uiCurUniqueCharsets, MAX_CHARSETS, &iCountCurUniqueCharsets);
    if(fRegCharsetsValid)
        Look_GetUniqueCharsets(uiRegCharsets, uiRegUniqueCharsets, MAX_CHARSETS, &iCountRegUniqueCharsets);
    
    //Check if these two arrays have the same charsets.
    if(fRegCharsetsValid && (Look_CompareUniqueCharsets(uiCurUniqueCharsets, iCountCurUniqueCharsets, uiRegUniqueCharsets, iCountRegUniqueCharsets) == 0))
        return S_FALSE;  //The charsets are the same; Nothing to do!

    // Get all the 6 UI fonts.
    Look_GetUIFonts(&ncm, lfUIFonts);

    // Check to see if the fonts have proper charsets. TRUE => fSilent
    Look_CheckFontsCharsets(lfUIFonts, NUM_FONTS, uiCurUniqueCharsets, iCountCurUniqueCharsets, TRUE, &fNewName, &fDirty, TEXT(""));

    // if one or more of the fonts have to change, save the changes
    if(fDirty)
        Look_SetUIFonts(&ncm, lfUIFonts);

    // Read the current scheme, if one exists.
    if(Look_GetCurSchemeNameAndData(&sd, szSchemeName, ARRAYSIZE(szSchemeName)))
    {
        TCHAR   szNewScheme[MAXSCHEMENAME];
        LPTSTR  pszSchemeName;
        
        //Yes a scheme exists.
        fDirty = FALSE;
        fNewName = FALSE;
        Look_CheckSchemeCharsets(&sd, szSchemeName, uiCurUniqueCharsets, iCountCurUniqueCharsets, TRUE, &fNewName, &fDirty);

        if(fNewName)
        {
            TCHAR   szCopyOfTemplate[40];  //To load "Copy of %s".
            
            LoadString(hInstance, IDS_COPYOF_SCHEME, szCopyOfTemplate, ARRAYSIZE(szCopyOfTemplate));
            wsprintf(szNewScheme, szCopyOfTemplate, szSchemeName);
            pszSchemeName = szNewScheme;
        }
        else
            pszSchemeName = szSchemeName;
            
        if(fDirty)
            Look_SaveGivenSchemeData(pszSchemeName, &sd);

        if(fNewName)
            Scheme_SetCurrentScheme(pszSchemeName);
    }

    //Save the cur charsets into the registry.
    Look_SaveCharsets(uiCurCharsets, MAX_CHARSETS);

    return S_OK; //Yes! We had to do some updates in connection with charsets and fonts.
}

#ifndef WINNT
typedef LANGID (*LPGETLANGIDPROC)(void);
#endif //WINNT

void Look_GetCurrentCharsets(UINT uiCharsets[], int iCount)
{
    ASSERT( iCount == MAX_CHARSETS);
    
    // Get all the four charsets we are interested in.
    uiCharsets[0] = GetCharsetFromLCID(GetSystemDefaultLCID());
    uiCharsets[1] = GetCharsetFromLCID(GetUserDefaultLCID());
#ifdef WINNT
    uiCharsets[2] = GetCharsetFromLang(GetSystemDefaultUILanguage());
    uiCharsets[3] = GetCharsetFromLang(GetUserDefaultUILanguage());
#else
    //Only for Win98 and above these APIs are available.
    {
        HMODULE hMod;

        if(hMod = GetModuleHandle(TEXT("KERNEL32.DLL")))
        {
            LPGETLANGIDPROC  pfnGetSysDefUILan;
            LPGETLANGIDPROC  pfnGetUsrDefUILan;
            
            pfnGetSysDefUILan = (LPGETLANGIDPROC)GetProcAddress(hMod, "GetSystemDefaultLanguage");
            uiCharsets[2] = pfnGetSysDefUILan ? (*pfnGetSysDefUILan)() : uiCharsets[0];
            pfnGetUsrDefUILan = (LPGETLANGIDPROC)GetProcAddress(hMod, "GetUserDefaultLanguage");
            uiCharsets[3] = pfnGetUsrDefUILan ? (*pfnGetUsrDefUILan)() : uiCharsets[1];
        }
    }
#endif
}


void Look_InitUniqueCharsetArray()
{
    UINT    uiCharsets[MAX_CHARSETS];

    Look_GetCurrentCharsets(uiCharsets, ARRAYSIZE(uiCharsets));
    // Find the unique Charsets and save that in a global array.
    Look_GetUniqueCharsets(uiCharsets, g_uiUniqueCharsets, MAX_CHARSETS, &g_iCountCharsets);    
}

void NEAR PASCAL Color_SetColor(HWND hDlg, int id, HBRUSH hbrColor);
/*
** new data has been set.  flush out current objects and rebuild
*/
void NEAR PASCAL Look_RebuildSysStuff(BOOL fInit)
{
    int i;
    PALETTEENTRY pal[4];
    HPALETTE hpal = GetStockObject(DEFAULT_PALETTE);

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    for (i = 0; i < NUM_FONTS; i++)
    {
        if (g_fonts[i].hfont)
            DeleteObject(g_fonts[i].hfont);
        g_fonts[i].hfont = CreateFontIndirect(&g_fonts[i].lf);
    }

    if (fInit)
    {
        // get current magic colors
        GetPaletteEntries(hpal, 8,  4, pal);
        SetPaletteEntries(g_hpal3D, 16,  4, pal);

        // set up magic colors in the 3d palette
        if (!IsPaletteColor(hpal, g_rgb[COLOR_3DFACE]))
            Set3DPaletteColor(g_rgb[COLOR_3DFACE], COLOR_3DFACE);

        if (!IsPaletteColor(hpal, g_rgb[COLOR_3DSHADOW]))
            Set3DPaletteColor(g_rgb[COLOR_3DSHADOW],  COLOR_3DSHADOW);

        if (!IsPaletteColor(hpal, g_rgb[COLOR_3DHILIGHT]))
            Set3DPaletteColor(g_rgb[COLOR_3DHILIGHT], COLOR_3DHILIGHT);
    }

    for (i = 0; i < COLOR_MAX; i++)
    {
        if (g_brushes[i])
            DeleteObject(g_brushes[i]);

        g_brushes[i] = CreateSolidBrush(NearestColor(i, g_rgb[i]));
    }

    if (g_iCurElement >= 0)
    {
        //
        // we changed the brushes out from under the buttons...
        //
        Color_SetColor(NULL, IDC_MAINCOLOR, ((ELCUR.iMainColor != COLOR_NONE)?
            g_brushes[ELCUR.iMainColor] : NULL));
        
        Color_SetColor(NULL, IDC_GRADIENT, ((ELCUR.iGradientColor != COLOR_NONE)?
            g_brushes[ELCUR.iGradientColor] : NULL));

        Color_SetColor(NULL, IDC_TEXTCOLOR, ((ELCUR.iTextColor != COLOR_NONE)?
            g_brushes[ELCUR.iTextColor] : NULL));
    }
}


//
//  simple form of Shell message box, does not handle param replacment
//  just calls LoadString and MessageBox
//
int WINAPI DeskShellMessageBox(HINSTANCE hAppInst, HWND hWnd, LPCTSTR lpcText, LPCTSTR lpcTitle, UINT fuStyle)
{
    TCHAR    achText[CCH_MAX_STRING];
    TCHAR    achTitle[CCH_MAX_STRING];

    if (HIWORD(lpcText) == 0)
    {
        LoadString(hAppInst, LOWORD(lpcText), achText, ARRAYSIZE(achText));
        lpcText = (LPCTSTR)achText;
    }

    if (HIWORD(lpcTitle) == 0)
    {
        if (LOWORD(lpcTitle) == 0)
            GetWindowText(hWnd, achTitle, ARRAYSIZE(achTitle));
        else
            LoadString(hAppInst, LOWORD(lpcTitle), achTitle, ARRAYSIZE(achTitle));

        lpcTitle = (LPCTSTR)achTitle;
    }

    return MessageBox(hWnd, lpcText, lpcTitle, fuStyle);
}

void Look_SaveAppliedDPIinReg(int iNewDPI)
{
    HKEY    hkWinMetrics;
    
    // Update the "AppliedDPI" value in the registry.
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                                     SZ_WINDOWMETRICS,
                                     0,
                                     KEY_WRITE,
                                     &hkWinMetrics) == ERROR_SUCCESS) 
    {
        DWORD dwSize = sizeof(iNewDPI);
        RegSetValueEx(hkWinMetrics,
                        SZ_APPLIEDDPI,
                        0L,
                        REG_DWORD,
                        (LPBYTE) &iNewDPI,
                        dwSize);
        RegCloseKey(hkWinMetrics);
    }
}

//------------------------------------------------------------------------------------------------
//
//  This function gets the current DPI, reads the last updated DPI from registry and compares 
// these two. If these two are equal, it returns immediately.
//  If these two DPI values are different, then it updates the size of UI fonts to reflect the
// change in the DPI values.
//  
//  This function is called from explorer sothat when DPI value is changed by admin and then every
// other user who logs-in gets this change.
//
//
// This function is also called from "Settings" tab to update the UI fonts when "Appearance" tab
// is not initialized yet. So, this function calls USER32 APIs to update the UI fonts directly.
//
//------------------------------------------------------------------------------------------------

void WINAPI UpdateUIfontsDueToDPIchange(int iOldDPI, int iNewDPI)
{
    NONCLIENTMETRICS ncm;
    LOGFONT     lfIconTitle;
    
    // Get the icon title font.
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT),
                (void far *)(LPLOGFONT)&lfIconTitle, FALSE);

    ncm.cbSize = sizeof(ncm);
    // Get all the other UI fonts.
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm),
                                (void far *)(LPNONCLIENTMETRICS)&ncm, FALSE);
                                
    // Fix all the font sizes based on DPI
    AdjustFontHeightsForDPIchange(&ncm, &lfIconTitle, iOldDPI, iNewDPI);
    
    // Set all the fonts to new sizes.
    // NOTE: We don't want to set a SPIF_WININICHANGE, because reboot will happen when DPI changes.
    TraceMsg(TF_GENERAL, "desk.cpl: Calling SPI_SETNONCLIENTMETRICS");
    SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(ncm),
            (void far *)(LPNONCLIENTMETRICS)&ncm,
            SPIF_UPDATEINIFILE);  
    TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_SETICONTITLELOGFONT");
    SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT),
            (void far *)(LPLOGFONT)&lfIconTitle,
            SPIF_UPDATEINIFILE);

    // Update the "AppliedDPI" value in the registry.
    Look_SaveAppliedDPIinReg(iNewDPI);
}

//
// We have three globals that keep track of DPI change. Since these globals are used by
// "Appearance" tab, "Settings" tab and the Advanced->"General" tab, we need to initialize
// these globals before any of those tabs are running. So, made this out to be a function
// that gets called from those tabs' init code.
//
void Look_InitDPIglobals()
{
    HDC hdc = GetDC(NULL);
    
    g_LogDPI = GetDeviceCaps(hdc, LOGPIXELSY);

    ReleaseDC(NULL, hdc);

    if(g_iAppliedNewDPI == 0)  //Check if this global hasn't been initialized yet!
    {
        HKEY    hk;
        
        //See if this AppliedNewDPI value is saved in the registry!
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                                     SZ_WINDOWMETRICS,
                                     0,
                                     KEY_READ,
                                     &hk) == ERROR_SUCCESS) 
        {
            DWORD dwSize = sizeof(g_iAppliedNewDPI);
            
            RegQueryValueEx(hk,
                            SZ_APPLIEDDPI,
                            NULL,
                            NULL,
                            (LPBYTE) &g_iAppliedNewDPI,
                            &dwSize);

            RegCloseKey(hk);
        }
    }

    if(g_iAppliedNewDPI == 0)  //If we failed to read from the registry,
        g_iAppliedNewDPI = g_LogDPI;
        
    if(g_iNewDPI == 0)  //If this global is not initialized yet, 
        g_iNewDPI = g_iAppliedNewDPI;
}


/*
** get all of the interesting system information and put it in the tables
*/
void NEAR PASCAL Look_InitSysStuff(void)
{
    int i;
    NONCLIENTMETRICS ncm;
    HKEY hkey;
    HDC hdc;

    
    Look_InitDPIglobals();
    
    hdc = GetDC(NULL);
  
    g_bPalette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;

    ReleaseDC(NULL, hdc);

    // always make a palette even on non-pal device
    if (g_bPalette || TRUE)
    {
        DWORD pal[21];
        HPALETTE hpal = GetStockObject(DEFAULT_PALETTE);

        pal[1]  = RGB(255, 255, 255);
        pal[2]  = RGB(0,   0,   0  );
        pal[3]  = RGB(192, 192, 192);
        pal[4]  = RGB(128, 128, 128);
        pal[5]  = RGB(255, 0,   0  );
        pal[6]  = RGB(128, 0,   0  );
        pal[7]  = RGB(255, 255, 0  );
        pal[8]  = RGB(128, 128, 0  );
        pal[9]  = RGB(0  , 255, 0  );
        pal[10] = RGB(0  , 128, 0  );
        pal[11] = RGB(0  , 255, 255);
        pal[12] = RGB(0  , 128, 128);
        pal[13] = RGB(0  , 0,   255);
        pal[14] = RGB(0  , 0,   128);
        pal[15] = RGB(255, 0,   255);
        pal[16] = RGB(128, 0,   128);

        GetPaletteEntries(hpal, 11, 1, (LPPALETTEENTRY)&pal[17]);
        pal[0]  = MAKELONG(0x300, 17);
        g_hpalVGA = CreatePalette((LPLOGPALETTE)pal);

        // get magic colors
        GetPaletteEntries(hpal, 8,  4, (LPPALETTEENTRY)&pal[17]);

        pal[0]  = MAKELONG(0x300, 20);
        g_hpal3D = CreatePalette((LPLOGPALETTE)pal);
    }

    // system colors
    for (i = 0; i < COLOR_MAX; i++)
    {
        g_rgb[i] = GetSysColor(i);
        g_brushes[i] = NULL;
    }

    // sizes and fonts
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm),
                                (void far *)(LPNONCLIENTMETRICS)&ncm, FALSE);

    SetMyNonClientMetrics(&ncm);

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT),
                (void far *)(LPLOGFONT)&(g_fonts[FONT_ICONTITLE].lf), FALSE);

    // default shell icon sizes
    g_sizes[ SIZE_ICON ].CurSize = GetSystemMetrics( SM_CXICON );
    g_sizes[ SIZE_SMICON ].CurSize = g_sizes[ SIZE_ICON ].CurSize / 2;

    if( RegOpenKey( HKEY_CURRENT_USER, c_szRegPathUserMetrics, &hkey )
        == ERROR_SUCCESS )
    {
        TCHAR val[ 8 ];
        LONG len = sizeof( val );

        if( RegQueryValueEx( hkey, c_szRegValIconSize, 0, NULL, (LPBYTE)&val,
            (LPDWORD)&len ) == ERROR_SUCCESS )
        {
            g_sizes[ SIZE_ICON ].CurSize = (int)MyStrToLong( val );
        }

        len = SIZEOF( val );
        if( RegQueryValueEx( hkey, c_szRegValSmallIconSize, 0, NULL, (LPBYTE)&val,
            (LPDWORD)&len ) == ERROR_SUCCESS )
        {
            g_sizes[ SIZE_SMICON ].CurSize = (int)MyStrToLong( val );
        }

        RegCloseKey( hkey );
    }

    g_sizes[ SIZE_DXICON ].CurSize =
        GetSystemMetrics( SM_CXICONSPACING ) - g_sizes[ SIZE_ICON ].CurSize;
    if( g_sizes[ SIZE_DXICON ].CurSize < 0 )
        g_sizes[ SIZE_DXICON ].CurSize = 0;

    g_sizes[ SIZE_DYICON ].CurSize =
        GetSystemMetrics( SM_CYICONSPACING ) - g_sizes[ SIZE_ICON ].CurSize;
    if( g_sizes[ SIZE_DYICON ].CurSize < 0 )
        g_sizes[ SIZE_DYICON ].CurSize = 0;

    // clean out the memory
    for (i = 0; i < NUM_FONTS; i++)
    {
        g_fonts[i].hfont = NULL;
    }

    // build all the brushes/fonts we need
    Look_RebuildSysStuff(TRUE);
    
    // Get the current System and User charsets based on locales and UI languages.
    Look_InitUniqueCharsetArray();
    
}

/*
** clean up any mess made in maintaining system information
** also, write out any global changes in our setup.
*/
void NEAR PASCAL Look_DestroySysStuff(void)
{
    int i;
    HKEY hkAppear;

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    for (i = 0; i < NUM_FONTS; i++)
    {
        if (g_fonts[i].hfont)
        {
            DeleteObject(g_fonts[i].hfont);
            g_fonts[i].hfont = NULL;
        }
    }
    
    for (i = 0; i < COLOR_MAX; i++)
    {
        if (g_brushes[i])
        {
            DeleteObject(g_brushes[i]);
            g_brushes[i] = NULL;
        }
    }
    if (g_hpal3D)
    {
        DeleteObject(g_hpal3D);
        g_hpal3D = NULL;
    }
    if (g_hpalVGA)
    {
        DeleteObject(g_hpalVGA);
        g_hpalVGA = NULL;
    }
    
    g_hbrGradientColor = g_hbrMainColor = g_hbrTextColor = NULL;
    // save out possible changes to custom color table
    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, &hkAppear) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkAppear, REGSTR_VAL_CUSTOMCOLORS, 0L, REG_BINARY,
                                (LPBYTE)g_CustomColors, sizeof(g_CustomColors));
        RegCloseKey(hkAppear);
    }

    // reset these so they init properly
    g_iCurElement = -2;
    g_LogDPI = 96;
    g_iPrevSize = SIZE_NONE;
    g_bPalette = FALSE;
    g_bInit = TRUE;
    g_bCrappyColor = FALSE;
}
    

/*
** set all of the data to the system.
**
** COMPATIBILITY NOTE:
**   EXCEL 5.0 BOZOS hook metrics changes off of WM_SYSCOLORCHANGE
** instead of WM_WININICHANGE.  Windows 3.1's Desktop applet always sent
** both when the metrics were updated, so nobody noticed this bug.
**   Be careful when re-arranging this function...
**
*/
void NEAR PASCAL Look_SetSysStuff(UINT fChanged)
{
    // COMPATIBILITY:
    //   Do metrics first since the color stuff might cause USER to generate a
    // WM_SYSCOLORCHANGE message and we don't want to send two of them...

    TraceMsg(TF_GENERAL, "desk.cpl: Look_SetSysStuff");

    if( fChanged & METRIC_CHANGE )
    {
        NONCLIENTMETRICS ncm;
        HKEY hkey;

        TraceMsg(TF_GENERAL, "desk.cpl: Metrics Changed");

        GetMyNonClientMetrics(&ncm);

        ncm.cbSize = sizeof(ncm);
        TraceMsg(TF_GENERAL, "desk.cpl: Calling SPI_SETNONCLIENTMETRICS");
        SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(ncm),
            (void far *)(LPNONCLIENTMETRICS)&ncm,
            SPIF_UPDATEINIFILE);

        TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_SETICONTITLELOGFONT");
        SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT),
            (void far *)(LPLOGFONT)&(g_fonts[FONT_ICONTITLE].lf),
            SPIF_UPDATEINIFILE);

        TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_ICONHORIZONTALSPACING");
        SystemParametersInfo(SPI_ICONHORIZONTALSPACING,
            g_sizes[ SIZE_DXICON ].CurSize + g_sizes[ SIZE_ICON ].CurSize,
            NULL, SPIF_UPDATEINIFILE);

        TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_ICONVERTICALSPACING");
        SystemParametersInfo(SPI_ICONVERTICALSPACING,
            g_sizes[ SIZE_DYICON ].CurSize + g_sizes[ SIZE_ICON ].CurSize,
            NULL, SPIF_UPDATEINIFILE );
        TraceMsg(TF_GENERAL,"desk.cpl: Done calling SPI's");

        if( RegCreateKey( HKEY_CURRENT_USER, c_szRegPathUserMetrics, &hkey )
            == ERROR_SUCCESS )
        {
            TCHAR val[ 8 ];

            wsprintf( val, TEXT("%d"), g_sizes[ SIZE_ICON ].CurSize );
            RegSetValueEx( hkey, c_szRegValIconSize, 0, REG_SZ,
                (LPBYTE)&val, SIZEOF(TCHAR) * (lstrlen( val ) + 1) );

#ifdef THE_SHELL_CAN_HANDLE_CUSTOM_SMALL_ICON_SIZES_YET
            wsprintf( val, TEXT("%d"), g_sizes[ SIZE_SMICON ].CurSize );
            RegSetValueEx( hkey, c_szRegValSmallIconSize, 0, REG_SZ,
                (LPBYTE)&val, SIZEOF(TCHAR) * (lstrlen( val ) + 1) );
#else
            RegDeleteValue( hkey, c_szRegValSmallIconSize );
#endif

            RegCloseKey( hkey );
        }

        // WM_SETTINGCHANGE is sent at the end of the function
    }

    if( fChanged & COLOR_CHANGE )
    {
        int i;
        int iColors[COLOR_MAX];
        COLORREF rgbColors[COLOR_MAX];
        TCHAR szRGB[32];
        COLORREF rgb;
        HKEY     hk;
        HDC      hdc;

        //
        // restore magic colors back to Win31 defaults.
        //
        hdc = GetDC(NULL);
        SetMagicColors(hdc, 0x00c0dcc0, 8);         // money green
        SetMagicColors(hdc, 0x00f0caa6, 9);         // IBM blue
        SetMagicColors(hdc, 0x00f0fbff, 246);       // off white
        ReleaseDC(NULL, hdc);

        // -------------------------------------------------
        // This call causes user to send a WM_SYSCOLORCHANGE
        // -------------------------------------------------
        for (i=0; i<COLOR_MAX; i++)
        {
            iColors[i] = i;
            rgbColors[i] = g_rgb[i] & 0x00FFFFFF;
        }

        SetSysColors(COLOR_MAX, iColors, rgbColors);

        if(RegCreateKey(HKEY_CURRENT_USER, szRegStr_Colors, &hk) == ERROR_SUCCESS)
        {
            // write out the color information to win.ini
            for (i = 0; i < COLOR_MAX; i++)
            {
                rgb = g_rgb[i];
                wsprintf(szRGB, TEXT("%d %d %d"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

                // For the time being we will update the INI file also.
                WriteProfileString(g_szColors, s_pszColorNames[i], szRGB);
                // Update the registry (Be sure to include the terminating zero in the byte count!)
                RegSetValueEx(hk, s_pszColorNames[i], 0L, REG_SZ, (LPBYTE)szRGB, SIZEOF(TCHAR) * (lstrlen(szRGB)+1));
                TraceMsg(TF_GENERAL, "CPL:Write Color: %s=%s\n\r",s_pszColorNames[i], szRGB);
            }
            RegCloseKey(hk);
        }
    }
    else if( fChanged & METRIC_CHANGE )
    {
        // COMPATIBILITY HACK:
        // no colors were changed, but metrics were
        // EXCEL 5.0 BOZOS tied metrics changes to WM_SYSCOLORCHANGE
        // and ignore the WM_WININICHANGE (now called WM_SETTINGCHANGE)

        // send a bogus WM_SYSCOLORCHANGE
        SendMessageTimeout(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0, SMTO_ABORTIFHUNG, 5000, NULL);
    }

    // if metrics changed at all send a WM_SETTINGCHANGE
    if( fChanged & METRIC_CHANGE )
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, 0, SMTO_ABORTIFHUNG, 5000, NULL);
}

//--------end------------- manage system settings --------------------------

//------------------------ mini font picker controls --------------------------
int NEAR PASCAL Font_HeightToPoint(int Height)
{
    if (Height < 0)
        Height = (-Height);
    return ((Height * 72 + g_iAppliedNewDPI/2)/ g_iAppliedNewDPI);
}
int NEAR PASCAL Font_PointToHeight(int Points)
{
    return (-((Points * g_iAppliedNewDPI + 72/2) / 72));
}

//
// Add a facename/script combination to the font dropdown combo list.
//
// The strings are formatted as "FaceName (ScriptName)"
//
INT 
Font_AddNameToList(
    HWND hwndList, 
    LPTSTR pszFace, 
    LPTSTR pszScript
    )
{
    //
    // Create temp buffer to hold a face name, a script name, one space
    // two parens and a NUL char.
    // 
    //  i.e.: "Arial (Western)"
    //
#ifdef DEBUG
    TCHAR szFaceAndScript[LF_FACESIZE + LF_FACESIZE + 4];
#endif //DEBUG
    LPTSTR pszDisplayName = pszFace;
    INT iItem;

// We decided not to show the scriptnames; Only facenames will be shown.
// For the purpose of debugging, I leave the script name in debug versions only.
#ifdef DEBUG
    if (NULL != pszScript && TEXT('\0') != *pszScript)
    {
        //
        // Font has a script name.  Append it to the facename in parens.
        // This format string controls the appearance of the font names
        // in the list.  If you change this, you must also change the
        // extraction logic in Font_GetNameFromList().
        //
        wsprintf(szFaceAndScript, TEXT("%s(%s)"), pszFace, pszScript);
    
        pszDisplayName = szFaceAndScript;
    }
#endif //DEBUG

    //
    // Add the display name string to the listbox.
    //
    iItem = (INT)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)pszDisplayName);
    if (CB_ERR != iItem)
    {
        //
        // Ensure the drop-down combo list will show the entire string.
        //
        HDC hdc = GetDC(hwndList);
        if (NULL != hdc)
        {
            SIZE sizeItem;
            //
            // Make sure the list's font is selected into the DC before
            // calculating the text extent.
            //
            HFONT hfontList = (HFONT)SendMessage(hwndList, WM_GETFONT, 0, 0);
            HFONT hfontOld  = (HFONT)SelectObject(hdc, hfontList);

            if (GetTextExtentPoint32(hdc, pszDisplayName, lstrlen(pszDisplayName), &sizeItem))
            {
                //
                // Get the current width of the dropped list.
                //
                INT cxList = (int)SendMessage(hwndList, CB_GETDROPPEDWIDTH, 0, 0);
                //
                // We need the length of this string plus two
                // widths of a vertical scroll bar.
                //
                sizeItem.cx += (GetSystemMetrics(SM_CXVSCROLL) * 2);
                if (sizeItem.cx > cxList)
                {
                    //
                    // List is not wide enough.  Increase the width.
                    //
                    SendMessage(hwndList, CB_SETDROPPEDWIDTH, (WPARAM)sizeItem.cx, 0);
                }
            }
            SelectObject(hdc, hfontOld);
            ReleaseDC(hwndList, hdc);
        }
    }
    return iItem;
}

//
// Retrieve a font name from the font dropdown combo list.
// Optionally, retrieve the script name string.
//
BOOL 
Font_GetNameFromList(
    HWND hwndList,      // HWND of combo.
    INT iItem,          // Index of item in list.
    LPTSTR pszFace,     // Destination for face name.
    INT cchFaceMax,     // Chars in face name buffer.
    LPTSTR pszScript,   // Optional. Can be NULL
    INT cchScriptMax    // Optional. Ignored if pszScript is NULL
    )
{
    BOOL bResult = FALSE;
    TCHAR szItemText[LF_FACESIZE + LF_FACESIZE + 4];

    if (CB_ERR != SendMessage(hwndList, CB_GETLBTEXT, (WPARAM)iItem, (LPARAM)szItemText))
    {
        LPTSTR pszEnd, pszParen;                            // Lookahead pointer
        LPCTSTR pszStart = pszEnd = pszParen = szItemText;  // "Start" anchor pointer.

        //
        // Find the left paren.
        //
        for ( ; *pszEnd; pszEnd++) {
             if (TEXT('(') == *pszEnd)
                 pszParen = pszEnd;
        }
        
        if(pszParen > pszStart) //Did we find a parenthis?
            pszEnd = pszParen;  // Then that is the end of the facename.

        if (pszEnd > pszStart)
        {
            //
            // Found it.  Copy face name.
            //
            INT cchCopy = (int)(pszEnd - pszStart) + 1; //Add one for the null terminator
            if (cchCopy > cchFaceMax)
                cchCopy = cchFaceMax;

            lstrcpyn(pszFace, pszStart, cchCopy); //(cchCopy-1) bytes are copies followed by a null
            bResult = TRUE;

            if (*pszEnd && (NULL != pszScript))
            {
                //
                // Caller wants the script part also.
                //
                pszStart = ++pszEnd;
                
                //
                // Find the right paren.
                //
                while(*pszEnd && TEXT(')') != *pszEnd)
                    pszEnd++;

                if (*pszEnd && pszEnd > pszStart)
                {
                    //
                    // Found it.  Copy script name.
                    //
                    cchCopy = (int)(pszEnd - pszStart) + 1;
                    if (cchCopy > cchScriptMax)
                        cchCopy = cchScriptMax;

                    lstrcpyn(pszScript, pszStart, cchCopy);
                }
            }
        }
    }
    return bResult;
}


//
// Locate a facename/charset pair in the font list.
//
INT Font_FindInList(HWND hwndList, LPCTSTR pszFaceName)
{
    INT cItems = (int)SendMessage(hwndList, CB_GETCOUNT, 0, 0);
    INT i;

    for (i = 0; i < cItems; i++)
    {
        // All items in the fontlist have the same charset (SYSTEM_LOCALE_CHARSET).So, no point
        // in checking for the charset.
        //
        // Let's just get the facename and see if it matches.
        //
        TCHAR szFaceName[LF_FACESIZE + 1];
        
        Font_GetNameFromList(hwndList, i, szFaceName, ARRAYSIZE(szFaceName), NULL, 0);

        if (0 == lstrcmpi(szFaceName, pszFaceName))
        {
            //
            // Face name matches.
            //
            return i;
        }
    }
    //
    // No match found.
    //
    return -1;
}


//
// Determine if a given font should be included in the font list.
// 
// dwType arg is DEVICE_FONTTYPE, RASTER_FONTTYPE, TRUETYPE_FONTTYPE.
//               EXTERNAL_FONTTYPE is a private code.  These are the
//               values returned to the enumproc from GDI.
//
BOOL
Font_IncludeInList(
    LPENUMLOGFONTEX lpelf,
    DWORD dwType
    )
{
    BOOL bResult   = TRUE; // Assume it's OK to include.
    BYTE lfCharSet = lpelf->elfLogFont.lfCharSet;

#define EXTERNAL_FONTTYPE 8

    //
    // Exclusions:
    //
    // 1. Don't display WIFE font for appearance because WIFE fonts are not 
    //    allowed to be any system use font such as menu/caption as it 
    //    realizes the font before WIFE gets initialized. B#5427
    //
    // 2. Exclude SYMBOL fonts.
    //
    // 3. Exclude OEM fonts.
    //
    // 4. Exclude vertical fonts.
    //
    if (EXTERNAL_FONTTYPE & dwType ||
        lfCharSet == SYMBOL_CHARSET ||
        lfCharSet == OEM_CHARSET ||
        TEXT('@') == lpelf->elfLogFont.lfFaceName[0])
    {
        bResult = FALSE;
    }
    return bResult;
}

int CALLBACK Font_EnumValidCharsets(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, LPARAM lData)
{
    // The purpose of this function is to determine if a font supports a particular charset;
    // If this callback gets called even once, then that means that this font supports the given
    // charset. There is no need to enumerate all the other styles. We immediately return zero to
    // stop the enumeration. Since we return zero, the EnumFontFamiliesEx() also returns zero in 
    // this case and that return value is used to determine if a given font supports a given
    // charset.

    return 0;
}


typedef struct  {
    HWND    hwndFontName;
    HDC     hdc;
}  ENUMFONTPARAM;

int CALLBACK Font_EnumNames( LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, LPARAM lData)
{
    ENUMFONTPARAM   *pEnumFontParam = (ENUMFONTPARAM *)lData;
    //
    // Should font be included in the "Font" list?
    //
    if (Font_IncludeInList(lpelf, Type))
    {
        int j;
        LOGFONT lf = lpelf->elfLogFont;             //Make a local copy of the given font
        BYTE    bSysCharset = lf.lfCharSet;         //Preserve the system charset we got.
        BOOL    fSupportsAllCharsets = TRUE;
        
        //The given font supports the system charset; Let's check if it supports the other charsets
        for(j = 1; j < g_iCountCharsets; j++)
        {
            lf.lfCharSet = (BYTE)g_uiUniqueCharsets[j];  //Let's try the next charset in the array.
            if(EnumFontFamiliesEx(pEnumFontParam->hdc, &lf, (FONTENUMPROC)Font_EnumValidCharsets, (LPARAM)0, 0) != 0)
            {
                // EnumFontFamiliesEx would have returned a zero if Font_EnumValidCharsets was called
                // even once. In other words, it returned a non-zero because not even a single font existed
                // that supported the given charset. Therefore, we need to skip this font.
                fSupportsAllCharsets = FALSE;
                break;
            }
        }

        if(fSupportsAllCharsets)
        {
            int i;

            //
            // Yep. Add it to the list.
            //
            i = Font_AddNameToList(pEnumFontParam->hwndFontName, lpelf->elfLogFont.lfFaceName, lpelf->elfScript);
            if (i != CB_ERR)
            {
                //
                // Remember the font type and charset in the itemdata.
                //
                // LOWORD = Type
                // HIWORD = System Charset
                //
                SendMessage(pEnumFontParam->hwndFontName, CB_SETITEMDATA, (WPARAM)i, MAKELPARAM(Type, bSysCharset));
            }
        }
    }
    return 1;
}


void NEAR PASCAL Font_InitList(HWND hDlg)
{
    LOGFONT lf ;
    ENUMFONTPARAM   EnumFontParam;

    //
    // Enumerate all fonts on the system.
    // Font_EnumNames will filter out ones we don't want to show.
    //
    lf.lfFaceName[0] = TEXT('\0') ;
    lf.lfCharSet     = (BYTE)g_uiUniqueCharsets[SYSTEM_LOCALE_CHARSET]; //Use charset from the System Locale.
#ifdef WINDOWS_ME
    lf.lfPitchAndFamily = MONO_FONT;
#else
    lf.lfPitchAndFamily = 0;
#endif
    EnumFontParam.hwndFontName = GetDlgItem(hDlg, IDC_FONTNAME);
    EnumFontParam.hdc = GetDC(NULL);
    EnumFontFamiliesEx(EnumFontParam.hdc, &lf, (FONTENUMPROC)Font_EnumNames, (LPARAM)&EnumFontParam, 0);
    GetLastError();

    ReleaseDC(NULL, EnumFontParam.hdc);
}

void NEAR PASCAL Font_AddSize(HWND hwndPoints, int iNewPoint, BOOL bSort)
{
    TCHAR szBuf[10];
    int i, iPoint, count;

    // find the sorted place for this point size
    if (bSort)
    {
        count = (int)SendMessage(hwndPoints, CB_GETCOUNT, 0, 0L);
        for (i=0; i < count; i++)
        {
            iPoint = LOWORD(SendMessage(hwndPoints, CB_GETITEMDATA, (WPARAM)i, 0L));

            // don't add duplicates
            if (iPoint == iNewPoint)
                return;

            // belongs before this one
            if (iPoint > iNewPoint)
                break;
        }
    }
    else
        i = -1;

    wsprintf(szBuf, TEXT("%d"), iNewPoint);
    i = (int)SendMessage(hwndPoints, CB_INSERTSTRING, (WPARAM)i, (LPARAM)szBuf);
    if (i != CB_ERR)
        SendMessage(hwndPoints, CB_SETITEMDATA, (WPARAM)i, (LPARAM)iNewPoint);
}

// enumerate sizes for a non-TrueType font
int CALLBACK Font_EnumSizes(LPENUMLOGFONT lpelf, LPNEWTEXTMETRIC lpntm, int Type, LPARAM lData)
{
    HWND hwndFontSize = (HWND)lData;

    Font_AddSize(hwndFontSize, Font_HeightToPoint(lpntm->tmHeight - lpntm->tmInternalLeading), TRUE);
    return 1;
}
/*
** a new font name was chosen.  build a new point size list.
*/
void NEAR PASCAL Font_SelectName(HWND hDlg, int iSel)
{
    INT dwItemData;
    HWND hwndFontSize = GetDlgItem(hDlg, IDC_FONTSIZE);
    HDC hdc;

    // build the approriate point size list
    SendMessage(hwndFontSize, CB_RESETCONTENT, 0, 0L);
    dwItemData = LOWORD(SendDlgItemMessage(hDlg, IDC_FONTNAME, CB_GETITEMDATA, (WPARAM)iSel, 0L));
    if (LOWORD(dwItemData) == TRUETYPE_FONTTYPE)
    {
        INT i;
        for (i = 6; i <= 24; i++)
            Font_AddSize(hwndFontSize, i, FALSE);
    }
    else
    {
        LOGFONT lf;

        Font_GetNameFromList(GetDlgItem(hDlg, IDC_FONTNAME),
                             iSel,
                             lf.lfFaceName,
                             ARRAYSIZE(lf.lfFaceName),
                             NULL,
                             0);
        hdc = GetDC(NULL);
        lf.lfCharSet = (BYTE)(HIWORD(dwItemData));
#ifdef WINDOWS_ME
        lf.lfPitchAndFamily = MONO_FONT;
#else
        lf.lfPitchAndFamily = 0;
#endif
        EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)Font_EnumSizes, (LPARAM)hwndFontSize, 0);
        ReleaseDC(NULL, hdc);
    }
}

// new font was chosen.  select the proper point size
// return: actual point size chosen
int NEAR PASCAL Font_SelectSize(HWND hDlg, int iCurPoint)
{
    int i, iPoint;
    HWND hwndFontSize = GetDlgItem(hDlg, IDC_FONTSIZE);

    i = (int)SendMessage(hwndFontSize, CB_GETCOUNT, 0, 0L);

    // the loop stops with i=0, so we get some selection for sure
    for (i--; i > 0; i--)
    {
        iPoint = LOWORD(SendMessage(hwndFontSize, CB_GETITEMDATA, (WPARAM)i, 0L));
        // walking backwards through list, find equal or next smallest
        if (iCurPoint >= iPoint)
            break;
    }
    SendMessage(hwndFontSize, CB_SETCURSEL, (WPARAM)i, 0L);

    return iPoint;
}

/*
** a new element was picked, resulting in needing to set up a new font.
*/
void NEAR PASCAL Font_NewFont(HWND hDlg, int iFont)
{
    int iSel;
    BOOL bBold;

    // find the name in the list and select it
    iSel = Font_FindInList(GetDlgItem(hDlg, IDC_FONTNAME),
                           g_fonts[iFont].lf.lfFaceName);

    SendDlgItemMessage(hDlg, IDC_FONTNAME, CB_SETCURSEL, (WPARAM)iSel, 0L);
    Font_SelectName(hDlg, iSel);

    Font_SelectSize(hDlg, Font_HeightToPoint(g_fonts[iFont].lf.lfHeight));

    // REVIEW: should new size (returned above) be set in logfont?

    CheckDlgButton(hDlg, IDC_FONTITAL, g_fonts[iFont].lf.lfItalic);

    if (g_fonts[iFont].lf.lfWeight > FW_MEDIUM)
        bBold = TRUE;
    else
        bBold = FALSE;
    CheckDlgButton(hDlg, IDC_FONTBOLD, bBold);
}

/*
** enable/disable the font selection controls.
** also involves blanking out anything meaningful if disabling.
*/
void NEAR PASCAL Font_EnableControls(HWND hDlg, BOOL bEnable)
{
    if (!bEnable)
    {
        SendDlgItemMessage(hDlg, IDC_FONTNAME, CB_SETCURSEL, (WPARAM)-1, 0L);
        SendDlgItemMessage(hDlg, IDC_FONTSIZE, CB_SETCURSEL, (WPARAM)-1, 0L);
        CheckDlgButton(hDlg, IDC_FONTITAL, 0);
        CheckDlgButton(hDlg, IDC_FONTBOLD, 0);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_FONTNAME), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FONTSIZE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FONTSIZELABEL), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FONTLABEL), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FONTBOLD), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FONTITAL), bEnable);
}

//--------end------------- mini font picker controls --------------------------

/*
**
*/
void NEAR PASCAL Color_SetColor(HWND hDlg, int id, HBRUSH hbrColor)
{
    HWND hwndItem;
    switch (id)
    {
        case IDC_GRADIENT:
            g_hbrGradientColor = hbrColor;
            break;

        case IDC_MAINCOLOR:
            g_hbrMainColor = hbrColor;
            break;

        case IDC_TEXTCOLOR:
            g_hbrTextColor = hbrColor;
            break;

        default:
            return;

    }

    hwndItem = GetDlgItem(hDlg, id);
    if (hwndItem)
    {
        InvalidateRect(hwndItem, NULL, FALSE);
        UpdateWindow(hwndItem);
    }
}

/*
**
*/
void Color_PickAColor( HWND hDlg, int CtlID )
{
        COLORPICK_INFO cpi;
    int iColor;

    switch (CtlID)
    {
        case IDC_GRADIENT:
            iColor = ELCUR.iGradientColor;
            break;

        case IDC_MAINCOLOR:
            iColor = ELCUR.iMainColor;
            break;

        case IDC_TEXTCOLOR:
            iColor = ELCUR.iTextColor;
            break;

        default:
            return;

    }

    cpi.hwndParent = GetParent( hDlg );         // Property Sheet
    cpi.hwndOwner = GetDlgItem( hDlg, CtlID );  // Color button
    cpi.hpal = g_hpal3D;
    cpi.rgb = g_rgb[iColor];
    cpi.flags = CC_RGBINIT | CC_FULLOPEN;

    if ((iColor == COLOR_3DFACE) && g_bPalette)
    {
        cpi.flags |= CC_ANYCOLOR;
    }
    else if (g_colorFlags[iColor] & COLORFLAG_SOLID)
    {
        cpi.flags |= CC_SOLIDCOLOR;
    }

    if (ChooseColorMini( &cpi ) && Look_ChangeColor(hDlg, iColor, cpi.rgb))
    {
        Color_SetColor(hDlg, CtlID, g_brushes[iColor]);
        Look_Repaint(hDlg, FALSE);
    }
}

void NEAR PASCAL DrawDownArrow(HDC hdc, LPRECT lprc, BOOL bDisabled)
{
    HBRUSH hbr;
    int x, y;

    x = lprc->right - cxEdge - 5;
    y = lprc->top + ((lprc->bottom - lprc->top)/2 - 1);

    if (bDisabled)
    {
        hbr = GetSysColorBrush(COLOR_3DHILIGHT);
        hbr = SelectObject(hdc, hbr);

        x++;
        y++;
        PatBlt(hdc, x, y, 5, 1, PATCOPY);
        PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
        PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

        SelectObject(hdc, hbr);
        x--;
        y--;
    }
    hbr = GetSysColorBrush(bDisabled ? COLOR_3DSHADOW : COLOR_BTNTEXT);
    hbr = SelectObject(hdc, hbr);

    PatBlt(hdc, x, y, 5, 1, PATCOPY);
    PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
    PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

    SelectObject(hdc, hbr);
    lprc->right = x;
}

/*
** draw the color combobox thing
**
** also, if button was depressed, popup the color picker
*/
void NEAR PASCAL Color_DrawButton(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
    SIZE thin = { cxEdge / 2, cyEdge / 2 };
    RECT rc = lpdis->rcItem;
    HDC hdc = lpdis->hDC;
    BOOL bFocus = ((lpdis->itemState & ODS_FOCUS) &&
        !(lpdis->itemState & ODS_DISABLED));

    if (!thin.cx) thin.cx = 1;
    if (!thin.cy) thin.cy = 1;

    if (lpdis->itemState & ODS_SELECTED)
    {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
        OffsetRect(&rc, 1, 1);
    }
    else
        DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT | BF_ADJUST);

    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

    if (bFocus)
    {
        InflateRect(&rc, -thin.cx, -thin.cy);
        DrawFocusRect(hdc, &rc);
        InflateRect(&rc, thin.cx, thin.cy);
    }

    InflateRect(&rc, 1-thin.cx, -cyEdge);

    rc.left += cxEdge;
    DrawDownArrow(hdc, &rc, lpdis->itemState & ODS_DISABLED);

    InflateRect(&rc, -thin.cx, 0);
    DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RIGHT);

    rc.right -= ( 2 * cxEdge ) + thin.cx;

    // color sample
    if ( !(lpdis->itemState & ODS_DISABLED) )
    {
        HPALETTE hpalOld = NULL;
        HBRUSH hbr = 0;

        switch (lpdis->CtlID)
        {
            case IDC_GRADIENT:
                hbr = g_hbrGradientColor;
                break;

            case IDC_MAINCOLOR:
                hbr = g_hbrMainColor;
                break;

            case IDC_TEXTCOLOR:
                hbr = g_hbrTextColor;
                break;

        }

        FrameRect(hdc, &rc, GetSysColorBrush(COLOR_BTNTEXT));
        InflateRect(&rc, -thin.cx, -thin.cy);

        if (g_hpal3D)
        {
            hpalOld = SelectPalette(hdc, g_hpal3D, FALSE);
            RealizePalette(hdc);
        }

        if (hbr)
        {
            hbr = SelectObject(hdc, hbr);
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
            SelectObject(hdc, hbr);
        }
        
        if (hpalOld)
        {
            SelectPalette(hdc, hpalOld, TRUE);
            RealizePalette(hdc);
        }
    }
}


//--------end------------- color stuff --------------------------------------

//------------------------ scheme stuff --------------------------------------
/*
**
*/
void NEAR PASCAL Scheme_Init(HWND hwndSchemes)
{
    HKEY hkSchemes;
    DWORD dw, dwSize;
    TCHAR szBuf[MAXSCHEMENAME];
    int id;

    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) != ERROR_SUCCESS)
        return;

    for (dw=0; ; dw++)
    {
        dwSize = ARRAYSIZE(szBuf);
        if (RegEnumValue(hkSchemes, dw, szBuf, &dwSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            break;  // Bail if no more values

        SendMessage(hwndSchemes, CB_ADDSTRING, 0, (LPARAM)szBuf);
    }
    RegCloseKey(hkSchemes);

    g_szCurScheme[0] = TEXT('\0');
    g_szLastScheme[0] = TEXT('\0');

    // select the current scheme
    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, &hkSchemes) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuf);
        if (RegQueryValueEx(hkSchemes, REGSTR_KEY_CURRENT, NULL, NULL, (LPBYTE)szBuf, &dwSize) == ERROR_SUCCESS)
        {
            id = (int)SendMessage(hwndSchemes, CB_FINDSTRINGEXACT, 0, (LPARAM)szBuf);
            if (id != CB_ERR)
            {
                SendMessage(hwndSchemes, CB_SETCURSEL, (WPARAM)id, 0L);
                lstrcpy(g_szCurScheme, szBuf);
                lstrcpy(g_szLastScheme, szBuf);
            }
        }

        // also, since this key is already open, get the custom colors
        dwSize = sizeof(g_CustomColors);
        dw = REG_BINARY;
        if (RegQueryValueEx(hkSchemes, REGSTR_VAL_CUSTOMCOLORS, NULL, &dw,
                        (LPBYTE)g_CustomColors, &dwSize) != ERROR_SUCCESS)
        {
            // if no colors are there, initialize to all white
            for (id = 0; id < 16; id++)
                g_CustomColors[id] = RGB(255, 255, 255);
        }

        RegCloseKey(hkSchemes);
    }
}

void NEAR PASCAL Scheme_SetCurrentScheme(LPTSTR szCurScheme)
{
    HKEY hkSchemes;

    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, &hkSchemes) == ERROR_SUCCESS)
    {
        if (*szCurScheme)
            RegSetValueEx(hkSchemes, REGSTR_KEY_CURRENT, 0L, REG_SZ, (LPBYTE)szCurScheme, (lstrlen(szCurScheme)+1) * SIZEOF(TCHAR));
        else
            RegDeleteValue(hkSchemes, REGSTR_KEY_CURRENT);

        RegCloseKey(hkSchemes);
    }
}

BOOL NEAR PASCAL Scheme_SaveScheme(LPCTSTR lpszName)
{
    BOOL bRet = FALSE;
    SCHEMEDATA sd;
    HKEY hkSchemes;
    int i;

    if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS)
    {
#ifdef WINNT
        sd.version = SCHEME_VERSION;
#else
        sd.version = SCHEME_VERSION_WIN32;
#endif
        sd.wDummy = 0;
        GetMyNonClientMetrics(&sd.ncm);
        sd.lfIconTitle = g_fonts[FONT_ICONTITLE].lf;

        RemoveDPIchangeFromSchemeData(sd);
            
        for (i = 0; i < COLOR_MAX; i++)
        {
            sd.rgb[i] = g_rgb[i];
        }

        if (RegSetValueEx(hkSchemes, lpszName, 0L, REG_BINARY, (LPBYTE)&sd, sizeof(sd)) == ERROR_SUCCESS)
            bRet = TRUE;

        RegCloseKey(hkSchemes);
    }
    return bRet;
}

void NEAR PASCAL Scheme_DeleteScheme(LPTSTR lpszName)
{
    HKEY hkSchemes;

    if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS)
    {
        RegDeleteValue(hkSchemes, lpszName);
        RegCloseKey(hkSchemes);
    }
}

void Look_GetDefaultFontName(LPTSTR pszDefFontName, DWORD dwSize)
{
    HKEY    hkDefFont;
    
    // Read the "DefaultFontName" to be used.
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     c_szRegDefFontPath,
                                     0,
                                     KEY_READ,
                                     &hkDefFont) == ERROR_SUCCESS) 
    {
        DWORD dwType = REG_SZ;
        
        if(RegQueryValueEx(hkDefFont,
                        c_szRegDefFontName,
                        NULL,
                        &dwType,
                        (LPBYTE) pszDefFontName,
                        &dwSize) != ERROR_SUCCESS)
        {
            //Value is not there in the registry; Use "Tahoma" as the default name.
            lstrcpy(pszDefFontName, c_szTahoma);
        }
        RegCloseKey(hkDefFont);
    }
}

BOOL    DoesFontSupportAllCharsets(HDC hdc, LPLOGFONT    plf, UINT uiUniqueCharsets[], int iCountUniqueCharsets)
{
    int j;
    
    //The given font supports the system charset; Let's check if it supports the other charsets
    for(j = 0; j < iCountUniqueCharsets; j++)
    {
        plf->lfCharSet = (BYTE)uiUniqueCharsets[j];  //Let's try the next charset in the array.
        if(EnumFontFamiliesEx(hdc, plf, (FONTENUMPROC)Font_EnumValidCharsets, (LPARAM)0, 0) != 0)
        {
            // EnumFontFamiliesEx would have returned a zero if Font_EnumValidCharsets was called
            // even once. In other words, it returned a non-zero because not even a single font existed
            // that supported the given charset.
            return FALSE;
        }
    }

    return TRUE; //Yes this font supports all the charsets we are interested in.
}

//
// Given an array of fonts and an array of unique charsets, this function checks if the fonts 
// support ALL these charsets.
// Returns TRUE if these fonts support all the charsets.
// if input parameter "fSilent" is TRUE, this function will ask the end-user if it is OK to change
// the font to "Tahoma". If the end-user says "No", then this function will return FALSE.
// In all other cases, this function will return TRUE. If the fonts need to be changed to support
// the given charsets, this function does all those changes.
//
//  lpszName is the name of the scheme to be used in the MessageBox that appears if fSilent is FALSE.
// 

BOOL Look_CheckFontsCharsets(LOGFONT lfUIFonts[], int iCountFonts, 
                        UINT uiCurUniqueCharsets[], int iCountCurUniqueCharsets, 
                        BOOL fSilent, BOOL *pfNewName, BOOL *pfDirty, LPCTSTR lpszName)
{
    int i;
    TCHAR   szDefaultFontFaceName[LF_FACESIZE];
    HDC     hdc;

    *pfNewName = FALSE; //Assume there is no need to save this scheme under a new name
    *pfDirty   = FALSE; //Assume that this scheme does not need to be saved.
    
    //Read the default font name from the registry (Mostly: Tahoma)
    Look_GetDefaultFontName(szDefaultFontFaceName, ARRAYSIZE(szDefaultFontFaceName));

    hdc = GetDC(NULL);
    
    //Check to see of the fonts support the system charset
    for(i = 0; i < iCountFonts; i++)
    {
        //Save the current charset because DoesFontSupportAllCharsets() destroys this field.
        BYTE    bCurCharset = lfUIFonts[i].lfCharSet;  

        if(!DoesFontSupportAllCharsets(hdc, &lfUIFonts[i], uiCurUniqueCharsets, iCountCurUniqueCharsets))
        {
            // This font does not support all the charsets we are interested in.
            // So, ask the end-user if it is OK to change the font to "Tahoma"
            if(!fSilent)
            {
                TCHAR   szMessage[1024];
                TCHAR   szFormated[1024];
                TCHAR   szTitle[128];
                TCHAR   szCopyOfTemplate[40];           // To load "Copy of %s" internationalized string.
                TCHAR   szNewSchemeName[MAXSCHEMENAME];  //To save the new scheme name

                LoadString(hInstance, IDS_SCHEME_WARNING, szTitle, ARRAYSIZE(szTitle));
                LoadString(hInstance, IDS_FONTCHANGE_IN_SCHEME, szMessage, ARRAYSIZE(szMessage));

                //Form the new "Copy of old-name" string in szNewSchemeName.
                LoadString(hInstance, IDS_COPYOF_SCHEME, szCopyOfTemplate, ARRAYSIZE(szCopyOfTemplate));
                wsprintf(szNewSchemeName, szCopyOfTemplate, lpszName);
                
                wsprintf(szFormated, szMessage, lpszName, szNewSchemeName);
                
                if(MessageBox(g_hDlg, szFormated, szTitle, MB_OKCANCEL) == IDCANCEL)
                {
                    ReleaseDC(NULL, hdc);
                    
                    return FALSE;  //We can not set the scheme.
                }
                    
                fSilent = TRUE; //We asked once; Don't ask again.
            }

            //Copy the default fontname to the font.
            lstrcpy(lfUIFonts[i].lfFaceName, szDefaultFontFaceName);
            *pfNewName = TRUE;  //Yes! This scheme's font has changed; So, we need to save it under a new name.
            *pfDirty = TRUE;  //This scheme needs to be saved.
        }

        //Restore the charset because DoesFontSupportAllCharsets() destroyed this field.
        lfUIFonts[i].lfCharSet = bCurCharset; // Restore the current charset.
        
        // Warning #1: The IconTitle font's Charset must always match the System Locale charset.
        // Warning #2: FoxPro's tooltips code expects the Status font's charset to the the System 
        // Locale's charset.
        // As per intl guys, we set the charset of all the UI fonts to SYSTEM_LOCALE_CHARSET.
        if(lfUIFonts[i].lfCharSet != uiCurUniqueCharsets[SYSTEM_LOCALE_CHARSET])
        {
            lfUIFonts[i].lfCharSet = (BYTE)uiCurUniqueCharsets[SYSTEM_LOCALE_CHARSET];
            *pfDirty = TRUE;
        }
    }  //For loop 

    ReleaseDC(NULL, hdc);

    return TRUE;  //The fonts have been modified as required.
}

BOOL Look_CheckSchemeCharsets(SCHEMEDATA  *psd, LPCTSTR lpszName, UINT uiUniqueCharsets[], int iCountCharsets, BOOL fSilent, BOOL *pfNewName, BOOL *pfDirty)
{
    LOGFONT     lfUIFonts[NUM_FONTS]; //Make a local copy of the fonts.
    BOOL        fRet = FALSE;

    *pfNewName = FALSE; //Assume there is no need to save this scheme under a new name
    *pfDirty   = FALSE; //Assume that this scheme does not need to be saved.
    
    //Make a copy of the ncm fonts into the local array.
    LF32toLF(&(psd->ncm.lfCaptionFont), &(lfUIFonts[FONT_CAPTION]));
    LF32toLF(&(psd->ncm.lfSmCaptionFont), &(lfUIFonts[FONT_SMCAPTION]));
    LF32toLF(&(psd->ncm.lfMenuFont), &(lfUIFonts[FONT_MENU]));
    LF32toLF(&(psd->ncm.lfStatusFont), &(lfUIFonts[FONT_STATUS]));
    LF32toLF(&(psd->ncm.lfMessageFont), &(lfUIFonts[FONT_MSGBOX]));
    //Make a copy of the icon title font
    LF32toLF(&(psd->lfIconTitle), &(lfUIFonts[FONT_ICONTITLE]));

    fRet = Look_CheckFontsCharsets(lfUIFonts, NUM_FONTS, uiUniqueCharsets, iCountCharsets, 
                                fSilent, pfNewName, pfDirty, lpszName);

    //Let's copy the data back into the SchemeData structure
    if(*pfDirty)
    {
        LFtoLF32(&(lfUIFonts[FONT_CAPTION]), &(psd->ncm.lfCaptionFont));
        LFtoLF32(&(lfUIFonts[FONT_SMCAPTION]), &(psd->ncm.lfSmCaptionFont));
        LFtoLF32(&(lfUIFonts[FONT_MENU]), &(psd->ncm.lfMenuFont));
        LFtoLF32(&(lfUIFonts[FONT_STATUS]), &(psd->ncm.lfStatusFont));
        LFtoLF32(&(lfUIFonts[FONT_MSGBOX]), &(psd->ncm.lfMessageFont));
        //Make a copy of the icon title font
        LFtoLF32(&(lfUIFonts[FONT_ICONTITLE]), &(psd->lfIconTitle));
    }

    return (fRet); //It is alright to use this scheme stored in *psd. 
}

BOOL NEAR PASCAL Scheme_SetScheme(LPCTSTR lpszName, BOOL fSilent, BOOL *pfNewName, BOOL *pfDirty)
{
    BOOL bRet = FALSE;
    SCHEMEDATA sd;
    HKEY hkSchemes;
    int i;
    DWORD dwType, dwSize;

    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS)
    {
        dwType = REG_BINARY;
        dwSize = sizeof(sd);
        if (RegQueryValueEx(hkSchemes, lpszName, NULL, &dwType, (LPBYTE)&sd, &dwSize) == ERROR_SUCCESS)
        {
            int n;
            
            *pfDirty = FALSE;
#ifdef WINNT
            if (sd.version != SCHEME_VERSION)
            {
                RegCloseKey(hkSchemes);
                return FALSE;
            }
#else
            // 16 -- bit version, need to convert. This is a Memphis only issue
            if (sd.version == SCHEME_VERSION  || sd.version == SCHEME_VERSION_400)
            {
                ThunkSchemeDataFrom16To32(&sd, dwSize);
            }
            else if (sd.version != SCHEME_VERSION_WIN32)
            {
                RegCloseKey(hkSchemes);
                return FALSE;
            }
#endif      
            // See if the fonts in the new scheme support the system charset and if not
            // ask the user if it is ok to change it to Tahoma.
            if(!Look_CheckSchemeCharsets(&sd, lpszName, g_uiUniqueCharsets, g_iCountCharsets, fSilent, pfNewName, pfDirty))
                return FALSE;   //User did not agree and this scheme can not be used.
                
            n = (int)(dwSize - (sizeof(sd) - sizeof(sd.rgb))) / sizeof(COLORREF);

            //If the DPI value was changed through the "general" tab, apply it here.
            ApplyDPIchangeToSchemeData(sd);
                
            SetMyNonClientMetrics(&sd.ncm);

            g_fonts[FONT_ICONTITLE].lf = sd.lfIconTitle;

            for (i = 0; i < min(n,COLOR_MAX); i++)
            {
                g_rgb[i] = sd.rgb[i];
            }
            if (n == COLOR_MAX_400)
            {
                Look_Get3DRules();
                g_rgb[COLOR_HOTLIGHT] = g_rgb[COLOR_ACTIVECAPTION];
                g_rgb[COLOR_GRADIENTACTIVECAPTION] = RGB(0,0,0);
                g_rgb[COLOR_GRADIENTINACTIVECAPTION] = RGB(0,0,0);
            }

            Look_RebuildSysStuff(TRUE);
            bRet = TRUE;
        }

        RegCloseKey(hkSchemes);
    }
    return bRet;
}

//--------end------------- scheme stuff --------------------------------------

// ----------------------------------------------------------------------------
//
//  SaveSchemeDlgProc() -
//
//  Dialog to let user save current color scheme with a name in the win.ini.
//
// ----------------------------------------------------------------------------

const static DWORD FAR aSaveSchemeHelpIds[] = {
        IDC_NO_HELP_1,  IDH_DISPLAY_APPEARANCE_SAVEAS_DIALOG_TEXTBOX,
        IDC_SAVESCHEME, IDH_DISPLAY_APPEARANCE_SAVEAS_DIALOG_TEXTBOX,

        0, 0
};

static void RemoveBlanks(LPTSTR lpszString)
{
    LPTSTR lpszPosn;

    /* strip leading blanks */
    lpszPosn = lpszString;
    while(*lpszPosn == TEXT(' ')) {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */
    if ((lpszPosn=lpszString+lstrlen(lpszString)) != lpszString) {
        lpszPosn = CharPrev(lpszString, lpszPosn);
        while(*lpszPosn == TEXT(' '))
           lpszPosn = CharPrev(lpszString, lpszPosn);
        lpszPosn = CharNext(lpszPosn);
        *lpszPosn = TEXT('\0');
    }

}

INT_PTR CALLBACK SaveSchemeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuf[MAXSCHEMENAME];

    switch (message)
    {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDC_SAVESCHEME,  g_szLastScheme);
            SendDlgItemMessage(hDlg, IDC_SAVESCHEME, EM_SETSEL, 0, -1);
            SendDlgItemMessage(hDlg, IDC_SAVESCHEME, EM_LIMITTEXT, 45, 0L);
            EnableWindow(GetDlgItem(hDlg, IDOK), (g_szLastScheme[0] != TEXT('\0')));
            return(TRUE);

        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, TEXT("display.hlp"),
                HELP_WM_HELP, (DWORD_PTR)  aSaveSchemeHelpIds);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            WinHelp((HWND) wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
                (DWORD_PTR)  aSaveSchemeHelpIds);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_SAVESCHEME:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        GetDlgItemText(hDlg, IDC_SAVESCHEME, szBuf, 2);
                        if (*szBuf)
                            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                    }
                    break;

                case IDOK:
                    GetDlgItemText(hDlg, IDC_SAVESCHEME, szBuf, ARRAYSIZE(szBuf));
                    RemoveBlanks(szBuf);

                    if (!(*szBuf))
                    {
                        DeskShellMessageBox(hInstance, hDlg, MAKEINTRESOURCE(IDS_BLANKNAME), NULL, MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }

                    lstrcpy(g_szCurScheme, szBuf);
                    lstrcpy(g_szLastScheme, szBuf);
                    // FALL THRU

                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return(TRUE);
            }
            break;
    }
    return (FALSE);
}

/*
** initialize the constant dialog components
**
** initialize the list of element names.  this stays constant with the
** possible exception that some items might be added/removed depending
** on some special case conditions.
*/
void NEAR PASCAL Look_InitDialog(HWND hDlg)
{
    int iEl, iName;
    TCHAR szName[CCH_MAX_STRING];
    HWND hwndElements;
    LOGFONT lf;
    HFONT hfont;
    int oldWeight;
    HDC screendc;

    //
    // The system locale could have changed since lasttime we brought this up.
    // Make necessary changes based on locale changes, if any.
    UpdateCharsetChanges();  
    

    // initialize some globals
    cyBorder = GetSystemMetrics(SM_CYBORDER);
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);

    Scheme_Init(GetDlgItem(hDlg, IDC_SCHEMES));

    hwndElements = GetDlgItem(hDlg, IDC_ELEMENTS);
    for (iEl = 0; iEl < ARRAYSIZE(g_elements); iEl++)
    {
        if ((g_elements[iEl].iResId != -1) &&
                LoadString(hInstance, g_elements[iEl].iResId, szName, ARRAYSIZE(szName)))
        {
            iName = (int)SendMessage(hwndElements, CB_FINDSTRINGEXACT, 0, (LPARAM)szName);

            if (iName == CB_ERR)
                iName = (int)SendMessage(hwndElements, CB_ADDSTRING, 0, (LPARAM)szName);

            // reference back to item in array
            if (iName != CB_ERR)
                SendMessage(hwndElements, CB_SETITEMDATA, (WPARAM)iName, (LPARAM)iEl);
        }
    }

    // make bold button have bold text
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_FONTBOLD, WM_GETFONT, 0, 0L);
    GetObject(hfont, sizeof(lf), &lf);
    oldWeight = lf.lfWeight;
    lf.lfWeight = FW_BOLD;
    hfont = CreateFontIndirect(&lf);
    if (hfont)
        SendDlgItemMessage(hDlg, IDC_FONTBOLD, WM_SETFONT, (WPARAM)hfont, 0L);

    // make italic button have italic text
    lf.lfWeight = oldWeight;
    lf.lfItalic = TRUE;
    hfont = CreateFontIndirect(&lf);
    if (hfont)
        SendDlgItemMessage(hDlg, IDC_FONTITAL, WM_SETFONT, (WPARAM)hfont, 0L);

    if ((screendc = GetDC(NULL)) != NULL)
    {
        if ((GetDeviceCaps(screendc, PLANES) *
            GetDeviceCaps(screendc, BITSPIXEL)) < 8 )
        {
            g_bCrappyColor = TRUE;
        }

        ReleaseDC(NULL, screendc);
    }
}

void NEAR PASCAL Look_DestroyDialog(HWND hDlg)
{
    HFONT hfont, hfontOther;

    hfontOther = (HFONT)SendDlgItemMessage(hDlg, IDC_MAINSIZE, WM_GETFONT, 0, 0L);
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_FONTBOLD, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
        DeleteObject(hfont);
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_FONTITAL, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
        DeleteObject(hfont);
}

/*
** a new element has been chosen.
**
** iElement - index into g_elements of the chosen one
** bSetCur - if TRUE, need to find element in elements combobox, too
*/
#define LSE_SETCUR 0x0001
#define LSE_ALWAYS 0x0002

void FAR PASCAL Look_SelectElement(HWND hDlg, int iElement, DWORD dwFlags)
{
    BOOL bEnable;
    int i;
    BOOL bEnableGradient;
    BOOL bGradient = FALSE;

    if ((iElement == g_iCurElement) && !(dwFlags & LSE_ALWAYS))
        return;

    SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (PVOID)&bGradient, 0);

    g_iCurElement = iElement;

    // if needed, find this element in the combobox and select it
    if (dwFlags & LSE_SETCUR)
    {
        i = (int)SendDlgItemMessage(hDlg, IDC_ELEMENTS, CB_GETCOUNT,0,0L);
        for (i--; i >=0 ; i--)
        {
            // if this is the one that references our element, stop
            if (iElement == (int)LOWORD(SendDlgItemMessage(hDlg, IDC_ELEMENTS, CB_GETITEMDATA, (WPARAM)i, 0L)))
                break;
        }
        SendDlgItemMessage(hDlg, IDC_ELEMENTS, CB_SETCURSEL, (WPARAM)i,0L);
    }

    bEnable = (ELCUR.iMainColor != COLOR_NONE);
    if (bEnable)
        Color_SetColor(hDlg, IDC_MAINCOLOR, g_brushes[ELCUR.iMainColor]);
    EnableWindow(GetDlgItem(hDlg, IDC_MAINCOLOR), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_COLORLABEL), bEnable);

    bEnableGradient = ((ELCUR.iGradientColor != COLOR_NONE));

    if (bEnableGradient)
        Color_SetColor(hDlg, IDC_GRADIENT, g_brushes[ELCUR.iGradientColor]);
   
    EnableWindow(GetDlgItem(hDlg, IDC_GRADIENT), (bEnableGradient && bGradient));
    EnableWindow(GetDlgItem(hDlg, IDC_GRADIENTLABEL), (bEnableGradient && bGradient));

    bEnable = (ELCUR.iFont != FONT_NONE);
    if (bEnable)
    {
        Font_NewFont(hDlg, ELCUR.iFont);
    }
    Font_EnableControls(hDlg, bEnable);

    // size may be based on font
    Look_DoSizeStuff(hDlg, FALSE);

    bEnable = (ELCUR.iSize != SIZE_NONE);
    EnableWindow(GetDlgItem(hDlg, IDC_MAINSIZE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_SIZEARROWS), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_SIZELABEL), bEnable);

    bEnable = (ELCUR.iTextColor != COLOR_NONE);
    if (bEnable)
        Color_SetColor(hDlg, IDC_TEXTCOLOR, g_brushes[ELCUR.iTextColor]);
    EnableWindow(GetDlgItem(hDlg, IDC_TEXTCOLOR), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_FNCOLORLABEL), bEnable);
}

void NEAR PASCAL Look_Repaint(HWND hDlg, BOOL bRecalc)
{

    HWND hwndLookPrev;

    hwndLookPrev = GetDlgItem(hDlg, IDC_LOOKPREV);
    if (bRecalc)
    {
        Look_SyncSize(hDlg);
        LookPrev_Recalc(hwndLookPrev);
    }
    LookPrev_Repaint(hwndLookPrev);
}

void NEAR PASCAL Look_SetCurSizeAndRange(HWND hDlg)
{
    if (ELCUR.iSize == SIZE_NONE)
        SetDlgItemText(hDlg, IDC_MAINSIZE, TEXT(""));
    else
    {
        SendDlgItemMessage(hDlg, IDC_SIZEARROWS, UDM_SETRANGE, 0,
            MAKELPARAM(ELCURSIZE.MaxSize, ELCURSIZE.MinSize));
        SetDlgItemInt(hDlg, IDC_MAINSIZE, ELCURSIZE.CurSize, TRUE);
    }
}

void NEAR PASCAL Look_UpdateSizeBasedOnFont(HWND hDlg, BOOL fComputeIdeal)
{
    if ((ELCUR.iSize != SIZE_NONE) && (ELCUR.iFont != FONT_NONE))
    {
        TEXTMETRIC tm;
        HFONT hfontOld = SelectObject(g_hdcMem, ELCURFONT.hfont);

        GetTextMetrics(g_hdcMem, &tm);
        if (ELCUR.iSize == SIZE_MENU)
        {
            // Include external leading for menus
            tm.tmHeight += tm.tmExternalLeading;
        }

        if (hfontOld)
            SelectObject(g_hdcMem, hfontOld);

        ELCURSIZE.MinSize = tm.tmHeight + 2 * cyBorder;

        if (fComputeIdeal)
        {
            if ((ELCUR.iSize == SIZE_CAPTION || ELCUR.iSize == SIZE_MENU) &&
                (ELCURSIZE.MinSize <
                (GetSystemMetrics(SM_CYICON)/2 + 2 * cyBorder)))
            {
                ELCURSIZE.CurSize =
                    GetSystemMetrics(SM_CYICON)/2 + 2 * cyBorder;
            }
            else
                ELCURSIZE.CurSize = ELCURSIZE.MinSize;
        }
        else if (ELCURSIZE.CurSize < ELCURSIZE.MinSize)
            ELCURSIZE.CurSize = ELCURSIZE.MinSize;
    }
}

void NEAR PASCAL
Look_SyncSize(HWND hDlg)
{
    if (g_iPrevSize != SIZE_NONE)
        g_sizes[g_iPrevSize].CurSize = g_elCurSize.CurSize;

    if (g_iCurElement >= 0)
        g_iPrevSize = ELCUR.iSize;
}

void NEAR PASCAL
Look_DoSizeStuff(HWND hDlg, BOOL fCanSuggest)
{
    Look_SyncSize(hDlg);

    if (ELCUR.iSize != SIZE_NONE)
    {
        ELCURSIZE = g_sizes[ELCUR.iSize];

        if (ELCUR.fLinkSizeToFont)
            Look_UpdateSizeBasedOnFont(hDlg, fCanSuggest);

        if (ELCURSIZE.CurSize < ELCURSIZE.MinSize)
        {
            ELCURSIZE.CurSize = ELCURSIZE.MinSize;
        }
        else if (ELCURSIZE.CurSize > ELCURSIZE.MaxSize)
        {
            ELCURSIZE.CurSize = ELCURSIZE.MaxSize;
        }
    }

    Look_SetCurSizeAndRange(hDlg);
}

void NEAR PASCAL Look_RebuildCurFont(HWND hDlg)
{
    if (ELCURFONT.hfont)
        DeleteObject(ELCURFONT.hfont);
    ELCURFONT.hfont = CreateFontIndirect(&ELCURFONT.lf);

    Look_DoSizeStuff(hDlg, TRUE);
    Look_Repaint(hDlg, TRUE);
}

void NEAR PASCAL Look_Changed(HWND hDlg, UINT fChange)
{
    if (g_bInit)
        return;

    if(( fChange != SCHEME_CHANGE ) && (fChange != DPI_CHANGE))
    {
        *g_szCurScheme = 0;

        if (hDlg)
        {
            SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_SETCURSEL,
                (WPARAM)-1, 0L);
        }
    }
    else
    {
        fChange = METRIC_CHANGE | COLOR_CHANGE;
    }

    g_fChanged |= fChange;

    if (hDlg)
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
}

void NEAR PASCAL Look_ChangeFontName(HWND hDlg, LPCTSTR szBuf, INT iCharSet)
{
    if (lstrcmpi(ELCURFONT.lf.lfFaceName, szBuf) == 0 &&
        ELCURFONT.lf.lfCharSet == iCharSet)
    {
        return;
    }

    lstrcpy(ELCURFONT.lf.lfFaceName, szBuf);
    ELCURFONT.lf.lfCharSet = (BYTE)iCharSet;

    Look_RebuildCurFont(hDlg);
    Look_Changed(hDlg, METRIC_CHANGE);
}

void NEAR PASCAL Look_ChangeFontSize(HWND hDlg, int Points)
{
    if (ELCURFONT.lf.lfHeight != Font_PointToHeight(Points))
    {
        ELCURFONT.lf.lfHeight = Font_PointToHeight(Points);
        Look_RebuildCurFont(hDlg);
        Look_Changed(hDlg, METRIC_CHANGE);
    }
}
void NEAR PASCAL Look_ChangeFontBI(HWND hDlg, int id, BOOL bCheck)
{
    if (id == IDC_FONTBOLD) // bold
    {
        if (bCheck)
            ELCURFONT.lf.lfWeight = FW_BOLD;
        else
            ELCURFONT.lf.lfWeight = FW_NORMAL;
    }
    else   // italic
    {
        ELCURFONT.lf.lfItalic = (BYTE)bCheck;
    }

    Look_RebuildCurFont(hDlg);
    Look_Changed(hDlg, METRIC_CHANGE);
}

void NEAR PASCAL Look_ChangeSize(HWND hDlg, int NewSize, BOOL bRepaint)
{
    if (ELCURSIZE.CurSize != NewSize)
    {
        ELCURSIZE.CurSize = NewSize;
        if (bRepaint)
            Look_Repaint(hDlg, TRUE);
        Look_Changed(hDlg, METRIC_CHANGE);
    }
}

BOOL NEAR PASCAL Look_ChangeColor(HWND hDlg, int iColor, COLORREF rgb)
{
    COLORREF rgbShadow, rgbHilight, rgbWatermark;

    if ((rgb & 0x00FFFFFF) == (g_rgb[iColor] & 0x00FFFFFF))
        return FALSE;

    if ( iColor == COLOR_3DFACE )
    {
        Look_Get3DRules();
        rgbShadow    = AdjustLuma(rgb, g_i3DShadowAdj,  g_fScale3DShadowAdj );
        rgbHilight   = AdjustLuma(rgb, g_i3DHilightAdj, g_fScale3DHilightAdj);
        rgbWatermark = AdjustLuma(rgb, g_iWatermarkAdj, g_fScaleWatermarkAdj);


        Set3DPaletteColor(rgb, COLOR_3DFACE);
        Set3DPaletteColor(rgbShadow, COLOR_3DSHADOW);
        Set3DPaletteColor(rgbHilight, COLOR_3DHILIGHT);

        // update colors tagged to 3DFACE
        g_rgb[COLOR_3DFACE] = rgb;
        g_rgb[COLOR_3DLIGHT] =  rgb; // BOGUS TEMPORARY
        g_rgb[COLOR_ACTIVEBORDER] =  rgb;
        g_rgb[COLOR_INACTIVEBORDER] =  rgb;
        g_rgb[COLOR_MENU] =  rgb;

        // update colors tagged to 3DSHADOW
        g_rgb[COLOR_GRAYTEXT] = rgbShadow;
        g_rgb[COLOR_APPWORKSPACE] = rgbShadow;
        g_rgb[COLOR_3DSHADOW] = rgbShadow;
        g_rgb[COLOR_INACTIVECAPTION] = rgbShadow;

        // update colors tagged to 3DHIGHLIGHT
        g_rgb[COLOR_3DHILIGHT] = rgbHilight;
        g_rgb[COLOR_SCROLLBAR] = rgbHilight;

        if ((g_rgb[COLOR_SCROLLBAR] & 0x00FFFFFF) ==
            (g_rgb[COLOR_WINDOW] & 0x00FFFFFF))
        {
            g_rgb[COLOR_SCROLLBAR] = RGB( 192, 192, 192 );
        }
    }
    else
        g_rgb[iColor] = rgb;

    Look_RebuildSysStuff(FALSE);
    Look_Changed(hDlg, COLOR_CHANGE);
    return TRUE;
}

//
// szBuf is name of scheme in registry
//
BOOL NEAR PASCAL Look_ChooseScheme(LPCTSTR szName)
{
    BOOL fNewSchemeName = FALSE;
    BOOL fDirty = FALSE;

    //
    // NOTE: We pass fSilent = TRUE in the following call. So, if the fonts need to be changed
    // to match the current charsets, it will be done silently without asking the end-user.
    //
    if (Scheme_SetScheme(szName, TRUE, &fNewSchemeName, &fDirty))  //TRUE => Do it silently!
    {
        if (g_hDlg)
        {
            int i;

            // prevent using value in edit box
            g_iPrevSize = SIZE_NONE;

            // force repaint of element information
            i = g_iCurElement;
            g_iCurElement = -1;
            Look_SelectElement(g_hDlg, i, 0);

            // repaint preview
            Look_Repaint(g_hDlg, TRUE);
        }

        // remember new current scheme
        lstrcpy(g_szCurScheme, szName);
        lstrcpy(g_szLastScheme, szName);

        Look_Changed(g_hDlg, SCHEME_CHANGE);
        
        if(fNewSchemeName || fDirty)
        {
            int    i;
            //Save charset changes
            if (Scheme_SaveScheme(szName) && g_hDlg)
            {
                //See if this name already exists.
                i = (int)SendDlgItemMessage(g_hDlg, IDC_SCHEMES, CB_FINDSTRINGEXACT, 0, (LPARAM)g_szCurScheme);
                if (i == CB_ERR) //if it doesn't exist, add it!
                    i = (int)SendDlgItemMessage(g_hDlg, IDC_SCHEMES, CB_ADDSTRING, 0, (LPARAM)g_szCurScheme);
                //Select the newly saved scheme.
                SendDlgItemMessage(g_hDlg, IDC_SCHEMES, CB_SETCURSEL, (WPARAM)i, 0L);
            }
        }
        return TRUE;
    }
    return FALSE;
}

void NEAR PASCAL Look_ApplyChanges(void)
{
    if (g_fChanged)
    {
        HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));
        HWND cover;
#if 1
        cover = CreateCoverWindow(COVER_NOPAINT);
#else
        // for debugging
        cover = NULL;
#endif

        Look_SetSysStuff(g_fChanged);
        g_fChanged = 0;

        if (cover)
            PostMessage(cover, WM_CLOSE, 0, 0L);

        SetCursor(old);
    }

    Scheme_SetCurrentScheme(g_szCurScheme);
}

#ifdef READ_3D_RULES_FROM_REGISTRY

static const char g_szDeskAppletSoftwareKey[]= REGSTR_PATH_CONTROLSFOLDER TEXT("\\Display");
static const char g_szShadowAdjust[] = TEXT("Shadow Adjust");
static const char g_szWatermarkAdjust[] = TEXT("Watermark Adjust");
static const char g_szHilightAdjust[] = TEXT("Hilight Adjust");

void NEAR PASCAL
Look_Load3DRatio(HKEY hk, LPCSTR szItem, int FAR *piValue, BOOL FAR *pfScale)
{
    char szAdjust[32];
    DWORD dwType, dwBytes = sizeof(szAdjust);

    if ((RegQueryValueEx(hk, szItem, NULL, &dwType, (LPBYTE)(LPSTR)szAdjust,
        &dwBytes) == ERROR_SUCCESS) && (dwType == REG_SZ) && *szAdjust)
    {
        char FAR *p = szAdjust;
        BOOL fScale;
        int iValue;

        if (*p == '#')
        {
            fScale = FALSE;
            p++;
        }
        else
            fScale = TRUE;

        iValue = (int)MyStrToLong(p);

        if (iValue)
        {
            *piValue = iValue;
            *pfScale = fScale;
        }
    }
}

void NEAR PASCAL Look_Get3DRules(void)
{
    HKEY hk;

    if ((g_i3DShadowAdj != 0) &&
        (g_i3DHilightAdj != 0) &&
        (g_iWatermarkAdj != 0))
    {
        return;
    }

    if (RegOpenKey(HKEY_LOCAL_MACHINE, g_szDeskAppletSoftwareKey, &hk) !=
        ERROR_SUCCESS)
    {
        hk = NULL;
    }

    if (g_i3DShadowAdj == 0)
    {
        g_i3DShadowAdj = -333;
        g_fScale3DShadowAdj = TRUE;

        if (hk)
        {
            Look_Load3DRatio(hk, g_szShadowAdjust,
                &g_i3DShadowAdj, &g_fScale3DShadowAdj);
        }
    }

    if (g_i3DHilightAdj == 0)
    {
        g_i3DHilightAdj = 500;
        g_fScale3DHilightAdj = TRUE;

        if (hk)
        {
            Look_Load3DRatio(hk, g_szHilightAdjust,
                &g_i3DHilightAdj, &g_fScale3DHilightAdj);
        }
    }

    if (g_iWatermarkAdj == 0)
    {
        g_iWatermarkAdj = -50;
        g_fScaleWatermarkAdj = TRUE;

        if (hk)
        {
            Look_Load3DRatio(hk, g_szWatermarkAdjust,
                &g_iWatermarkAdj, &g_fScaleWatermarkAdj);
        }
    }

    if (hk)
        RegCloseKey(hk);
}

void NEAR PASCAL Look_Reset3DRatios(void)
{
    g_i3DShadowAdj = g_i3DHilightAdj = g_iWatermarkAdj = 0;
}
#else
void NEAR PASCAL Look_Reset3DRatios(void)   {}
void NEAR PASCAL Look_Get3DRules(void)      {}
#endif

const static DWORD FAR aAppearanceHelpIds[] = {
        IDC_LOOKPREV,       IDH_DISPLAY_APPEARANCE_GRAPHIC,

        IDC_SCHEMES,        IDH_DISPLAY_APPEARANCE_SCHEME,
        IDC_SAVESCHEME,     IDH_DISPLAY_APPEARANCE_SAVEAS_BUTTON,
        IDC_DELSCHEME,      IDH_DISPLAY_APPEARANCE_DELETE_BUTTON,

        IDC_ELEMENTS,       IDH_DISPLAY_APPEARANCE_ITEM_LIST,

        IDC_SIZELABEL,      IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
        IDC_MAINSIZE,       IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
        IDC_SIZEARROWS,     IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
        IDC_COLORLABEL,     IDH_DISPLAY_APPEARANCE_ITEM_COLOR, 
        IDC_MAINCOLOR,      IDH_DISPLAY_APPEARANCE_ITEM_COLOR, 
        IDC_GRADIENT,       IDH_DISPLAY_APPEARANCE_ITEM_COLOR2,
        IDC_GRADIENTLABEL,  IDH_DISPLAY_APPEARANCE_ITEM_COLOR2, 

        IDC_FONTLABEL,      IDH_DISPLAY_APPEARANCE_FONT_LIST, 
        IDC_FONTNAME,       IDH_DISPLAY_APPEARANCE_FONT_LIST,
        IDC_FONTSIZELABEL,  IDH_DISPLAY_APPEARANCE_FONT_SIZE,
        IDC_FONTSIZE,       IDH_DISPLAY_APPEARANCE_FONT_SIZE,
        IDC_TEXTCOLOR,      IDH_DISPLAY_APPEARANCE_FONT_COLOR, 
        IDC_FNCOLORLABEL,   IDH_DISPLAY_APPEARANCE_FONT_COLOR, 
        IDC_FONTBOLD,       IDH_DISPLAY_APPEARANCE_FONT_BOLD,
        IDC_FONTITAL,       IDH_DISPLAY_APPEARANCE_FONT_ITALIC, 

        0, 0
};


void NEAR PASCAL _PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndChild;

    /*
     * Don't propagate during exit since this is only for good looks, and the Up/Down's
     * get confused if they get a WM_SETTINGSHCANGED while they are getting destroyed
     */
    if (g_fProprtySheetExiting)
        return;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
#ifdef DBG_PRINT
        TCHAR szTmp[256];
        GetClassName(hwndChild, szTmp, 256);

        TraceMsg(TF_GENERAL, "desk (PropagateMessage): SendingMessage( 0x%08lX cls:%s, 0x%08X, 0x%08lX, 0x%08lX )\n", hwndChild, szTmp, uMessage, wParam, lParam ));
#endif
        SendMessage(hwndChild, uMessage, wParam, lParam);
        TraceMsg(TF_GENERAL,"desk (PropagateMessage): back from SendingMessage\n");
    }
}

void PASCAL Look_UpdateGradientButton(HWND hDlg)
{
    BOOL bEnableGradient = FALSE;
    BOOL bGradient = FALSE;

    SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (PVOID)&bGradient, 0);

    if (g_iCurElement >= 0)
        bEnableGradient = ((ELCUR.iGradientColor != COLOR_NONE));

    EnableWindow(GetDlgItem(hDlg, IDC_GRADIENT), 
                 (bEnableGradient && bGradient));
    EnableWindow(GetDlgItem(hDlg, IDC_GRADIENTLABEL), 
                 (bEnableGradient && bGradient));
}

//
// When the "Appearance" tab realizes that the DPI value has changed, it updates all it's
// datastructures to update the font heights based on the new DPI value; 
// It also re-calculates the sizes of various elements; Finally, it repaints to show the
// effect of font height change. It also remembers that a change has occured sothat when 
// "Apply" occurs, it can save the changes to the registry.
//

void Look_UpdateDPIchange(HWND hDlg)
{
    //Check if the DPI value has really changed since the last time we applied a DPI change.
    if(g_iAppliedNewDPI != g_iNewDPI)
    {
        //Yes! The DPI has changed by "General" tab and the system hasn't been rebooted yet!
        
        int i, iTempCurElement, iSizeIndex, iElem;
        LOOK_SIZE TempElCurSize;

        //Cycle through all the UI fonts and change their sizes.
        for(i = 0; i < NUM_FONTS; i++)
        {
            g_fonts[i].lf.lfHeight = MulDiv(g_fonts[i].lf.lfHeight,  g_iNewDPI, g_iAppliedNewDPI);
            //Delete the old font!
            if (g_fonts[i].hfont)
                DeleteObject(g_fonts[i].hfont);
            //Create the new one with the new size!
            g_fonts[i].hfont = CreateFontIndirect(&g_fonts[i].lf);
        }

        //Cycle through all the sizes and update them
        iTempCurElement = g_iCurElement; //Save the current element.
        TempElCurSize = ELCURSIZE;      //Save the current size
        for(iSizeIndex = 0; iSizeIndex < NUM_SIZES; iSizeIndex++)
        {
            //Cycle through all the elements to find the ones whose size depends on font
            for(iElem = 0; iElem < NUM_ELEMENTS; iElem++)
            {
                if((g_elements[iElem].iSize == iSizeIndex) && (g_elements[iElem].fLinkSizeToFont))
                {
                    g_iCurElement = iElem; //Set this as the current element.
                    ELCURSIZE = g_sizes[iSizeIndex]; //Set this as the current size.

                    Look_UpdateSizeBasedOnFont(hDlg, TRUE);
                }
            }
        }
        g_iCurElement = iTempCurElement;  //Restore the current element.
        ELCURSIZE = TempElCurSize;

        //We have applied the DPI change to alter the font sizes. 
        g_iAppliedNewDPI = g_iNewDPI;

        g_fAppliedDPIchanged = TRUE; //Remember that we need to save this in registry!
        
        //The font size has changed; Make sure the new size is selected in the Font size combo box.
        Font_SelectSize(hDlg, Font_HeightToPoint(ELCURFONT.lf.lfHeight));

        Look_Repaint(hDlg, TRUE);
        
        Look_Changed(hDlg, DPI_CHANGE);

    }
}

INT_PTR CALLBACK  AppearanceDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    int i;
    TCHAR szBuf[100];
    LPTSTR pszSchemeName;
    TCHAR szNewScheme[150];
    BOOL bCheck;

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
#ifdef READ_3D_RULES_FROM_REGISTRY
                    Look_Reset3DRatios();
#endif

                    Look_UpdateDPIchange(hDlg);
                    SendDlgItemMessage(hDlg, IDC_LOOKPREV, WM_RECREATEBITMAP, 0, 0L);
                    Look_UpdateGradientButton(hDlg);
                    break;
                case PSN_APPLY:
                    g_fProprtySheetExiting = (BOOL)((LPPSHNOTIFY)(lParam))->lParam;
                    Look_UpdateDPIchange(hDlg);
                    Look_ApplyChanges();
                    if(g_fAppliedDPIchanged)
                    {
                        Look_SaveAppliedDPIinReg(g_iAppliedNewDPI);
                        g_fAppliedDPIchanged = FALSE;
                    }
                    break;

                case PSN_RESET:
                    break;
            }
            break;

        case WM_INITDIALOG:
            g_hDlg = hDlg;
            g_bInit = TRUE;
            Look_InitDialog(hDlg);
            Look_InitSysStuff();
            Font_InitList(hDlg);

            // paint the preview
            Look_Repaint(hDlg, TRUE);
            Look_SelectElement(hDlg, ELEMENT_DESKTOP, LSE_SETCUR);
            g_bInit = FALSE;
            break;

        case WM_DESTROY:
            Look_DestroySysStuff();
            Look_DestroyDialog(hDlg);
            g_hDlg = NULL;
            break;

        case WM_DRAWITEM:
            switch (wParam)
            {
                case IDC_GRADIENT:
                case IDC_MAINCOLOR:
                case IDC_TEXTCOLOR:
                    Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam);
                    return TRUE;
            }
            break;

        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_DISPLAYCHANGE:
            _PropagateMessage(hDlg, message, wParam, lParam);
            break;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            SendDlgItemMessage(hDlg, IDC_LOOKPREV, message, wParam, lParam);
            return TRUE;

        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, TEXT("display.hlp"),
                HELP_WM_HELP, (DWORD_PTR)  aAppearanceHelpIds);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            WinHelp((HWND) wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
                (DWORD_PTR)  aAppearanceHelpIds);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_ELEMENTS:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        i = (int)SendDlgItemMessage(hDlg, IDC_ELEMENTS, CB_GETCURSEL,0,0L);
                        i = LOWORD(SendDlgItemMessage(hDlg, IDC_ELEMENTS, CB_GETITEMDATA, (WPARAM)i, 0L));
                        Look_SelectElement(hDlg, i, 0);
                    }
                    break;

                case IDC_FONTNAME:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        DWORD dwItemData;
                        i = (int)SendDlgItemMessage(hDlg, IDC_FONTNAME, CB_GETCURSEL,0,0L);
                        dwItemData = (DWORD)SendDlgItemMessage(hDlg, IDC_FONTNAME, CB_GETITEMDATA, (WPARAM)i, 0);
                        Font_SelectName(hDlg, i);
                        Font_SelectSize(hDlg, Font_HeightToPoint(ELCURFONT.lf.lfHeight));

                        Font_GetNameFromList(GetDlgItem(hDlg, IDC_FONTNAME),
                                             i,
                                             szBuf,
                                             ARRAYSIZE(szBuf),
                                             NULL,
                                             0);
                        //
                        // Change font to currently selected name and charset.
                        //
                        Look_ChangeFontName(hDlg, szBuf, HIWORD(dwItemData));
                    }
                    break;

                case IDC_FONTSIZE:
                    switch (HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                            i = (int)SendDlgItemMessage(hDlg, IDC_FONTSIZE, CB_GETCURSEL,0,0L);
                            i = LOWORD(SendDlgItemMessage(hDlg, IDC_FONTSIZE, CB_GETITEMDATA, (WPARAM)i, 0L));
                            Look_ChangeFontSize(hDlg, i);
                            break;

                        case CBN_EDITCHANGE:
                            GetWindowText(GetDlgItem(hDlg, IDC_FONTSIZE), szBuf, ARRAYSIZE(szBuf));
                            Look_ChangeFontSize(hDlg, (int)MyStrToLong(szBuf));
                            break;
                    }
                    break;

                case IDC_FONTBOLD:
                case IDC_FONTITAL:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        bCheck = !IsDlgButtonChecked(hDlg, LOWORD(wParam));
                        CheckDlgButton(hDlg, LOWORD(wParam), bCheck);
                        Look_ChangeFontBI(hDlg, LOWORD(wParam), bCheck);
                    }
                    break;

                case IDC_MAINSIZE:
                    if((HIWORD(wParam) == EN_CHANGE) && (g_iCurElement >= 0) &&
                                        (ELCUR.iSize >= 0))
                    {
                        i = (int)LOWORD(SendDlgItemMessage(hDlg, IDC_SIZEARROWS, UDM_GETPOS,0,0L));
                        Look_ChangeSize(hDlg, i, TRUE);
                    } else if(HIWORD(wParam) == EN_KILLFOCUS) {
                        i = (int)SendDlgItemMessage(hDlg, IDC_SIZEARROWS, UDM_GETPOS,0,0L);
                        if (HIWORD(i) != 0) {
                            SetDlgItemInt(hDlg, IDC_MAINSIZE, (UINT)LOWORD(i), FALSE);
                        }
                    }
                    break;

                case IDC_GRADIENT:
                case IDC_MAINCOLOR:
                case IDC_TEXTCOLOR:
                    if (HIWORD(wParam) == BN_CLICKED)
                        Color_PickAColor( hDlg, (int)LOWORD(wParam) );
                    break;

                case IDC_SCHEMES:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        BOOL    fNewSchemeName = FALSE;
                        BOOL    fDirty  = FALSE;
                        
                        i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_GETCURSEL, 0, 0L);
                        SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_GETLBTEXT, (WPARAM)i, (LPARAM)szBuf);

                        if (lstrcmpi(g_szCurScheme, szBuf) == 0)
                            break;

                        // fSilent = FALSE; This implies that this call will ask the end-user for
                        // confirmation if it needed to change any of the font name to support the
                        // charsets. In other words, the following call can fail if the end-user
                        // says "No".
                        if (Scheme_SetScheme(szBuf, FALSE, &fNewSchemeName, &fDirty))
                        {
                            // prevent using value in edit box
                            g_iPrevSize = SIZE_NONE;

                            // force repaint of element information
                            i = g_iCurElement;
                            g_iCurElement = -1;
                            Look_SelectElement(hDlg, i, 0);

                            // repaint preview
                            Look_Repaint(hDlg, TRUE);

                            // remember new current scheme
                            if(fNewSchemeName)
                            {
                                TCHAR   szCopyOfTemplate[40]; //"Copy of %s" internationalized string.
                                
                                LoadString(hInstance, IDS_COPYOF_SCHEME, szCopyOfTemplate, ARRAYSIZE(szCopyOfTemplate));
                                wsprintf(szNewScheme, szCopyOfTemplate, szBuf);
                                pszSchemeName = szNewScheme;
                            }
                            else
                                pszSchemeName = szBuf;
                                
                            lstrcpy(g_szCurScheme, pszSchemeName);
                            lstrcpy(g_szLastScheme, pszSchemeName);

                            Look_Changed(hDlg, SCHEME_CHANGE);

                            if(fNewSchemeName || fDirty)
                            {
                                int    i;
                                //Save it under new name
                                if (Scheme_SaveScheme(pszSchemeName))
                                {
                                    //See if this name already exists.
                                    i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_FINDSTRINGEXACT, 0, (LPARAM)g_szCurScheme);
                                    if (i == CB_ERR) //if it doesn't exist, add it!
                                        i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_ADDSTRING, 0, (LPARAM)g_szCurScheme);
                                    //Select the newly saved scheme.
                                    SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_SETCURSEL, (WPARAM)i, 0L);
                                }
                            }
                        }
                        else
                        {
                            int i;
                            
                            //Since we could not set this scheme, let's revert back to the original
                            //scheme we had.
                            
                            //See if this name already exists.
                            i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_FINDSTRINGEXACT, 0, (LPARAM)g_szCurScheme);
                            if (i == CB_ERR) //if it doesn't exist, add it!
                                i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_ADDSTRING, 0, (LPARAM)g_szCurScheme);
                            //Select the existing scheme.
                            SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_SETCURSEL, (WPARAM)i, 0L);
                        }
                    }
                    break;

                case IDC_SAVESCHEME:
                    if (DialogBox(hInstance, MAKEINTRESOURCE(DLG_SAVESCHEME), hDlg, SaveSchemeDlgProc) == IDOK)
                    {
                        if (Scheme_SaveScheme(g_szCurScheme))
                        {
                            i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_FINDSTRINGEXACT, 0, (LPARAM)g_szCurScheme);
                            if (i == CB_ERR)
                                i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_ADDSTRING, 0, (LPARAM)g_szCurScheme);

                            SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_SETCURSEL, (WPARAM)i, 0L);
                        }
                    }
                    break;

                case IDC_DELSCHEME:
                    i = (int)SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_GETCURSEL, 0, 0L);
                    if (i != CB_ERR)
                    {
                        SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_GETLBTEXT, (WPARAM)i, (LPARAM)szBuf);
                        Scheme_DeleteScheme(szBuf);
                        SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_DELETESTRING, (WPARAM)i, 0L);
                        SendDlgItemMessage(hDlg, IDC_SCHEMES, CB_SETCURSEL, (WPARAM)-1, 0L);
                    }
                    else
                    {
                        DeskShellMessageBox(hInstance, hDlg, MAKEINTRESOURCE(IDS_NOSCHEME2DEL), NULL, MB_OK | MB_ICONEXCLAMATION);
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

//
// exported to allow other dudes to leverage our scheme change code
//
#ifdef UNICODE

#   define REAL_DESK_SET_CURRENT_SCHEME     DeskSetCurrentSchemeW
#   define ALT_DESK_SET_CURRENT_SCHEME      DeskSetCurrentSchemeA

#   define LPCOSTR  LPCSTR
#   define ALT_TO_NATIVE( szSrc, szDst, cchDst )    MultiByteToWideChar( CP_ACP, 0, szSrc, -1, szDst, cchDst)

#else

#   define REAL_DESK_SET_CURRENT_SCHEME     DeskSetCurrentSchemeA
#   define ALT_DESK_SET_CURRENT_SCHEME      DeskSetCurrentSchemeW

#   define LPCOSTR  LPCWSTR
#   define ALT_TO_NATIVE( szSrc, szDst, cchDst )    WideCharToMultiByte( CP_ACP, 0, szSrc, -1, szDst, cchDst, NULL, NULL)

#endif

BOOL WINAPI REAL_DESK_SET_CURRENT_SCHEME(LPCTSTR szName)
{
    BOOL fSuccess;

    //
    // init our global stuff if nobody else did
    //
    if (!g_hDlg)
    {
        g_bInit = TRUE;
        Look_InitSysStuff();
        g_bInit = FALSE;
    }

    //
    // fake scheme selection
    //
    fSuccess = Look_ChooseScheme(szName);

    //
    // apply changes if it worked
    //
    if (fSuccess)
        Look_ApplyChanges();

    //
    // clean up our global stuff if nobody else did
    //
    if (!g_hDlg)
        Look_DestroySysStuff();

    return fSuccess;
}

BOOL ALT_DESK_SET_CURRENT_SCHEME( LPCOSTR oszName ) {
    int cch;
    BOOL fRet;
    LPTSTR pszName;

    cch = ALT_TO_NATIVE( oszName, NULL, 0);
    if (cch == 0)
        return FALSE;

    pszName = LocalAlloc( LMEM_FIXED, cch * SIZEOF(TCHAR) );
    if (pszName == NULL)
        return FALSE;

    ALT_TO_NATIVE( oszName, pszName, cch);

    fRet = REAL_DESK_SET_CURRENT_SCHEME( pszName );

    LocalFree(pszName);

    return fRet;
}

BYTE WINAPI MyStrToByte(LPCTSTR sz)
{
    BYTE l=0;

    while (*sz >= TEXT('0') && *sz <= TEXT('9'))
        l = l*10 + (*sz++ - TEXT('0'));

    return l;
}

COLORREF ConvertColor (LPTSTR lpColor)
{
    BYTE RGBTemp[3];
    LPTSTR lpTemp = lpColor;
    UINT i;

    if (!lpColor || !*lpColor) {
        return RGB(0,0,0);
    }


    for (i =0; i < 3; i++) {

        //
        // Remove leading spaces
        //

        while (*lpTemp == TEXT(' ')) {
            lpTemp++;
        }


        //
        // Set lpColor to the beginning of the number
        //

        lpColor = lpTemp;


        //
        // Find the end of the number and null terminate
        //

        while ((*lpTemp) && (*lpTemp != TEXT(' '))) {
            lpTemp++;
        }

        if (*lpTemp != TEXT('\0')) {
            *lpTemp = TEXT('\0');
        }

        lpTemp++;

        RGBTemp[i] = MyStrToByte(lpColor);
    }


    return (RGB(RGBTemp[0], RGBTemp[1], RGBTemp[2]));
}

VOID RefreshColors (void)
{
    UINT i;
    HKEY hk;
    TCHAR szColor[15];
    DWORD dwSize, dwType;
    int iColors[COLOR_MAX];
    COLORREF rgbColors[COLOR_MAX];


    //
    // Open the Colors key in the registry
    //

    if(RegOpenKeyEx(HKEY_CURRENT_USER, szRegStr_Colors, 0,
                    KEY_READ, &hk) != ERROR_SUCCESS) {
       return;
    }


    //
    // Query for the color information
    //

    for (i = 0; i < COLOR_MAX; i++) {

        dwSize = 15 * sizeof(TCHAR);

        if (RegQueryValueEx (hk, s_pszColorNames[i], NULL, &dwType,
                        (LPBYTE) szColor, &dwSize) == ERROR_SUCCESS) {

            g_rgb[i] = ConvertColor (szColor);

        } else {
            g_rgb[i] = GetSysColor (i);
        }
    }

    RegCloseKey(hk);


    //
    // This call causes user to send a WM_SYSCOLORCHANGE
    //

    for (i=0; i < COLOR_MAX; i++)
    {
        iColors[i] = i;
        rgbColors[i] = g_rgb[i] & 0x00FFFFFF;
    }

    SetSysColors(COLOR_MAX, iColors, rgbColors);


    //
    // Refresh the background bitmap
    //

    SystemParametersInfo (SPI_SETDESKWALLPAPER, 0, 0, FALSE);

}
