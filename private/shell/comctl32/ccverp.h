//
// Version numbering for comctl32.dll
//

#ifndef FEATURE_IE40

#undef VER_PRODUCTBETA_STR
#undef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION_W
#undef VER_PRODUCTVERSION_DW

//
// NT SUR, IE 3.0
//

#define COMCTL32_BUILD_NUMBER	   1146

#ifdef WINNT
#define VER_PRODUCTVERSION          4,70,COMCTL32_BUILD_NUMBER,1
#else
#define VER_PRODUCTVERSION          4,70,0,COMCTL32_BUILD_NUMBER
#endif

#define VER_PRODUCTBETA_STR         ""
#define VER_PRODUCTVERSION_STR      "4.70"
#define VER_PRODUCTVERSION_W        (0x0446)
#define VER_PRODUCTVERSION_DW       (0x04460000 | COMCTL32_BUILD_NUMBER)

#endif  // FEATURE_IE40


