#include "generic.h"
#include "stdio.h"
#pragma code_seg(_ICMSEG)

char ICMSEG BeginString[]       = "<";
char ICMSEG EndString[]         = ">";
char ICMSEG BeginArray[]        = "[";
char ICMSEG EndArray[]          = "]";
char ICMSEG BeginFunction[]     = "{";
char ICMSEG EndFunction[]       = "}bind ";
char ICMSEG BeginDict[]         = "<<" ;
char ICMSEG EndDict[]           = ">>" ;
char ICMSEG BlackPoint[]        = "[0 0 0]" ;
char ICMSEG DictType[]          = "/ColorRenderingType 1 ";
char ICMSEG IntentType[]        = "/RenderingIntent ";
char ICMSEG IntentPer[]         = "/Perceptual";
char ICMSEG IntentSat[]         = "/Saturation";
char ICMSEG IntentACol[]        = "/AbsoluteColorimetric";
char ICMSEG IntentRCol[]        = "/RelativeColorimetric";

char ICMSEG WhitePointTag[]     = "/WhitePoint " ;
char ICMSEG BlackPointTag[]     = "/BlackPoint " ;
char ICMSEG RangePQRTag[]       = "/RangePQR " ;
char ICMSEG TransformPQRTag[]   = "/TransformPQR " ;
char ICMSEG MatrixPQRTag[]      = "/MatrixPQR " ;

char ICMSEG RangeABCTag[]       = "/RangeABC " ;
char ICMSEG MatrixATag[]        = "/MatrixA ";
char ICMSEG MatrixABCTag[]      = "/MatrixABC ";
char ICMSEG EncodeABCTag[]      = "/EncodeABC " ;
char ICMSEG RangeLMNTag[]       = "/RangeLMN " ;
char ICMSEG MatrixLMNTag[]      = "/MatrixLMN " ;
char ICMSEG EncodeLMNTag[]      = "/EncodeLMN " ;
char ICMSEG RenderTableTag[]    = "/RenderTable " ;
char ICMSEG CIEBasedATag[]      = "/CIEBasedA " ;
char ICMSEG CIEBasedABCTag[]    = "/CIEBasedABC " ;
char ICMSEG CIEBasedDEFGTag[]   = "/CIEBasedDEFG " ;
char ICMSEG CIEBasedDEFTag[]    = "/CIEBasedDEF " ;
char ICMSEG DecodeATag[]        = "/DecodeA " ;
char ICMSEG DecodeABCTag[]      = "/DecodeABC " ;
char ICMSEG DecodeLMNTag[]      = "/DecodeLMN " ;
char ICMSEG DeviceRGBTag[]      = "/DeviceRGB " ;
char ICMSEG DeviceCMYKTag[]     = "/DeviceCMYK " ;
char ICMSEG DeviceGrayTag[]     = "/DeviceGray " ;
char ICMSEG TableTag[]          = "/Table " ;
char ICMSEG DecodeDEFGTag[]     = "/DecodeDEFG " ;
char ICMSEG DecodeDEFTag[]      = "/DecodeDEF " ;

char ICMSEG NullOp[]            = "";
char ICMSEG DupOp[]             = "dup ";
char ICMSEG UserDictOp[]        = "userdict ";
char ICMSEG GlobalDictOp[]      = "globaldict ";
char ICMSEG CurrentGlobalOp[]   = "currentglobal ";
char ICMSEG SetGlobalOp[]       = "setglobal ";
char ICMSEG DefOp[]             = "def ";
char ICMSEG BeginOp[]           = "begin ";
char ICMSEG EndOp[]             = "end ";
char ICMSEG TrueOp[]            = "true ";
char ICMSEG FalseOp[]           = "false ";
char ICMSEG MulOp[]             = "mul ";
char ICMSEG DivOp[]             = "div ";

char ICMSEG NewLine[]           = "\n" ;
char ICMSEG Slash[]             = "/" ;
char ICMSEG Space[]             = " " ;
char ICMSEG CRDBegin[]          = "%** CRD Begin ";
char ICMSEG CRDEnd[]            = "%** CRD End ";
char ICMSEG CieBasedDEFGBegin[] = "%** CieBasedDEFG CSA Begin ";
char ICMSEG CieBasedDEFBegin[]  = "%** CieBasedDEF CSA Begin ";
char ICMSEG CieBasedABCBegin[]  = "%** CieBasedABC CSA Begin ";
char ICMSEG CieBasedABegin[]    = "%** CieBasedA CSA Begin ";
char ICMSEG CieBasedDEFGEnd[]   = "%** CieBasedDEFG CSA End ";
char ICMSEG CieBasedDEFEnd[]    = "%** CieBasedDEF CSA End ";
char ICMSEG CieBasedABCEnd[]    = "%** CieBasedABC CSA End ";
char ICMSEG CieBasedAEnd[]      = "%** CieBasedA CSA End ";
char ICMSEG RangeABC[]          = "[ 0 1 0 1 0 1 ] ";
char ICMSEG RangeLMN[]          = "[ 0 2 0 2 0 2 ] ";
char ICMSEG Identity[]          = "[1 0 0 0 1 0 0 0 1]";
char ICMSEG RangeABC_Lab[]      = "[0 100 -128 127 -128 127]";

/********** This PostScript code clips incoming value between 0.0 and 1.0
   Use:   x <clip>   --   <clipped x>                                   */
char ICMSEG Clip01[]            = "dup 1.0 ge{pop 1.0}{dup 0.0 lt{pop 0.0}if}ifelse " ;
char ICMSEG DecodeA3[]          = "256 div exp ";
char ICMSEG DecodeA3Rev[]       = "256 div 1.0 exch div exp ";
char ICMSEG DecodeABCArray[]    = "DecodeABC_";
char ICMSEG InputArray[]        = "Inp_";
char ICMSEG OutputArray[]       = "Out_";
char ICMSEG PreViewInArray[]    = "IPV_";
char ICMSEG PreViewOutArray[]   = "OPV_";

char ICMSEG AdobeCSA[] = "[ /CIEBasedABC << /DecodeLMN [\n\
{dup 0.03928 le {12.92321 div}{0.055 add 1.055 div 2.4 exp}ifelse} bind dup dup ]\n\
/MatrixLMN [0.412457 0.212673 0.019334 0.357576 0.715152 0.119192 0.180437 0.072175 0.950301]\n\
/WhitePoint [ 0.9505 1 1.0890 ] >> ]";

