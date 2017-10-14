#define DLG_SCREENSAVER  150
#define DLG_BACKGROUND   151
#define DLG_APPEARANCE   152
#define DLG_MONITOR      153
#define DLG_ADAPTER      154
#define DLG_CUSTOMFONT   155
#define DLG_COLORPICK    156
#define DLG_PATTERN      157

#define DLG_FLICKER	160
#define DLG_KEEPNEW	161
#define DLG_SAVESCHEME	162

#define IDS_ICON    40
#define IDS_NAME    41
#define IDS_INFO    42
#define IDS_DESK_NOMEM  43

#define IDB_VIDMETR	120
#define IDB_COLOR1	121
#define IDB_COLOR4	122
#define IDB_COLOR8	123
#define IDB_ENERGYSTAR	124

#define IDC_NO_HELP_1	200	// Used in place of IDC_STATIC when context Help
#define IDC_NO_HELP_2	201	// should be disabled for a control
#define IDC_NO_HELP_3	202	

// screen saver controls
#define IDC_CHOICES		1000
#define IDC_METHOD              1001
#define IDC_BIGICON             1002
#define IDC_SETTING             1003
#define IDC_TEST                1004
#define IDC_ENERGYSTAR_BMP	1005
#define IDC_SCREENSAVEDELAY     1006
#define IDC_SCREENSAVEARROW     1007
#define IDC_LOWPOWERDELAY       1008
#define IDC_LOWPOWERARROW       1009
#define IDC_LOWPOWERSWITCH      1010
#define IDC_POWEROFFDELAY       1011
#define IDC_POWEROFFARROW       1012
#define IDC_POWEROFFSWITCH      1013
//                              1014        unused
#define IDC_DEMO                1015
#define IDC_SSDELAYLABEL        1016
#define IDC_ENERGY_TEXT         1017
#define IDC_ENERGY_TEXT2        1018
#define IDC_ENERGY_TEXT3        1019
#define IDC_USEPASSWORD	        1020
#define IDC_SETPASSWORD         1021
#define IDC_SSDELAYSCALE        1022

// background controls
#define IDC_PATLIST	1100
#define IDC_WALLLIST	1101
#define IDC_EDITPAT	1102
#define IDC_BROWSEWALL	1103
#define IDC_CENTER	1104
#define IDC_TILE	1105
#define IDC_PATTERN	1106
#define IDC_WALLPAPER	1107
#define IDC_BACKPREV	1108
#define IDC_TXT_DISPLAY 1109

// background dialog strings
#define IDS_NONE	1100
#define IDS_UNLISTEDPAT	1101
#define IDS_BITMAPOPENERR	1102
#define IDS_DIB_NOOPEN		1103
#define IDS_DIB_INVALID		1104
#define IDS_DIB_NOMEM		1105
#define IDS_BADWALLPAPER	1106
#define IDS_BROWSETITLE         1107
#define IDS_BROWSEFILTER        1108

// appearance controls
#define IDC_SCHEMES	1400
#define IDC_SAVESCHEME	1401
#define IDC_DELSCHEME	1402
#define IDC_ELEMENTS	1403
#define IDC_MAINSIZE	1404
#define IDC_FONTNAME	1407
#define IDC_FONTSIZE	1408
#define IDC_FONTBOLD	1409
#define IDC_FONTITAL	1410
#define IDC_SIZEARROWS	1411
#define IDC_MAINCOLOR	1412
#define IDC_TEXTCOLOR	1413

#define IDC_SIZELABEL		1450
#define IDC_COLORLABEL		1451
#define IDC_FONTLABEL		1452
#define IDC_FNCOLORLABEL	1453
#define IDC_FONTSIZELABEL	1454

#define IDC_LOOKPREV	1470

// these need to be all clumped together because they are treated as a group
#define IDC_STARTMAINCOLOR	1500
#define IDC_CUSTOMMAINCOLOR	1549
#define IDC_ENDMAINCOLOR	IDC_CUSTOMMAINCOLOR
#define IDC_STARTTEXTCOLOR	1550
#define IDC_CUSTOMTEXTCOLOR	1599
#define IDC_ENDTEXTCOLOR	IDC_CUSTOMTEXTCOLOR

// appearance elements
#define ELNAME_DESKTOP		1401
#define ELNAME_INACTIVECAPTION	1402
#define ELNAME_INACTIVEBORDER	1403
#define ELNAME_ACTIVECAPTION	1404
#define ELNAME_ACTIVEBORDER	1405
#define ELNAME_MENU		1406
#define ELNAME_MENUSELECTED	1407
#define ELNAME_WINDOW		1408
#define ELNAME_SCROLLBAR	1409
#define ELNAME_BUTTON		1410
#define ELNAME_SMALLCAPTION	1411
#define ELNAME_ICONTITLE	1412
#define ELNAME_CAPTIONBUTTON	1413
#define ELNAME_DISABLEDMENU	1414
#define ELNAME_MSGBOX		1415
#define ELNAME_SCROLLBUTTON	1416
#define ELNAME_APPSPACE		1417
#define ELNAME_SMCAPSYSBUT	1418
#define ELNAME_SMALLWINDOW	1419
#define ELNAME_DXICON           1420
#define ELNAME_DYICON           1421
#define ELNAME_INFO             1422
#define ELNAME_ICON             1423
#define ELNAME_SMICON           1424