char ICMSEG AdobeCRD[] = " /RangePQR [ -0.5 2 -0.5 2 -0.5 2 ]\n\
/MatrixPQR [0.8951 -0.7502  0.0389 0.2664  1.7135 -0.0685 -0.1614  0.0367  1.0296]\n\
/TransformPQR [{exch pop exch 3 get mul exch pop exch 3 get div} bind\n\
{exch pop exch 4 get mul exch pop exch 4 get div} bind\n\
{exch pop exch 5 get mul exch pop exch 5 get div} bind]\n\
/MatrixLMN [3.240449 -0.969265  0.055643 -1.537136  1.876011 -0.204026 -0.498531  0.041556  1.057229]\n\
/EncodeABC [{dup 0.00304 le {12.92321 mul}{1 2.4 div exp 1.055 mul 0.055 sub}ifelse} bind dup dup]\n\
/WhitePoint[0.9505 1 1.0890] >>";

// This PostScript segment takes value in range from 0.0 to 1.0 and
//  interpolates the result using array supplied.
//   x [array]  -- <interpolated value>

char ICMSEG IndexArray16b[]     = \
" dup length 1 sub 3 -1 roll mul dup dup floor cvi \
exch ceiling cvi 3 index exch get 32768 add 4 -1 roll 3 -1 roll get 32768 add \
dup 3 1 roll sub 3 -1 roll dup floor cvi sub mul add ";

char ICMSEG IndexArray[]        = \
" dup length 1 sub 3 -1 roll mul dup dup floor cvi \
exch ceiling cvi 3 index exch get 4 -1 roll 3 -1 roll get \
dup 3 1 roll sub 3 -1 roll dup floor cvi sub mul add ";

char ICMSEG TestingDEFG[]       = \
"/SupportDEFG? {/CIEBasedDEFG /ColorSpaceFamily resourcestatus { pop pop languagelevel 3 ge}{false} ifelse} def";

char ICMSEG SupportDEFG_S[]     = "SupportDEFG? { ";
char ICMSEG NotSupportDEFG_S[]  = "SupportDEFG? not { ";
char ICMSEG SupportDEFG_E[]     = "}if ";

char ICMSEG StartClip[]         = "dup 1.0 le{dup 0.0 ge{" ;
char ICMSEG EndClip[]           = "}if}if " ;

char ICMSEG Scale8[]            = "255 div " ;
char ICMSEG Scale16[]           = "65535 div " ;
char ICMSEG Scale16XYZ[]        = "32768 div " ;
char ICMSEG TFunction8[]        = "exch 255 mul round cvi get 255 div " ;
char ICMSEG TFunction8XYZ[]     = "exch 255 mul round cvi get 128 div " ;
char ICMSEG MatrixABCLab[]      = "[1 1 1 1 0 0 0 0 -1]" ;
char ICMSEG DecodeABCLab1[]     = "[{16 add 116 div} bind {500 div} bind {200 div} bind]";
char ICMSEG DecodeALab[]        = " 50 mul 16 add 116 div ";
char ICMSEG DecodeLMNLab[]      = \
"dup 0.206897 ge{dup dup mul mul}{0.137931 sub 0.128419 mul} ifelse ";

char ICMSEG RangeLMNLab[]       = "[0 1 0 1 0 1]" ;
char ICMSEG EncodeLMNLab[]      = "\
dup 0.008856 le{7.787 mul 0.13793 add}{0.3333 exp}ifelse " ;

char ICMSEG MatrixABCLabCRD[]   = "[0 500 0 116 -500 200 0 0 -200]" ;
char ICMSEG MatrixABCXYZCRD[]   = "[0 1 0 1 0 0 0 0 1]" ;
char ICMSEG EncodeABCLab1[]     = "16 sub 100 div " ;
char ICMSEG EncodeABCLab2[]     = "128 add 255 div " ;

char ICMSEG   RangePQR[]          = "[ -0.07 2.2 -0.02 1.4 -0.2 4.8 ]";
char ICMSEG   MatrixPQR[]         = "[0.8951 -0.7502 0.0389 0.2664 1.7135 -0.0685 -0.1614 0.0367 1.0296]";

char *TransformPQR[3]             = {
"exch pop exch 3 get mul exch pop exch 3 get div ",
"exch pop exch 4 get mul exch pop exch 4 get div ",
"exch pop exch 5 get mul exch pop exch 5 get div " };

#define SRGBCRC 0x678175D2L

#pragma optimize("",off)

/***************************************************************************
*                               CreateLutCRD
*  function:
*    this is the function which creates the Color Rendering Dictionary (CRD)
*    from the data supplied in the ColorProfile's LUT8 or LUT16 tag.
*  prototype:
*       SINT EXTERN CreateLutCRD(
*                          CHANDLE      cp,
*                          SINT         Index,
*                          MEMPTR       lpMem,
*                          BOOL         AllowBinary)
*  parameters:
*       cp          --  Color Profile handle
*       Index       --  Index of the tag
*       lpMem       --  Pointer to the memory block
*       AllowBinary --  1: binary CRD allowed,  0: only ascii CRD allowed.
*
*  returns:
*       SINT        --  !=0 if the function was successful,
*                         0 otherwise.
*                       Returns number of bytes required/transferred
***************************************************************************/