// appearance strings for sample
#define IDS_ACTIVE	1450
#define IDS_INACTIVE	1451
#define IDS_MINIMIZED	1452
#define IDS_ICONTITLE	1453
#define IDS_NORMAL	1454
#define IDS_DISABLED	1455
#define IDS_SELECTED	1456
#define IDS_MSGBOX	1457
#define IDS_BUTTONTEXT	1458
#define IDS_SMCAPTION	1459
#define IDS_WINDOWTEXT	1460
#define IDS_MSGBOXTEXT	1461

#define IDS_BLANKNAME		1480
#define IDS_NOSCHEME2DEL	1481

// appearance preview menu
#define IDR_MENU	1
#define IDM_NORMAL	10
#define IDM_DISABLED	11
#define IDM_SELECTED	12

// monitor settings controls
#define IDC_COLORLIST	1200
#define IDC_RESLIST	1201
#define IDC_RESSIZE	1204
#define IDC_SCREENSAMPLE 1205
#define IDC_COLORSAMPLE 1206
#define IDC_RESXY	1207
#define IDC_ZOOMIN	1209
#define IDC_ZOOMOUT	1210
#define IDC_CUSTOMFONT  1211
#define IDC_FONTLIST    1212
#define IDC_CHANGEDRV   1213
#define IDC_FULLDRAG    1214
#define IDC_SMOOTHFONTS 1215
#define IDC_FONTGROUPBOX 1216

#define BMP_MONITOR	1250

// adapter settings controls
#define IDC_ADAPTERNAME		1301
#define IDC_ADAPTERCHANGE	1302
#define IDC_MONITORNAME		1303
#define IDC_MONITORCHANGE	1304
#define IDC_DRVMAN		1305
#define IDC_DRVVER		1306
#define IDC_DRVFILES		1307
#define IDC_DPMS		1308
#define IDC_DRVMAN_TXT		1309
#define IDC_DRVVER_TXT		1310
#define IDC_DRVFILES_TXT	1311

#define IDS_CLOSE		1300

// strings for monitor settings
// the ordering matches a color resolution array
// NOTE: numbering = base + (res/4)
#define IDS_COLOR	1200
#define IDS_COLOR_1	IDS_COLOR + 0
#define IDS_COLOR_4	IDS_COLOR + 1
#define IDS_COLOR_8	IDS_COLOR + 2
#define IDS_COLOR_16	IDS_COLOR + 4
#define IDS_COLOR_24	IDS_COLOR + 6
#define IDS_COLOR_32	IDS_COLOR + 8

#define IDS_CANCELNEW	1220
#define IDS_FAILRESET	1221
#define IDS_X_BY_Y	1224
#define IDS_UNKNOWN	1225
#define IDS_NOINFO	1226
#define IDS_CUSTFONTPER 1229
#define IDS_CUSTFONTWARN 1230
#define IDS_PAT_REMOVE  1231
#define IDS_PAT_CHANGE  1232
#define IDS_PAT_CREATE  1233
#define IDS_REMOVEPATCAPTION	1234
#define IDS_CHANGEPATCAPTION	1235

#define IDS_WARNFLICK1  1250
#define IDS_WARNFLICK2  1251
// order is based on values in ddk\inc16\valmode.inc
#define	IDS_NOVALID0	1254
#define	IDS_NOVALID1	1255
#define	IDS_NOVALID2	1256
#define	IDS_NOVALID3	1257
#define	IDS_NOVALID4	1258
#define	IDS_NOMONITOR	1260
#define IDS_FALLBACKMODE 1261
#define IDS_FALLBACKDRV 1262

#define IDS_NOADAPTER           1270
#define IDS_CONFLICT            1271
#define IDS_BADSETTINGS         1272
#define IDS_DISPDISABLED        1273

//controls & strings for custom font dialog
#define IDC_CUSTOMSAMPLE	1400
#define IDC_CUSTOMRULER		1501
#define IDC_CUSTOMCOMBO		1502

#define IDS_10PTSAMPLE		1500
#define IDS_RULERDIRECTION      1501
#define IDS_10PTSAMPLEFACENAME  1510

#define IDS_NEWSETTINGS         1600
#define IDS_NEWSETTINGS_RESTART 1601

// color picker mini-dialog
#define IDC_16COLORS	1615
#define IDC_COLORCUST	1616
#define IDC_COLOROTHER	1617
#define IDC_COLORETCH	1618

// patern edit dialog

#define IDD_PATTERN		1700
#define IDD_PATTERNCOMBO	1701
#define IDD_ADDPATTERN		1702
#define IDD_CHANGEPATTERN       1703
#define IDD_DELPATTERN		1704
#define IDD_PATSAMPLE           1705
#define IDD_PATSAMPLE_TXT       1706
#define IDD_PATTERN_TXT		1707