SINT EXTERN 
CreateLutCRD (CHANDLE cp, SINT Index, MEMPTR lpMem, DWORD InputIntent, BOOL AllowBinary)
{
    SINT nInputCh, nOutputCh, nGrids;
    SINT nInputTable, nOutputTable, nNumbers;
    CSIG Tag, PCS;
    CSIG IntentSig;

    SINT Ret;
    SINT i, j;
    MEMPTR lpTable;

    SFLOAT IlluminantWP[3];
    SFLOAT MediaWP[3];
    MEMPTR Buff = NULL;
    SINT MemSize = 0;
    MEMPTR lpOldMem = lpMem;
    char PublicArrayName[TempBfSize];
    HGLOBAL hMem;
    MEMPTR lpLineStart;
 // Check if we can generate the CRD
    if (!GetCPTagSig (cp, Index, (LPCSIG) & IntentSig) ||
        !GetCPElementType (cp, Index, (LPCSIG) & Tag) ||
        ((Tag != icSigLut8Type) && (Tag != icSigLut16Type)) ||
        !GetCPConnSpace (cp, (LPCSIG) & PCS) ||
        !GetCPElementSize (cp, Index, (LPSINT) & MemSize) ||
        !MemAlloc (MemSize, (HGLOBAL FAR *)&hMem, (LPMEMPTR) & Buff) ||
        !GetCPElement (cp, Index, Buff, MemSize))
    {
        if (NULL != Buff)
        {
            MemFree (hMem);
        }
        return (0);
    }
    GetCLUTinfo(Tag, Buff, &nInputCh, &nOutputCh, 
        &nGrids, &nInputTable, &nOutputTable, &i);
 // Level 2 printers support only tri-component CIEBasedABC input,
 // but can have either 3 or 4 output channels.
    if (((nOutputCh != 3) &&
         (nOutputCh != 4)) ||
        (nInputCh != 3))
    {
        SetCPLastError (CP_POSTSCRIPT_ERR);
        MemFree (hMem);
        return (0);
    }
    Ret = nInputCh * nInputTable * 6 +
        nOutputCh * nOutputTable * 6 +  // Number of INT bytes
        nOutputCh * nGrids * nGrids * nGrids * 2 +  // LUT HEX bytes
        nInputCh * (lstrlen (IndexArray) +
                    lstrlen (StartClip) +
                    lstrlen (EndClip)) +
        nOutputCh * (lstrlen (IndexArray) +
                     lstrlen (StartClip) +
                     lstrlen (EndClip)) +
        2048;                           // + other PS stuff

    if (lpMem == NULL)                  // This is a size request
    {
        MemFree (hMem);
        return (Ret);
    }
//  Get all necessary params from the header
//  GetCPRenderIntent (cp, (LPCSIG) & Intent);  // Get Intent
    GetCPWhitePoint (cp, (LPSFLOAT) & IlluminantWP);          // .. Illuminant

 // Support absolute whitePoint
    if (InputIntent == icAbsoluteColorimetric)
    {
        if (!GetCPMediaWhitePoint (cp, (LPSFLOAT) & MediaWP)) // .. Media WhitePoint
        {
            MediaWP[0] = IlluminantWP[0];
            MediaWP[1] = IlluminantWP[1];
            MediaWP[2] = IlluminantWP[2];
        }
    }

//******** Define golbal array used in EncodeABC and RenderTaber
    GetPublicArrayName (cp, IntentSig, PublicArrayName);
    lpMem += WriteNewLineObject (lpMem, CRDBegin);

    lpMem += EnableGlobalDict(lpMem);
    lpMem += BeginGlobalDict(lpMem);

    lpMem += CreateInputArray (lpMem, nInputCh, nInputTable,
             (MEMPTR) PublicArrayName, Tag, Buff, AllowBinary, NULL);

    i = nInputTable * nInputCh +
        nGrids * nGrids * nGrids * nOutputCh;
    lpMem += CreateOutputArray (lpMem, nOutputCh, nOutputTable, i,
             (MEMPTR) PublicArrayName, Tag, Buff, AllowBinary, NULL);

    lpMem += EndGlobalDict(lpMem);

//************* Start writing  CRD  ****************************
    lpMem += WriteNewLineObject (lpMem, BeginDict);    // Begin dictionary
    lpMem += WriteObject (lpMem, DictType); // Dictionary type
    
    lpMem += WriteNewLineObject (lpMem, IntentType); // RenderingIntent
    switch (InputIntent)
    {
        case icPerceptual:
            lpMem += WriteObject (lpMem, IntentPer);
            break;

        case icSaturation:
            lpMem += WriteObject (lpMem, IntentSat);
            break;

        case icRelativeColorimetric:
            lpMem += WriteObject (lpMem, IntentRCol);
            break;

        case icAbsoluteColorimetric:
            lpMem += WriteObject (lpMem, IntentACol);
            break;
    }
    
 //********** Send Black/White Point.
    lpMem += SendCRDBWPoint(lpMem, IlluminantWP);

 //********** Send PQR - used for Absolute Colorimetric *****
    lpMem += SendCRDPQR(lpMem, InputIntent, IlluminantWP);

 //********** Send LMN - For Absolute Colorimetric use WhitePoint's XYZs
    lpMem += SendCRDLMN(lpMem, InputIntent, IlluminantWP, MediaWP, PCS);

 // ******** Create MatrixABC and  EncodeABC  stuff
    lpMem += SendCRDABC(lpMem, PublicArrayName, 
        PCS, nInputCh, Buff, NULL, Tag, AllowBinary);

 //********** /RenderTable
    lpMem += WriteNewLineObject (lpMem, RenderTableTag);
    lpMem += WriteObject (lpMem, BeginArray);

    lpMem += WriteInt (lpMem, nGrids);  // Send down Na
    lpMem += WriteInt (lpMem, nGrids);  // Send down Nb
    lpMem += WriteInt (lpMem, nGrids);  // Send down Nc

    lpLineStart = lpMem;
    lpMem += WriteNewLineObject (lpMem, BeginArray);
    nNumbers = nGrids * nGrids * nOutputCh;
    for (i = 0; i < nGrids; i++)        // Na strings should be sent
    {
        lpMem += WriteObject (lpMem, NewLine);
        lpLineStart = lpMem;
        if (Tag == icSigLut8Type)
        {
            lpTable = (MEMPTR) (((lpcpLut8Type) Buff)->lut.data) +
                nInputTable * nInputCh +
                nNumbers * i;
        } else
        {
            lpTable = (MEMPTR) (((lpcpLut16Type) Buff)->lut.data) +
                2 * nInputTable * nInputCh +
                2 * nNumbers * i;
        }
        if (!AllowBinary)               // Output ASCII CRD
        {
            lpMem += WriteObject (lpMem, BeginString);
            if (Tag == icSigLut8Type)
                lpMem += WriteHexBuffer (lpMem, lpTable, lpLineStart, nNumbers);
            else
            {
                for (j = 0; j < nNumbers; j++)
                {
                    lpMem += WriteHex (lpMem, ui16toSINT (lpTable) / 256);
                    lpTable += sizeof (icUInt16Number);
                    if (((SINT) (lpMem - lpLineStart)) > MAX_LINELENG)
                    {
                        lpLineStart = lpMem;
                        lpMem += WriteObject (lpMem, NewLine);
                    }
                }
            }
            lpMem += WriteObject (lpMem, EndString);
        } else
        {                               // Output BINARY CRD
            lpMem += WriteStringToken (lpMem, 143, nNumbers);
            if (Tag == icSigLut8Type)
                lpMem += WriteByteString (lpMem, lpTable, nNumbers);
            else
                lpMem += WriteInt2ByteString (lpMem, lpTable, nNumbers);
        }
    }

    lpMem += WriteObject (lpMem, EndArray); // End array
    lpMem += WriteInt (lpMem, nOutputCh);   // Send down m

 //********** Send Output Table.
    lpMem += SendCRDOutputTable(lpMem, PublicArrayName, 
        nOutputCh, Tag, FALSE, AllowBinary);

    lpMem += WriteObject (lpMem, EndArray); // End array
    lpMem += WriteObject (lpMem, EndDict);  // End dictionary definition

    lpMem += WriteNewLineObject (lpMem, CRDEnd);

// Testing Convert binary to ascii
//    i = ConvertBinaryData2Ascii(lpOldMem, (SINT)(lpMem - lpOldMem), Ret);
//    lpMem = lpOldMem + i;
// Testing Convert binary to ascii

    MemFree (hMem);
    return ((SINT) ((unsigned long) (lpMem - lpOldMem)));
}
/***************************************************************************
*                               GetRevCurve
*  function:
*  prototype:
*       BOOL  GetRevCurve(
*                          MEMPTR       Buff,
*                          MEMPTR       lpRevCurve)
*  parameters:
*       Buff        --
*       lpRevCurve  --
*  returns:
*       BOOL        --  TRUE:  successful,
*                       FALSE: otherwise.
***************************************************************************/

BOOL
GetRevCurve (MEMPTR lpBuff, MEMPTR lpCurve, MEMPTR lpRevCurve)
{
    SINT i, j, nCount;
    MEMPTR lpTable;
    PUSHORT lpInput, lpOutput;
    SFLOAT fTemp;
    SINT iBegin, iEnd, iTemp;
    nCount = ui32toSINT (((lpcpCurveType) lpBuff)->curve.count);
    lpTable = (MEMPTR) (((lpcpCurveType) lpBuff)->curve.data);
    lpOutput = (PUSHORT) lpRevCurve;
    lpInput = (PUSHORT) lpCurve;

    for (i = 0; i < nCount; i++)
    {
        lpInput[i] = (USHORT) (ui16toSINT (lpTable));
        lpTable += sizeof (icUInt16Number);
    }

    j = nCount * REVCURVE_RATIO;
    for (i = 0; i < j; i++)
    {
        fTemp = (SFLOAT) i *65535 / (j - 1);
        lpOutput[i] = (fTemp < 65535) ? (USHORT) fTemp : (USHORT) 65535;
    }

    for (i = 0; i < j; i++)
    {
        iBegin = 0;
        iEnd = nCount - 1;
        for (;;)
        {
            if ((iEnd - iBegin) <= 1)
                break;
            iTemp = (iEnd + iBegin) / 2;
            if (lpOutput[i] < lpInput[iTemp])
                iEnd = iTemp;
            else
                iBegin = iTemp;
        }
        if (lpOutput[i] <= lpInput[iBegin])
            fTemp = (SFLOAT) iBegin;
        else if (lpOutput[i] >= lpInput[iEnd])
            fTemp = (SFLOAT) iEnd;
        else
        {
            fTemp = ((SFLOAT) (lpInput[iEnd] - lpOutput[i])) /
                (lpOutput[i] - lpInput[iBegin]);
            fTemp = (iBegin * fTemp + iEnd) / (fTemp + 1);
        }
        fTemp = (fTemp / (nCount - 1)) * 65535;
        lpOutput[i] = (fTemp < 65535) ? (USHORT) fTemp : (USHORT) 65535;
    }

    return TRUE;
}
/***************************************************************************
*                               CreateMonoCRD
*  function:
*    this is the function which creates the Color Rendering Dictionary (CRD)
*    from the data supplied in the GrayTRC tag.
*  prototype:
*       BOOL EXTERN CreateMonoCRD(
*                          CHANDLE      cp,
*                          SINT         Index,
*                          MEMPTR       lpMem)
*  parameters:
*       cp          --  Color Profile handle
*       Index       --  Index of the tag
*       lpMem       --  Pointer to the memory block
*  returns:
*       SINT        --  !=0 if the function was successful,
*                         0 otherwise.
*                       Returns number of bytes required/transferred
***************************************************************************/
//  According to the spec this tag-based function only converts from
//  Device to PCS, so we need to create an inverse function to perform
//  PCS->device conversion. By definition the CRD is only
//  for XYZ->DeviceRGB/CMYK conversion.
SINT EXTERN 
CreateMonoCRD (CHANDLE cp, SINT Index, MEMPTR lpMem, DWORD InputIntent)
{
    SINT nCount;
    CSIG Tag, PCS;

    MEMPTR Buff = NULL;
    SINT MemSize = 0;
    MEMPTR lpOldMem = lpMem;
    MEMPTR lpCurve, lpRevCurve;
    HGLOBAL hRevCurve;
    SINT Ret = 0;
    HGLOBAL hMem;
    SINT i;
    MEMPTR lpTable;
    SFLOAT IlluminantWP[3];
    SFLOAT MediaWP[3];
    MEMPTR lpLineStart;
 // Check if we can generate the CRD
    if (!GetCPElementType (cp, Index, (LPCSIG) & Tag) ||
        (Tag != icSigCurveType) ||
        !GetCPConnSpace (cp, (LPCSIG) & PCS) ||
        !GetCPElementSize (cp, Index, (LPSINT) & MemSize) ||
        !MemAlloc (MemSize, (HGLOBAL FAR *)&hMem, (LPMEMPTR) & Buff) ||
        !GetCPElement (cp, Index, Buff, MemSize))
    {
        if (NULL != Buff)
        {
            MemFree (hMem);
        }
        return (0);
    }
    nCount = ui32toSINT (((lpcpCurveType) Buff)->curve.count);

 // Estimate the memory size required to hold CRD
 // SRGB98
    Ret = nCount * 6 * REVCURVE_RATIO + // Number of INT elements
        2048;                           // + other PS stuff
    if (lpMem == NULL)                  // This is a size request
    {
        MemFree (hMem);
        return (Ret);
    }
    if (!MemAlloc (nCount * 2 * (REVCURVE_RATIO + 1),
                   (HGLOBAL FAR *) &hRevCurve, (LPMEMPTR) & lpRevCurve))
    {
        MemFree (hMem);
        return (FALSE);
    }
    lpCurve = lpRevCurve + 2 * REVCURVE_RATIO * nCount;
    GetRevCurve (Buff, lpCurve, lpRevCurve);

 //   GetCPCMMType (cp, (LPCSIG) & Intent);   // Get Intent
    GetCPWhitePoint (cp, (LPSFLOAT) & IlluminantWP);          // .. Illuminant

 // Support absolute whitePoint
    if (InputIntent == icAbsoluteColorimetric)
    {
        if (!GetCPMediaWhitePoint (cp, (LPSFLOAT) & MediaWP)) // .. Media WhitePoint
        {
            MediaWP[0] = IlluminantWP[0];
            MediaWP[1] = IlluminantWP[1];
            MediaWP[2] = IlluminantWP[2];
        }
    }

//************* Start writing  CRD  ****************************
    lpMem += WriteNewLineObject (lpMem, BeginDict);    // Begin dictionary
    lpMem += WriteObject (lpMem, DictType); // Dictionary type

    lpMem += WriteNewLineObject (lpMem, IntentType); // RenderingIntent
    switch (InputIntent)
    {
        case icPerceptual:
            lpMem += WriteObject (lpMem, IntentPer);
            break;

        case icSaturation:
            lpMem += WriteObject (lpMem, IntentSat);
            break;
        
        case icRelativeColorimetric:
            lpMem += WriteObject (lpMem, IntentRCol);
            break;
        
        case icAbsoluteColorimetric:
            lpMem += WriteObject (lpMem, IntentACol);
            break;
    }
    
 //********** Send Black/White Point.
    lpMem += SendCRDBWPoint(lpMem, IlluminantWP);

 //********** /TransformPQR
    lpMem += SendCRDPQR(lpMem, InputIntent, IlluminantWP);

 //********** /MatrixLMN
    lpMem += SendCRDLMN(lpMem, InputIntent, IlluminantWP, MediaWP, PCS);

 //********** /MatrixABC
    if (PCS == icSigXYZData)
    {   // Switch ABC to BAC, since we want to output B which is converted from Y.
        lpMem += WriteNewLineObject (lpMem, MatrixABCTag);
        lpMem += WriteObject (lpMem, MatrixABCXYZCRD);
    }
    else if (PCS == icSigLabData)
    {
        lpMem += WriteNewLineObject (lpMem, MatrixABCTag);
        lpMem += WriteObject (lpMem, MatrixABCLabCRD);
    }
 //********** /EncodeABC
    if (nCount != 0)
    {
        lpMem += WriteObject (lpMem, NewLine);
        lpLineStart = lpMem;
        lpMem += WriteObject (lpMem, EncodeABCTag);
        lpMem += WriteObject (lpMem, BeginArray);
        lpMem += WriteObject (lpMem, BeginFunction);
        if (nCount == 1)                // Gamma supplied in ui16 format
        {
            lpTable = (MEMPTR) (((lpcpCurveType) Buff)->curve.data);
            lpMem += WriteInt (lpMem, ui16toSINT (lpTable));
            lpMem += WriteObject (lpMem, DecodeA3Rev);
        } else
        {
            if (PCS == icSigLabData)
            {
                lpMem += WriteObject (lpMem, EncodeABCLab1);
            }
            lpMem += WriteObject (lpMem, StartClip);
            lpMem += WriteObject (lpMem, BeginArray);
            for (i = 0; i < nCount * REVCURVE_RATIO; i++)
            {
                lpMem += WriteInt (lpMem, (SINT) (*((PUSHORT) lpRevCurve)));
                lpRevCurve += sizeof (icUInt16Number);
                if (((SINT) (lpMem - lpLineStart)) > MAX_LINELENG)
                {
                    lpLineStart = lpMem;
                    lpMem += WriteObject (lpMem, NewLine);
                }
            }
            lpMem += WriteObject (lpMem, EndArray);
            lpLineStart = lpMem;
            lpMem += WriteNewLineObject (lpMem, IndexArray);
            lpMem += WriteObject (lpMem, Scale16);
            lpMem += WriteObject (lpMem, EndClip);
        }
        lpMem += WriteObject (lpMem, EndFunction);
        lpMem += WriteObject (lpMem, DupOp);
        lpMem += WriteObject (lpMem, DupOp);
        lpMem += WriteObject (lpMem, EndArray);
    }
    lpMem += WriteObject (lpMem, EndDict);  // End dictionary definition

    MemFree (hRevCurve);
    MemFree (hMem);
    return ((SINT) ((unsigned long) (lpMem - lpOldMem)));
}

// SRGB98 start
BOOL
GetTRCElementSize(CHANDLE cp, CSIG icSigXTRCTag, LPSINT pIndex, LPSINT pTRCSize)
{
    CSIG Tag;

    if (!GetCPTagIndex (cp, icSigXTRCTag, (LPSINT) pIndex) ||
        !GetCPElementType (cp, *pIndex, (LPCSIG) & Tag) ||
        !(Tag == icSigCurveType) ||
        !GetCPElementSize (cp, *pIndex, (LPSINT) pTRCSize))
    {
        return FALSE;
    }
    return TRUE;
}

BOOL
DoesTRCAndColorantTagExist (CHANDLE cp)
{
    if (DoesCPTagExist (cp, icSigRedColorantTag) &&
        DoesCPTagExist (cp, icSigRedTRCTag ) &&
        DoesCPTagExist (cp, icSigGreenColorantTag) &&
        DoesCPTagExist (cp, icSigGreenTRCTag ) &&
        DoesCPTagExist (cp, icSigBlueColorantTag) &&
        DoesCPTagExist (cp, icSigBlueTRCTag ))
    {
        return TRUE;
    }

    return FALSE;
}

static SINT 
CreateRevArray (CHANDLE cp, MEMPTR lpMem, MEMPTR lpBuff, 
                MEMPTR lpRevCurve, CSIG CPTag, BOOL AllowBinary)
{
    SINT i, nCount;
    SINT MemSize = 0;
    MEMPTR lpOldMem, lpLineStart;
    MEMPTR lpCurve;

    lpOldMem = lpMem;

    lpLineStart = lpMem;
    nCount = ui32toSINT (((lpcpCurveType) lpBuff)->curve.count);
    if (nCount > 1)
    {
        lpMem += WriteNewLineObject (lpMem, Slash);
        lpMem += WriteObject (lpMem, InputArray);
        lpMem += WriteInt (lpMem, (SINT) CPTag);

        lpCurve = lpRevCurve + 2 * REVCURVE_RATIO * nCount;
        GetRevCurve (lpBuff, lpCurve, lpRevCurve);

        if (!AllowBinary)           // Output ASCII DATA
        {
            lpMem += WriteObject (lpMem, BeginArray);
            for (i = 0; i < nCount * REVCURVE_RATIO; i++)
            {
                lpMem += WriteInt (lpMem, (SINT) (*((PUSHORT) lpRevCurve)));
                lpRevCurve += sizeof (icUInt16Number);
                if (((SINT) (lpMem - lpLineStart)) > MAX_LINELENG)
                {
                    lpLineStart = lpMem;
                    lpMem += WriteObject (lpMem, NewLine);
                }
            }
            lpMem += WriteObject (lpMem, EndArray);
        } else
        {                           // Output BINARY DATA
            lpMem += WriteHNAToken (lpMem, 149, nCount);
            lpMem += WriteIntStringU2S_L (lpMem, lpRevCurve, nCount);
        }
    }
    lpMem += WriteObject (lpMem, DefOp);
    return ((SINT) (lpMem - lpOldMem));
}

static SINT 
SendRevArray (CHANDLE cp, MEMPTR lpMem, MEMPTR lpBuff, 
              CSIG CPTag, BOOL AllowBinary)
{
    SINT nCount;
    MEMPTR lpOldMem;
    MEMPTR lpTable;

    lpOldMem = lpMem;
    lpMem += WriteObject (lpMem, BeginFunction);
    nCount = ui32toSINT (((lpcpCurveType) lpBuff)->curve.count);
    if (nCount != 0)
    {
        if (nCount == 1)            // Gamma supplied in ui16 format
        {
            lpTable = (MEMPTR) (((lpcpCurveType) lpBuff)->curve.data);
            lpMem += WriteInt (lpMem, ui16toSINT (lpTable));
            lpMem += WriteObject (lpMem, DecodeA3Rev);
        } else
        {
            lpMem += WriteObject (lpMem, StartClip);
            lpMem += WriteObject (lpMem, InputArray);
            lpMem += WriteInt (lpMem, (SINT) CPTag);

            if (!AllowBinary)       // Output ASCII CS
            {
                lpMem += WriteObject (lpMem, IndexArray);
            } else
            {                       // Output BINARY CS
                lpMem += WriteObject (lpMem, IndexArray16b);
            }
            lpMem += WriteObject (lpMem, Scale16);
            lpMem += WriteObject (lpMem, EndClip);
        }
    }
    lpMem += WriteObject (lpMem, EndFunction);
    return ((SINT) (lpMem - lpOldMem));
}

SINT 
CreateColorantArray(CHANDLE cp, double FAR *lpArray, CSIG CPTag)
{
    SINT i, Index;
    MEMPTR lpTable;
    MEMPTR Buff = NULL;
    SINT MemSize = 0;
    HGLOBAL hBuff;

    if (GetCPTagIndex (cp, CPTag, (LPSINT) & Index) &&
        GetCPElementSize (cp, Index, (LPSINT) & MemSize) &&
        MemAlloc (MemSize, (HGLOBAL FAR *)&hBuff, (LPMEMPTR) & Buff) &&
        GetCPElement (cp, Index, Buff, MemSize))
    {
        lpTable = (MEMPTR) & (((lpcpXYZType) Buff)->data);
        for (i = 0; i < 3; i++)
        {
            *lpArray++ = (SFLOAT)si16f16toSFLOAT (lpTable);
            lpTable += sizeof (icS15Fixed16Number);
        }
        MemFree (hBuff);
    }
    return ( TRUE );
}  


/***************************************************************************
*                           IsSRGB
*  function: check if the profile is sRGB
*
*  parameters:
*       cp          --  Color Profile handle
*
*  returns:
*       BOOL        --  TRUE if the profile is sRGB
*                       FALSE otherwise.
***************************************************************************/
BOOL FAR IsSRGB (CHANDLE cp)
{   
    BOOL    match = FALSE;
    SINT    RedTRCIndex, GreenTRCIndex, BlueTRCIndex, RedCIndex, GreenCIndex, BlueCIndex;
    SINT    MemSize;
    SINT    RedTRCSize=0, GreenTRCSize=0, BlueTRCSize=0, RedCSize=0, GreenCSize=0, BlueCSize=0;
    HGLOBAL hMem;
    MEMPTR  lpRed = NULL,lpGreen, lpBlue, lpRedC, lpGreenC, lpBlueC;
    DWORD   crc;

    if (GetCPTagIndex (cp, icSigRedTRCTag, &RedTRCIndex) &&
        GetCPElementSize (cp, RedTRCIndex, &RedTRCSize)  &&
            
        GetCPTagIndex (cp, icSigGreenTRCTag, &GreenTRCIndex) &&
        GetCPElementSize (cp, GreenTRCIndex, &GreenTRCSize)  &&
            
        GetCPTagIndex (cp, icSigBlueTRCTag, &BlueTRCIndex) &&
        GetCPElementSize (cp, BlueTRCIndex, &BlueTRCSize) && 
            
        GetCPTagIndex (cp, icSigRedColorantTag, &RedCIndex) &&
        GetCPElementSize (cp, RedCIndex, &RedCSize)  &&
            
        GetCPTagIndex (cp, icSigGreenColorantTag, &GreenCIndex) &&
        GetCPElementSize (cp, GreenCIndex, &GreenCSize)  &&
            
        GetCPTagIndex (cp, icSigBlueColorantTag, &BlueCIndex) &&
        GetCPElementSize (cp, BlueCIndex, &BlueCSize) )
    {
        MemSize = RedTRCSize + GreenTRCSize + BlueTRCSize + RedCSize + GreenCSize + BlueCSize;

        if ( (MemSize == 6240) && // #bytes in sRGBColorTags
             MemAlloc (MemSize, (HGLOBAL FAR *)&hMem, (LPMEMPTR)&lpRed) )
        {
            lpGreen = lpRed + RedTRCSize;
            lpBlue = lpGreen + GreenTRCSize;
            lpRedC = lpBlue + BlueTRCSize;
            lpGreenC = lpRedC + RedCSize;
            lpBlueC = lpGreenC + GreenCSize;
            
            if (GetCPElement (cp, RedTRCIndex, lpRed, RedTRCSize) &&
                GetCPElement (cp, GreenTRCIndex, lpGreen, GreenTRCSize ) &&
                GetCPElement (cp, BlueTRCIndex, lpBlue, BlueTRCSize ) &&
                GetCPElement (cp, RedCIndex, lpRedC, RedCSize) &&
                GetCPElement (cp, GreenCIndex, lpGreenC, GreenCSize ) &&
                GetCPElement (cp, BlueCIndex, lpBlueC, BlueCSize ))
            {   
                crc = crc32( lpRed, 6240 );    
                match = (crc == SRGBCRC);
            }
            MemFree(hMem);
        }    
    }            
    return (match);
}

/***************************************************************************
*                           CreateMatrixCRD
*  function:
*    this is the function which creates the Color Rendering Dictionary (CRD)
*    from the data supplied in the redTRC, greenTRC, blueTRA, redColorant,
*    greenColorant and BlueColorant tags
*  prototype:
*       BOOL EXTERN CreateMatrixCRD(
*                          CHANDLE      cp,
*                          MEMPTR       lpMem,
*                          BOOL         AllowBinary)
*  parameters:
*       cp          --  Color Profile handle
*       lpMem       --  Pointer to the memory block
*       AllowBinary --
*  returns:
*       SINT        --  !=0 if the function was successful,
*                         0 otherwise.
*                       Returns number of bytes required/transferred
***************************************************************************/
// With matrix/TRC model, only the CIEXYZ encoding of the PCS can be used.
// So, we don't need to worry about CIELAB.
SINT EXTERN 
CreateMatrixCRD (CHANDLE cp, MEMPTR lpMem, DWORD InputIntent, BOOL AllowBinary)
{
    SINT RedTRCIndex, GreenTRCIndex, BlueTRCIndex;
    SINT i, MemSize;
    SINT nRedCount, nGreenCount, nBlueCount;
    MEMPTR lpRed = NULL,lpGreen, lpBlue;
    SINT RedTRCSize = 0, GreenTRCSize = 0, BlueTRCSize = 0;
    MEMPTR lpOldMem = lpMem;
    MEMPTR lpRevCurve;
    HGLOBAL hRevCurve;
    SINT Ret = 0;
    HGLOBAL hMem;
    SFLOAT IlluminantWP[3];
    double Colorant[9];
    double RevColorant[9];
 
 // Check if we can generate the CRD
    if (!GetTRCElementSize(cp, icSigRedTRCTag, &RedTRCIndex, &RedTRCSize) ||
        !GetTRCElementSize(cp, icSigGreenTRCTag, &GreenTRCIndex, &GreenTRCSize) ||
        !GetTRCElementSize(cp, icSigBlueTRCTag, &BlueTRCIndex, &BlueTRCSize))
    {
        return 0;
    }
    MemSize = RedTRCSize + GreenTRCSize + BlueTRCSize;
    if (!MemAlloc (MemSize, (HGLOBAL FAR *)&hMem, (LPMEMPTR) & lpRed))
        return 0;

    lpGreen = lpRed + RedTRCSize;
    lpBlue = lpGreen + GreenTRCSize;
    if (!GetCPElement (cp, RedTRCIndex, lpRed, RedTRCSize) ||
        !GetCPElement (cp, GreenTRCIndex, lpGreen, GreenTRCSize ) ||
        !GetCPElement (cp, BlueTRCIndex, lpBlue, BlueTRCSize ))
    {
        MemFree (hMem);
        return (0);
    }
    nRedCount = ui32toSINT (((lpcpCurveType) lpRed)->curve.count);
    nGreenCount = ui32toSINT (((lpcpCurveType) lpGreen)->curve.count);
    nBlueCount = ui32toSINT (((lpcpCurveType) lpBlue)->curve.count);

 // Estimate the memory size required to hold CRD
    Ret = (nRedCount + nGreenCount + nBlueCount) * 6 * REVCURVE_RATIO + 
        2048;                // Number of INT elements + other PS stuff
    if (lpMem == NULL)       // This is a size request
    {
        MemFree (hMem);
        return (Ret);
    }

    if (!MemAlloc (nRedCount * 2 * (REVCURVE_RATIO + 1),
                  (HGLOBAL FAR *) &hRevCurve, (LPMEMPTR) & lpRevCurve))
    {
        MemFree (hMem);
        return (0);
    }

    if (IsSRGB(cp))
    {
       lpMem += WriteNewLineObject (lpMem, CRDBegin);
       lpMem += WriteNewLineObject (lpMem, BeginDict);    // Begin dictionary
       lpMem += WriteObject (lpMem, DictType); // Dictionary type
       lpMem += WriteNewLineObject (lpMem, IntentType); // RenderingIntent
       switch (InputIntent)
       {
           case icPerceptual:
               lpMem += WriteObject (lpMem, IntentPer);
               break;

           case icSaturation:
               lpMem += WriteObject (lpMem, IntentSat);
               break;
        
           case icRelativeColorimetric:
               lpMem += WriteObject (lpMem, IntentRCol);
               break;
        
           case icAbsoluteColorimetric:
               lpMem += WriteObject (lpMem, IntentACol);
               break;
       }
       lpMem += WriteNewLineObject (lpMem, AdobeCRD);
       lpMem += WriteNewLineObject (lpMem, CRDEnd);
    }
    else
    {
       lpMem += EnableGlobalDict(lpMem);
       lpMem += BeginGlobalDict(lpMem);
       
       lpMem += CreateRevArray (cp, lpMem, lpRed, lpRevCurve, icSigRedTRCTag, AllowBinary);
       lpMem += CreateRevArray (cp, lpMem, lpGreen, lpRevCurve, icSigGreenTRCTag, AllowBinary);
       lpMem += CreateRevArray (cp, lpMem, lpBlue, lpRevCurve, icSigBlueTRCTag, AllowBinary);
   
       lpMem += EndGlobalDict(lpMem);
   
    //   GetCPCMMType (cp, (LPCSIG) & Intent);   // Get Intent
       GetCPWhitePoint (cp, (LPSFLOAT) & IlluminantWP);          // .. Illuminant
   
    //************* Start writing  CRD  ****************************
       lpMem += WriteNewLineObject (lpMem, CRDBegin);
   
       lpMem += WriteNewLineObject (lpMem, BeginDict);    // Begin dictionary
       lpMem += WriteObject (lpMem, DictType); // Dictionary type
                            
       lpMem += WriteNewLineObject (lpMem, IntentType); // RenderingIntent
       switch (InputIntent)
       {
           case icPerceptual:
               lpMem += WriteObject (lpMem, IntentPer);
               break;

           case icSaturation:
               lpMem += WriteObject (lpMem, IntentSat);
               break;
        
           case icRelativeColorimetric:
               lpMem += WriteObject (lpMem, IntentRCol);
               break;
        
           case icAbsoluteColorimetric:
               lpMem += WriteObject (lpMem, IntentACol);
               break;
       }
                     
    //********** Send Black/White Point.
       lpMem += SendCRDBWPoint(lpMem, IlluminantWP);
   
    //********** /TransformPQR
       lpMem += SendCRDPQR(lpMem, InputIntent, IlluminantWP);
   
    //********** /MatrixLMN
       CreateColorantArray(cp, &Colorant[0], icSigRedColorantTag);
       CreateColorantArray(cp, &Colorant[3], icSigGreenColorantTag);
       CreateColorantArray(cp, &Colorant[6], icSigBlueColorantTag);
       InvertMatrix (Colorant, RevColorant);
   
       lpMem += WriteNewLineObject (lpMem, MatrixLMNTag);
   
       lpMem += WriteObject (lpMem, BeginArray);
       for (i = 0; i < 9; i++)
       {
           lpMem += WriteFloat (lpMem, (SFLOAT)RevColorant[i]);
       }
       lpMem += WriteObject (lpMem, EndArray);
   
    //********** /EncodeABC
       lpMem += WriteNewLineObject (lpMem, EncodeABCTag);
       lpMem += WriteObject (lpMem, BeginArray);
   
       lpMem += WriteObject (lpMem, NewLine);
       lpMem += SendRevArray (cp, lpMem, lpRed, icSigRedTRCTag, AllowBinary);
       lpMem += WriteObject (lpMem, NewLine);
       lpMem += SendRevArray (cp, lpMem, lpGreen, icSigGreenTRCTag, AllowBinary);
       lpMem += WriteObject (lpMem, NewLine);
       lpMem += SendRevArray (cp, lpMem, lpBlue, icSigBlueTRCTag, AllowBinary);
       lpMem += WriteNewLineObject (lpMem, EndArray);
   
       lpMem += WriteObject (lpMem, EndDict);  // End dictionary definition
   
       lpMem += WriteNewLineObject (lpMem, CRDEnd);
    }
    MemFree (hRevCurve);
    MemFree (hMem);
    return ((SINT) ((unsigned long) (lpMem - lpOldMem)));
}
// SRGB98 End

BOOL EXTERN
GetPS2ColorRenderingDictionary (
                                CHANDLE cp,
                                DWORD Intent,
                                MEMPTR lpMem,
                                LPDWORD lpcbSize,
                                BOOL AllowBinary)
{
    SINT Index;
    SINT Ret, Size;
    CSIG icSigPs2CRDx, icSigBToAx;

    if (!cp)
        return FALSE;

    if ((lpMem == NULL) || (*lpcbSize == 0))
    {
        lpMem = NULL;
        *lpcbSize = 0;
    }
    Ret = 0;
    Size = (SINT) * lpcbSize;

    switch (Intent)
    {
        case icPerceptual:
            icSigPs2CRDx = icSigPs2CRD0Tag;
            icSigBToAx = icSigBToA0Tag;
            break;

        case icRelativeColorimetric:
            icSigPs2CRDx = icSigPs2CRD1Tag;
            icSigBToAx = icSigBToA1Tag;
            break;

        case icSaturation:
            icSigPs2CRDx = icSigPs2CRD2Tag;
            icSigBToAx = icSigBToA2Tag;
            break;

        case icAbsoluteColorimetric:
            icSigPs2CRDx = icSigPs2CRD3Tag;
            icSigBToAx = icSigBToA1Tag;
            break;

        default:
            *lpcbSize = (DWORD) Ret;
            return (Ret > 0);
    }

    if (
        (DoesCPTagExist (cp, icSigPs2CRDx) &&
         GetCPTagIndex (cp, icSigPs2CRDx, (LPSINT) & Index) &&
         GetCPElementDataSize (cp, Index, (LPSINT) & Ret) &&
         ((Size == 0) ||
          GetCPElementData (cp, Index, lpMem, Size)) &&
         (Ret = Convert2Ascii (cp, Index, lpMem, Size, Ret, AllowBinary))
        ) ||
        (DoesCPTagExist (cp, icSigBToAx) &&
         GetCPTagIndex (cp, icSigBToAx, (LPSINT) & Index) &&
         (Ret = CreateLutCRD (cp, Index, lpMem, Intent, AllowBinary))
        ) || 
        // SRGB98 Support Windows 98 sRGB icc profile.
        // Create CRD from TRC and Colorant Tags.
        (DoesTRCAndColorantTagExist (cp) &&
         (Ret = CreateMatrixCRD (cp, lpMem, Intent, AllowBinary))
        ) ||
        (DoesCPTagExist (cp, icSigGrayTRCTag) &&
         GetCPTagIndex (cp, icSigGrayTRCTag, (LPSINT) & Index) &&
         (Ret = CreateMonoCRD (cp, Index, lpMem, Intent))
        )
       )
    {
    }

    *lpcbSize = (DWORD) Ret;
    return (Ret > 0);
}
