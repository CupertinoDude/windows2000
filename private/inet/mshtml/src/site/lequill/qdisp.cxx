/*
 *  QDISP.C
 *
 *  Purpose:
 *      CDisplay class
 *
 *  Owner:
 *      Original RichEdit code: David R. Fulmer
 *      Christian Fortini
 *      Murray Sargent
 *
 * This is the version customized for hosting Quill.
 *
 */

#include "headers.hxx"

#ifndef X__DISP_H_
#define X__DISP_H_
#include "_disp.h"
#endif

#ifndef X__FONT_H_
#define X__FONT_H_
#include "_font.h"
#endif

#ifndef X_MISCPROT_H_
#define X_MISCPROT_H_
#include "miscprot.h"
#endif

#ifndef X_TREEPOS_HXX_
#define X_TREEPOS_HXX_
#include "treepos.hxx"
#endif

#ifndef X_DOWNLOAD_HXX_
#define X_DOWNLOAD_HXX_
#include "download.hxx"
#endif

#ifndef X_TASKMAN_HXX_
#define X_TASKMAN_HXX_
#include "taskman.hxx"
#endif

#ifndef X_CSITE_HXX_
#define X_CSITE_HXX_
#include "csite.hxx"
#endif

#ifndef X_DOCPRINT_HXX_
#define X_DOCPRINT_HXX_
#include "docprint.hxx"
#endif

#ifndef X_FPRINT_HXX_
#define X_FPRINT_HXX_
#include "fprint.hxx"
#endif

#ifndef X_COLLECT_HXX_
#define X_COLLECT_HXX_
#include "collect.hxx"
#endif

#ifndef X_RCLCLPTR_HXX_
#define X_RCLCLPTR_HXX_
#include "rclclptr.hxx"
#endif

#ifndef X_TABLE_HXX_
#define X_TABLE_HXX_
#include "table.hxx"
#endif

#ifndef X_PEER_HXX_
#define X_PEER_HXX_
#include "peer.hxx"
#endif

#ifndef X_DEBUGPAINT_HXX_
#define X_DEBUGPAINT_HXX_
#include "debugpaint.hxx"
#endif

#ifndef X_LTABLE_HXX_
#define X_LTABLE_HXX_
#include "ltable.hxx"
#endif

#ifndef X_DISPTREE_H_
#define X_DISPTREE_H_
#pragma INCMSG("--- Beg <disptree.h>")
#include <disptree.h>
#pragma INCMSG("--- End <disptree.h>")
#endif

#ifndef X_REGION_HXX_
#define X_REGION_HXX_
#include "region.hxx"
#endif

#ifndef X_DISPTYPE_HXX_
#define X_DISPTYPE_HXX_
#include "disptype.hxx"
#endif

#ifndef X_DISPNODE_HXX_
#define X_DISPNODE_HXX_
#include "dispnode.hxx"
#endif

#ifndef X_DISPROOT_HXX_
#define X_DISPROOT_HXX_
#include "disproot.hxx"
#endif

#ifdef QUILL

#ifndef X_QUILGLUE_HXX_
#define X_QUILGLUE_HXX_
#include "quilglue.hxx"
#endif

#endif  // QUILL

#ifndef X_LSRENDER_HXX_
#define X_LSRENDER_HXX_
#include "lsrender.hxx"
#endif

#ifndef X_SWITCHES_HXX_
#define X_SWITCHES_HXX_
#include "switches.hxx"
#endif

EXTERN_C const GUID CLSID_HTMLCheckboxElement;
EXTERN_C const GUID CLSID_HTMLRadioElement;

// Timer tick counts for background task
#define cmsecBgndInterval   300
#define cmsecBgndBusy       100

// Lines ahead
const LONG g_cExtraBeforeLazy = 10;

PerfDbgTag(tagRecalc, "Layout Recalc Engine", "Layout recalcEngine");
DeclareTag(tagPositionObjects, "PositionObjects", "PositionObjects");
DeclareTag(tagRenderingRect, "Rendering rect", "Rendering rect");
DeclareTag(tagRelDispNodeCache, "Relative disp node cache", "Trace changes to disp node cache");
DeclareTag(tagTableCalcDontReuseMeasurer, "Tables", "Disable measurer reuse across Table cells");
PerfDbgExtern(tagPaintWait);

MtDefine(CRecalcTask, Layout, "CRecalcTask")
MtDefine(CDisplay, Layout, "CDisplay")
MtDefine(CDisplay_aryRegionCollection_pv, CDisplay, "CDisplay::_aryRegionCollection::_pv")
MtDefine(CRelDispNodeCache, CDisplay, "CRelDispNodeCache::_pv")
MtDefine(CDisplayUpdateView_aryInvalRects_pv, Locals, "CDisplay::UpdateView aryInvalRects::_pv")
MtDefine(CDisplayDrawBackgroundAndBorder_aryRects_pv, Locals, "CDisplay::DrawBackgroundAndBorder aryRects::_pv")
MtDefine(CDisplayDrawBackgroundAndBorder_aryNodesWithBgOrBorder_pv, Locals, "CDisplay::DrawBackgroundAndBorder aryNodesWithBgOrBorder::_pv")

//
// This function does exactly what IntersectRect does, except that
// if one of the rects is empty, it still returns TRUE if the rect
// is located inside the other rect. [ IntersectRect rect in such
// case returns FALSE. ]
//

BOOL
IntersectRectE (RECT * prRes, const RECT * pr1, const RECT * pr2)
{
    // nAdjust is used to control what statement do we use in conditional
    // expressions: a < b or a <= b. nAdjust can be 0 or 1;
    // when (nAdjust == 0): (a - nAdjust < b) <==> (a <  b)  (*)
    // when (nAdjust == 1): (a - nAdjust < b) <==> (a <= b)  (**)
    // When at least one of rects to intersect is empty, and the empty
    // rect lies on boundary of the other, then we consider that the
    // rects DO intersect - in this case nAdjust == 0 and we use (*).
    // If both rects are not empty, and rects touch, then we should
    // consider that they DO NOT intersect and in that case nAdjust is
    // 1 and we use (**).
    //
    int nAdjust;

    Assert (prRes && pr1 && pr2);
    Assert (pr1->left <= pr1->right && pr1->top <= pr1->bottom &&
            pr2->left <= pr2->right && pr2->top <= pr2->bottom);

    prRes->left  = max (pr1->left,  pr2->left);
    prRes->right = min (pr1->right, pr2->right);
    nAdjust = (int) ( (pr1->left != pr1->right) && (pr2->left != pr2->right) );
    if (prRes->right - nAdjust < prRes->left)
        goto NoIntersect;

    prRes->top    = max (pr1->top,  pr2->top);
    prRes->bottom = min (pr1->bottom, pr2->bottom);
    nAdjust = (int) ( (pr1->top != pr1->bottom) && (pr2->top != pr2->bottom) );
    if (prRes->bottom - nAdjust < prRes->top)
        goto NoIntersect;

    return TRUE;

NoIntersect:
    SetRect (prRes, 0,0,0,0);
    return FALSE;
}

#if DBG == 1
//
// because IntersectRectE is quite fragile on boundary cases and these
// cases are not obvious, and also because bugs on these boundary cases
// would manifest in a way difficult to debug, we use this function to
// assert (in debug build only) that the function returns results we
// expect.
//
void
AssertIntersectRectE ()
{
    struct  ASSERTSTRUCT
    {
        RECT    r1;
        RECT    r2;
        RECT    rResExpected;
        BOOL    fResExpected;
    };

    ASSERTSTRUCT ts [] =
    {
        //  r1                  r2                  rResExpected      fResExpected
        // 1st non-empty, no intersect
        { {  0,  2, 99,  8 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },
        { {  0, 22, 99, 28 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },
        // 1st non-empty, intersect
        { {  0,  2, 99, 18 }, {  0, 10, 99, 20 }, {  0, 10, 99, 18 }, TRUE  },
        { {  0, 12, 99, 28 }, {  0, 10, 99, 20 }, {  0, 12, 99, 20 }, TRUE  },
        { {  0, 12, 99, 18 }, {  0, 10, 99, 20 }, {  0, 12, 99, 18 }, TRUE  },
        { {  0,  2, 99, 28 }, {  0, 10, 99, 20 }, {  0, 10, 99, 20 }, TRUE  },
        // 1st non-empty, touch
        { {  0,  2, 99, 10 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },
        { {  0, 20, 99, 28 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },

        // 1st empty, no intersect
        { {  0,  2, 99,  2 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },
        { {  0, 28, 99, 28 }, {  0, 10, 99, 20 }, {  0,  0,  0,  0 }, FALSE },
        // 1st empty, intersect
        { {  0, 12, 99, 12 }, {  0, 10, 99, 20 }, {  0, 12, 99, 12 }, TRUE  },
        // 1st empty, touch
        { {  0, 10, 99, 10 }, {  0, 10, 99, 20 }, {  0, 10, 99, 10 }, TRUE  },
        { {  0, 20, 99, 20 }, {  0, 10, 99, 20 }, {  0, 20, 99, 20 }, TRUE  },

        // both empty
        { {  0, 10, 99, 10 }, {  0, 10, 99, 10 }, {  0, 10, 99, 10 }, TRUE  }
    };

    ASSERTSTRUCT *  pts;
    RECT            r1;
    RECT            r2;
    RECT            rResActual;
    RECT            rResExpected;
    BOOL            fResActual;
    int             c;

    for (
        c = ARRAY_SIZE(ts), pts = &ts[0];
        c;
        c--, pts++)
    {
        // test
        fResActual = IntersectRectE(&rResActual, &pts->r1, &pts->r2);
        if (!EqualRect(&rResActual, &pts->rResExpected) || fResActual != pts->fResExpected)
            goto Failed;

        // now swap rects and test
        fResActual = IntersectRectE(&rResActual, &pts->r2, &pts->r1);
        if (!EqualRect(&rResActual, &pts->rResExpected) || fResActual != pts->fResExpected)
            goto Failed;

        // now swap left<->top and right<->bottom
        //   swapped left         top           right           bottom
        SetRect (&r1, pts->r1.top, pts->r1.left, pts->r1.bottom, pts->r1.right);
        SetRect (&r2, pts->r2.top, pts->r2.left, pts->r2.bottom, pts->r2.right);
        SetRect (&rResExpected, pts->rResExpected.top, pts->rResExpected.left, pts->rResExpected.bottom, pts->rResExpected.right);

        // test
        fResActual = IntersectRectE(&rResActual, &r1, &r2);
        if (!EqualRect(&rResActual, &rResExpected) || fResActual != pts->fResExpected)
            goto Failed;

        // now swap rects and test
        fResActual = IntersectRectE(&rResActual, &r2, &r1);
        if (!EqualRect(&rResActual, &rResExpected) || fResActual != pts->fResExpected)
            goto Failed;

    }

    return;

Failed:
    Assert (0 && "IntersectRectE returns an unexpected result");
}
#endif

// ===========================  CLed  =====================================================


void CLed::SetNoMatch()
{
    _cpMatchNew  = _cpMatchOld  =
    _iliMatchNew = _iliMatchOld =
    _yMatchNew   = _yMatchOld   = MAXLONG;
}


//-------------------- Start: Code to implement background recalc in lightwt tasks

class CRecalcTask : public CTask
{
public:

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CRecalcTask))

    CRecalcTask (CDisplay *pdp, DWORD grfLayout)
    {
        _pdp = pdp ;
        _grfLayout = grfLayout;
    }

    virtual void OnRun (DWORD dwTimeOut)
    {
        _pdp->StepBackgroundRecalc (dwTimeOut, _grfLayout) ;
    }

    virtual void OnTerminate () {}

private:
    CDisplay *_pdp ;
    DWORD     _grfLayout;
} ;

//-------------------- End: Code to implement background recalc in lightwt tasks


// ===========================  CDisplay  =====================================================

CDisplay::~CDisplay()
{
    // The recalc task should have disappeared during the detach!
    Assert (!HasBgRecalcInfo() && !RecalcTask());
}

CElement *
CDisplay::GetFlowLayoutElement() const
{
    return GetFlowLayout()->ElementContent();
}

CMarkup * CDisplay::GetMarkup() const
{
    return GetFlowLayout()->GetContentMarkup();
}

CDisplay::CDisplay ()
{
#if DBG == 1
    _pFL = CONTAINING_RECORD(this, CFlowLayout, _dp);
#endif
    _fRecalcDone = TRUE;

#if DBG == 1
    AssertIntersectRectE ();
#endif
}

//+----------------------------------------------------------------------------
//  Member:     CDisplay::Init
//
//  Synopsis:   Initializes CDisplay
//
//  Returns:    TRUE - initialization succeeded
//              FALSE - initalization failed
//
//+----------------------------------------------------------------------------
BOOL CDisplay::Init()
{
    CFlowLayout * pFL = GetFlowLayout();

    Assert( _yCalcMax     == 0 );        // Verify allocation zeroed memory out
    Assert( _xWidth       == 0 );
    Assert( _yHeight      == 0 );
    Assert( RecalcTask()  == NULL );

    SetWordWrap(pFL->GetWordWrap());

#ifndef DISPLAY_TREE
    if (_fPrinting)
    {
        // In a frame HTML document hierarchy, only the rootdoc is a printdoc during printing.
        // Embedded docs are just CDoc's.
        CPrintDoc * pPrintDoc = DYNCAST(CPrintDoc, pFL->Doc()->GetRootDoc());

        // Sanity check.
        Assert( pFL->Doc()->IsPrintDoc() );

        _xWidthView  = pPrintDoc->_dci._sizeDst.cx;
        _yHeightView = pPrintDoc->_dci._sizeDst.cy;

        pFL->EnsureBgRecalcInfo();

        Assert(pFL->BgRecalcInfo() && "Should have a BgRecalcInfo");
        if (pFL->HasBgRecalcInfo())
        {
            pFL->BgRecalcInfo()->_yWait = MAXLONG;
        }
    }
#endif

    return TRUE;
}


//+------------------------------------------------------------------------
//
//  Member:     Detach
//
//  Synopsis:   Do stuff before dying
//
//-------------------------------------------------------------------------
void
CDisplay::Detach()
{
    // If there's a timer on, get rid of it before we detach the
    // object. This prevents us from trying to recalc the lines
    // after the CTxtSite has gone away.
    FlushRecalc();
}

/*
 *  CDisplay::GetFirstCp
 *
 *  @mfunc
 *      Return the first cp
 */
LONG
CDisplay::GetFirstCp() const
{
    return GetFlowLayout()->GetContentFirstCp();
}

/*
 *  CDisplay::GetLastCp
 *
 *  @mfunc
 *      Return the last cp
 */
LONG
CDisplay::GetLastCp() const
{
    return GetFlowLayout()->GetContentLastCp();
}

#ifndef DISPLAY_TREE
/*
 *  CDisplay::GetFirstVisibleCp
 *
 *  @mfunc
 *      Return the first visible cp
 */
LONG
CDisplay::GetFirstVisibleCp() const
{
    CFlowLayout * pFL = GetFlowLayout();

    return pFL->GetFirstCp() + pFL->FirstVisibleDCp();
}
#endif

/*
 *  CDisplay::GetMaxCpCalced
 *
 *  @mfunc
 *      Return the last cp calc'ed. Note that this is
 *      relative to the start of the site, not the story.
 */
LONG
CDisplay::GetMaxCpCalced() const
{
    if (GetFlowLayout()->FExternalLayout())
    {
        if (GetFlowLayout()->GetQuillGlue())
            return GetFlowLayout()->GetQuillGlue()->GetMaxCpCalced();

        return GetFlowLayout()->GetContentFirstCp();
    }

    return GetFlowLayout()->GetContentFirstCp() + _dcpCalcMax;
}


inline BOOL
CDisplay::AllowBackgroundRecalc(CCalcInfo * pci, BOOL fBackground)
{
    CFlowLayout * pFL = GetFlowLayout();

#ifdef SWITCHES_ENABLED
    if (IsSwitchNoBgRecalc())
        return(FALSE);
#endif

    // Allow background recalc when:
    //  a) Not currently calcing in the background
    //  b) It is a SIZEMODE_NATURAL request
    //  c) The CTxtSite does not size to its contents
    //  d) The site is not part of a print document
    //  e) The site allows background recalc
    return (!fBackground                             &&
            (pci->_smMode == SIZEMODE_NATURAL)       &&
            !pFL->_fContentsAffectSize   &&
            !pFL->GetAutoSize()          &&
            !_fPrinting                  &&
            !pFL->TestClassFlag(CElement::ELEMENTDESC_NOBKGRDRECALC));
}


/*
 *  CDisplay::FlushRecalc()
 *
 *  @mfunc
 *      Destroys the line array, therefore ensure a full recalc next time
 *      RecalcView or UpdateView is called.
 *
 */
void CDisplay::FlushRecalc()
{
    CFlowLayout * pFL = GetFlowLayout();

    StopBackgroundRecalc();

    if (LineCount())
    {
        Remove(0, -1, AF_KEEPMEM);          // Remove all old lines from *this
    }

    TraceTag((tagRelDispNodeCache, "SN: (%d) FlushRecalc:: invalidating rel line cache",
                                GetFlowLayout()->SN()));

    pFL->_fContainsRelative = FALSE;
    pFL->CancelChanges();
    pFL->MarkHasAlignedLayouts(FALSE);

    VoidRelDispNodeCache();
    DestroyFlowDispNodes();

    _fRecalcDone = FALSE;
    _yCalcMax   = 0;                        // Set both maxes to start of text
    _dcpCalcMax = 0;                        // Relative to the start cp of site.
    _xWidth     = 0;
    _yHeight    = 0;
    _yHeightMax = 0;
    _fHasEmbeddedLayouts = FALSE;
}

//+---------------------------------------------------------------------------
//
//  Member:     NoteMost
//
//  Purpose:    Notes if the line has anything which will need us to compute
//              information for negative lines/absolute or relative divs
//
//----------------------------------------------------------------------------
void
CDisplay::NoteMost(CLine *pli)
{
    Assert (pli);

    if ( !_fRecalcMost && (pli->GetYMostTop() < 0 || pli->_fHasAbsoluteElt))
    {
        _fRecalcMost = TRUE;
    }
}

//+---------------------------------------------------------------------------
//
//  Member:     RecalcMost
//
//  Purpose:    Calculates the most negative line and most positive/negative
//              positioned site from scratch.
//
//  BUGBUG(sujalp): We initially had an incremental way of computing both the
//  negative line hts info AND +/- positioned site info. However, this logic was
//  incorrect for consecutive lines with -ve line height. So we changed it so
//  that we compute this AND +/- info always. If this becomes a performance issue
//  we could go back for incremental computation for div's easily -- but would
//  have to maintain extra state information. For negative line heights we could
//  also do some incremental stuff, but it would be much much more complicated
//  than what we have now.
//
//----------------------------------------------------------------------------
void
CDisplay::RecalcMost()
{
    if (_fRecalcMost)
    {
        CFlowLayout * pFlowLayout = GetFlowLayout();
        LONG ili;

        long yNegOffset = 0;        // offset at which the current line is drawn
                                    // as a result of a series of lines with negative
                                    // height
        long yPosOffset = 0;

        long yBottomOffset = 0;     // offset by which the current lines contents
                                    // extend beyond the yHeight of the line.
        long yTopOffset = 0;        // offset by which the current lines contents
                                    // extend before the current y position

        pFlowLayout->_fContainsAbsolute = FALSE;
        _yMostNeg = 0;
        _yMostPos = 0;

        for (ili = 0; ili < LineCount(); ili++)
        {
            CLine *pli = Elem (ili);
            LONG   yLineBottomOffset = pli->GetYHeightBottomOff();

            // Update most positive/negative positions for relative positioned sites
            if (pli->_fHasAbsoluteElt)
                pFlowLayout->_fContainsAbsolute = FALSE;

            // top offset of the current line
            yTopOffset = pli->GetYMostTop() + yNegOffset;

            yBottomOffset = yLineBottomOffset + yPosOffset;

            // update the most negative value if the line has negative before space
            // or line height < actual extent
            if(yTopOffset < 0 && _yMostNeg > yTopOffset)
            {
                _yMostNeg = yTopOffset;
            }

            if (yBottomOffset > 0 && _yMostPos < yBottomOffset)
            {
                _yMostPos = yBottomOffset;
            }

            // if the current line forces a new line and has negative height
            // update the negative offset at which the next line is drawn.
            if(pli->_fForceNewLine)
            {
                if(pli->_yHeight < 0)
                {
                    yNegOffset += pli->_yHeight;
                }
                else
                {
                    yNegOffset = 0;
                }

                if (yLineBottomOffset > 0)
                {
                    yPosOffset += yLineBottomOffset;
                }
                else
                {
                    yPosOffset = 0;
                }
            }
        }

        _fRecalcMost = FALSE;
    }
}

//+---------------------------------------------------------------------------
//
//  Member:     RecalcPlainTextSingleLine
//
//  Purpose:    A high-performance substitute for RecalcView. Does not go
//              through Line Services. Can only be used to measure a single
//              line of plain text (i.e. no HTML).
//
//----------------------------------------------------------------------------

BOOL
CDisplay::RecalcPlainTextSingleLine(CCalcInfo * pci)
{
    CFlowLayout *       pFlowLayout = GetFlowLayout();
    CTreeNode *         pNode       = pFlowLayout->GetFirstBranch();
    TCHAR               chPassword  = pFlowLayout->GetPasswordCh();
    long                cch         = pFlowLayout->GetContentTextLength();
    const CCharFormat * pCF         = pNode->GetCharFormat();
    const CParaFormat * pPF         = pNode->GetParaFormat();
    CCcs *              pccs;
    long                lWidth;
    long                lCharWidth;
    long                xShift;
    long                xWidth, yHeight;
    long                lPadding[PADDING_MAX];
    BOOL                fViewChanged = FALSE;
    CLine *             pli;
    UINT                uJustified;


    Assert(pPF);
    Assert(pCF);
    Assert(pci);
    Assert(cch >= 0);

    if (!pPF || !pCF || !pci || cch < 0)
        return FALSE;

    // Bail out if there is anything special in the format
    // BUGBUG (MohanB) Should be able to handle line height. For now though,
    // do this to keep RoboVision happy (with lheight.htm).
    if (    pCF->IsTextTransformNeeded()
        ||  !pCF->_cuvLineHeight.IsNullOrEnum()
        ||  !pCF->_cuvLetterSpacing.IsNullOrEnum())
    {
        goto HardCase;
    }


    pccs = fc().GetCcs(pci->_hdc, pci, pCF);
    if (!pccs)
        return FALSE;

    lWidth = 0;

    if (cch)
    {
        if (chPassword)
        {
            if (!pccs->Include(chPassword, lCharWidth) )
            {
                Assert(0 && "Char not in font!");
            }
            lWidth = cch * lCharWidth;
        }
        else
        {
            CTxtPtr     tp(GetMarkup(), pFlowLayout->GetContentFirstCp());
            LONG        cchValid;
            LONG        cchRemaining = cch;

            for (;;)
            {
                const TCHAR * pchText = tp.GetPch(cchValid);
                LONG i = min(cchRemaining, cchValid);

                while (i--)
                {
                    const TCHAR ch = *pchText++;

                    // Bail out if not a simple ASCII char

                    if (!InRange( ch, 32, 127 ))
                        goto HardCase;

                    if (!pccs->Include(ch, lCharWidth))
                    {
                        Assert(0 && "Char not in font!");
                    }
                    lWidth += lCharWidth;
                }

                if (cchRemaining <= cchValid)
                {
                    break;
                }
                else
                {
                    cchRemaining -= cchValid;
                    tp.AdvanceCp(cchValid);
                }
            }
        }
    }

    GetPadding(pci, lPadding, pci->_smMode == SIZEMODE_MMWIDTH);
    FlushRecalc();

    pli = Add(1, NULL);
    if (!pli)
        return FALSE;

    pli->_cch               = cch;
    pli->_xWidth            = lWidth;
    pli->_yHeight           = pccs->_yHeight;
    pli->_yTxtDescent       = pccs->_yDescent;
    pli->_yDescent          = pccs->_yDescent;
    pli->_xLineOverhang     = pccs->_xOverhang;
    pli->_yExtent           = pccs->_yHeight;
    pli->_yBeforeSpace      = lPadding[PADDING_TOP];
    pli->_xLeft             = lPadding[PADDING_LEFT];
    pli->_xRight            = lPadding[PADDING_RIGHT];
    pli->_xLeftMargin       = 0;
    pli->_xRightMargin      = 0;
    pli->_fForceNewLine     = TRUE;
    pli->_fFirstInPara      = TRUE;
    pli->_fFirstFragInLine  = TRUE;
    pli->_fCanBlastToScreen = !chPassword;

    _yBottomMargin  = lPadding[PADDING_BOTTOM];
    _dcpCalcMax     = cch;

    yHeight          = pli->_yHeight;
    xWidth           = pli->_xLeft + pli->_xWidth + pli->_xLineOverhang + pli->_xRight;

    Assert(pci->_smMode != SIZEMODE_MINWIDTH);

    xShift = ComputeLineShift(
                        (htmlAlign)pPF->GetBlockAlign(TRUE),
                        _fRTL,
                        pPF->HasRTL(TRUE),
                        pci->_smMode == SIZEMODE_MMWIDTH,
                        _xWidthView,
                        xWidth + GetCaret(),
                        &uJustified);

    pli->_fJustified = uJustified;

    if(_fRTL)
        pli->_xLeft += xShift;
    else
        pli->_xRight += xShift;

    xWidth      += xShift;

    pli->_xLineWidth = max(xWidth, _xWidthView);

    if(yHeight != _yHeight || xWidth != _xWidth)
        fViewChanged = TRUE;

    _yCalcMax       =
    _yHeightMax     =
    _yHeight        = yHeight;
    _xWidth         = xWidth;
    _fRecalcDone    = TRUE;
    _fMinMaxCalced  = TRUE;
    _xMinWidth      =
    _xMaxWidth      = _xWidth + GetCaret();

    if (pci->_smMode == SIZEMODE_NATURAL && fViewChanged)
    {
        pFlowLayout->SizeContentDispNode(CSize(GetMaxWidth(), GetHeight()));

        // If our contents affects our size, ask our parent to initiate a re-size
        if (    pFlowLayout->GetAutoSize()
            ||  pFlowLayout->_fContentsAffectSize)
        {
            pFlowLayout->ElementOwner()->ResizeElement();
        }
    }

    pccs->Release();

    return TRUE;

HardCase:

    // Just do it the hard way
    return RecalcLines(pci);
}

/*
 *  CDisplay::RecalcLines()
 *
 *  @mfunc
 *      Recalc all line breaks.
 *      This method does a lazy calc after the last visible line
 *      except for a bottomless control
 *
 *  @rdesc
 *      TRUE if success
 */

BOOL CDisplay::RecalcLines(CCalcInfo * pci)
{
#ifdef SWITCHES_ENABLED
    if (IsSwitchNoRecalcLines())
        return FALSE;
#endif

    SwitchesBegTimer(SWITCHES_TIMER_RECALCLINES);

    BOOL fRet;

    if (GetFlowLayout()->ElementOwner()->IsInMarkup())
    {
        if (        pci->_fTableCalcInfo
                    && ((CTableCalcInfo *) pci)->_pme
          WHEN_DBG( && !IsTagEnabled(tagTableCalcDontReuseMeasurer) ) )
        {
            // Save calcinfo's measurer.
            CTableCalcInfo * ptci = (CTableCalcInfo *) pci;
            CLSMeasurer * pme = ptci->_pme;

            // Reinitialize the measurer.
            pme->Reinit(this, ptci);

            // Make sure noone else uses this measurer.
            ptci->_pme = NULL;

            // Do actual RecalcLines work with this measurer.
            fRet = RecalcLinesWithMeasurer(ptci, pme);

            // Restore TableCalcInfo measurer.
            ptci->_pme = pme;
        }
        else
        {
            // Cook up measurer on the stack.
            CLSMeasurer me(this, pci);

            fRet = RecalcLinesWithMeasurer(pci, &me);
        }
    }
    else
        fRet = FALSE;

    SwitchesEndTimer(SWITCHES_TIMER_RECALCLINES);

    return fRet;
}


/*
 *  CDisplay::RecalcLines()
 *
 *  @mfunc
 *      Recalc all line breaks.
 *      This method does a lazy calc after the last visible line
 *      except for a bottomless control
 *
 *  @rdesc
 *      TRUE if success
 */

BOOL CDisplay::RecalcLinesWithMeasurer(CCalcInfo * pci, CLSMeasurer * pme)
{
#ifdef SWITCHES_ENABLED
    if (IsSwitchNoRecalcLines())
        return FALSE;
#endif

    CFlowLayout * pFlowLayout = GetFlowLayout();
    CElement    * pElementFL  = pFlowLayout->ElementOwner();
    CElement::CLock     Lock(pElementFL, CElement::ELEMENTLOCK_RECALC);

    CRecalcLinePtr  RecalcLinePtr(this, pci);
    CLine *         pliNew = NULL;
    int             iLine;
    UINT            uiMode;
    UINT            uiFlags;
    BOOL            fDone                   = TRUE;
    BOOL            fFirstInPara            = TRUE;
    BOOL            fWaitingForFirstVisible = TRUE;
    LONG            cpFirstVisible          = 0;
    BOOL            fAllowBackgroundRecalc;
    LONG            cliWait         = g_cExtraBeforeLazy; // Extra lines before we use background recalc.
    LONG            yHeight         = 0;
    LONG            yAlignDescent   = 0;
    LONG            yHeightView     = GetViewHeight();
    LONG            xMinLineWidth;
    LONG *          pxMinLineWidth  = NULL;
    LONG            xMaxLineWidth   = 0;
    LONG            yHeightDisplay  = 0;     // to keep track of line with top negative margins
    LONG            yHeightMax;
    long            lPadding[PADDING_MAX];
    BOOL            fViewChanged = FALSE;

    // we should not be measuring hidden stuff.
    Assert(!pElementFL->IsDisplayNone() || pElementFL->Tag() == ETAG_BODY);

    if (pFlowLayout->FExternalLayout())
        return FALSE;

    if (pElementFL->Tag() == ETAG_ROOT)
        return TRUE;

    if (!pme->_pLS)
        return FALSE;

    if (!pElementFL->IsInMarkup())
    {
        return TRUE;
    }

    GetPadding(pci, lPadding, pci->_smMode == SIZEMODE_MMWIDTH);

    _yBottomMargin = lPadding[PADDING_BOTTOM];

    // Set up the CCalcInfo to the correct mode and parent size
    if (pci->_smMode == SIZEMODE_SET)
    {
        pci->_smMode = SIZEMODE_NATURAL;
    }

    // Determine the mode
    uiMode = MEASURE_BREAKATWORD;

    Assert (pci);

    switch(pci->_smMode)
    {
    case SIZEMODE_MMWIDTH:
        uiMode = MEASURE_BREAKATWORD | MEASURE_MAXWIDTH;
        _xMinWidth = 0;
        pxMinLineWidth = &xMinLineWidth;
        break;
    case SIZEMODE_MINWIDTH:
        uiMode = MEASURE_BREAKATWORD | MEASURE_MINWIDTH;
        pme->AdvanceToNextAlignedObject();
        break;
    }

    uiFlags = uiMode;

    // Determine if background recalc is allowed
    // (And if it is, do not calc past the visible portion)
    fAllowBackgroundRecalc = AllowBackgroundRecalc(pci);

    if (fAllowBackgroundRecalc)
        cpFirstVisible = GetFirstVisibleCp();

    // Flush all old lines
    FlushRecalc();

    if ( GetWordWrap() && GetWrapLongLines() )
    {
        uiFlags |= MEASURE_BREAKLONGLINES;
    }

    pme->_pDispNodePrev = NULL;

    if (fAllowBackgroundRecalc)
    {
        if (!SUCCEEDED(EnsureBgRecalcInfo()))
        {
            fAllowBackgroundRecalc = FALSE;
            AssertSz(FALSE, "CDisplay::RecalcLinesWithMeasurer - Could not create BgRecalcInfo");
        }
    }

    RecalcLinePtr.Init((CLineArray *)this, 0, NULL);

    // recalculate margins
    RecalcLinePtr.RecalcMargins(0, 0, yHeight, 0);

    // The following loop generates new lines
    do
    {
        // Add one new line
        pliNew = RecalcLinePtr.AddLine();
        if (!pliNew)
        {
            Assert(FALSE);
            goto err;
        }

        uiFlags &= ~MEASURE_FIRSTINPARA;
        uiFlags |= (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

        PerfDbgLog1(tagRecalc, this, "Measuring new line from cp = %d", pme->GetCp());

        iLine = LineCount() - 1;

        if (long(pme->GetCp()) == pme->GetLastCp())
        {
            uiFlags |= MEASURE_EMPTYLASTLINE;
        }

        if(!RecalcLinePtr.MeasureLine(*pme, uiFlags,
                                        &iLine, &yHeight, &yAlignDescent,
                                        pxMinLineWidth, &xMaxLineWidth))
        {
            goto err;
        }

        // fix for bug 16519 (srinib)
        // Keep track of the line that contributes to the max height, ex: if the
        // last lines top margin is negative.
        if(yHeightDisplay < yHeight)
            yHeightDisplay = yHeight;

        pliNew = iLine >=0 ? RecalcLinePtr[iLine] : NULL;
        fFirstInPara = (iLine >= 0)
                            ? pliNew->IsNextLineFirstInPara()
                            : TRUE;

        if (pliNew)
            NoteMost(pliNew);

        if (fAllowBackgroundRecalc)
        {
            Assert(HasBgRecalcInfo());

            if (fWaitingForFirstVisible)
            {
                if ((LONG)pme->GetCp() > cpFirstVisible)
                {
                    BgRecalcInfo()->_yWait  = yHeight + yHeightView;
                    fWaitingForFirstVisible = FALSE;
                }
            }

            else if (yHeightDisplay > YWait())
            {
                fDone = FALSE;
                break;
            }
        }

        // When doing a full min pass, just calculate the islands around aligned
        // objects. Note that this still calc's the lines around right aligned
        // images, which isn't really necessary because they are willing to
        // overlap text. Fix it next time.
        if ((uiMode & MEASURE_MINWIDTH) &&
            !RecalcLinePtr._marginInfo._xLeftMargin &&
            !RecalcLinePtr._marginInfo._xRightMargin )
            pme->AdvanceToNextAlignedObject();

    } while( pme->GetCp() < pme->GetLastCp());

    _fRecalcDone = fDone;

    if(fDone && pliNew)
    {
        if (GetFlowLayout()->IsEditable() && // if we are in design mode
                (pliNew->_fHasEOP || pliNew->_fHasBreak || pliNew->_fSingleSite))
        {
            Assert(pliNew == RecalcLinePtr[iLine]);
            CreateEmptyLine(pme, &RecalcLinePtr, &yHeight, pliNew->_fHasEOP );

            // Creating the line could have reallocated the memory to which pliNew points,
            // so we need to refresh the pointer just in case.

            pliNew = RecalcLinePtr[iLine];
        }

        // fix for bug 16519
        // Keep track of the line that contributes to the max height, ex: if the
        // last lines top margin is negative.
        if(yHeightDisplay < yHeight)
            yHeightDisplay = yHeight;

        // In table cells, Netscape actually adds the interparagraph spacing
        // for any closed tags to the height of the table cell.
        // BUGBUG: This actually applies the spacing to all nested text containers, not just
        //         table cells. Is this correct? (brendand).
        // It's not correct to add the spacing at all, but given that Netscape
        // has done so, and that they will probably do so for floating block
        // elements. Arye.
        else
        {
            int iLineLast = iLine;

            // we need to force scrolling when bottom-margin is set on the last block tag
            // in body. (20400)

            while (iLineLast-- > 0 &&   // Lines available
                   !pliNew->_fForceNewLine &&   // Just a dummy line.
                   pliNew->_yHeight == 0 && pliNew->_yExtent == 0)    // Line contains no text.
                --pliNew;
            _yBottomMargin += RecalcLinePtr.NetscapeBottomMargin(pme, pliNew);
        }
    }

    yHeightMax = max(yHeightDisplay, yAlignDescent);

    if(yHeightDisplay != _yHeight)
    {
        _yHeight = yHeightDisplay;
        fViewChanged = TRUE;
    }

    if(yHeightMax != _yHeightMax)
    {
        _yHeightMax = yHeightMax;
        fViewChanged = TRUE;
    }

    if (!(uiMode & MEASURE_MAXWIDTH))
    {
        xMaxLineWidth = CalcDisplayWidth();
    }

    if(xMaxLineWidth != _xWidth)
    {
        _xWidth = xMaxLineWidth;
        fViewChanged = TRUE;
    }

    _dcpCalcMax = (LONG)pme->GetCp() - GetFirstCp();
    _yCalcMax   = yHeightDisplay;

    if(pxMinLineWidth)
    {
        pFlowLayout->MarkHasAlignedLayouts(RecalcLinePtr._cLeftAlignedLayouts ||
                                           RecalcLinePtr._cRightAlignedLayouts > 1);
    }

    // If the view or size changed, re-size or update scrollbars as appropriate
    // BUGBUG: Collapse both RecalcLines into one and remove knowledge of
    //         CSite::SIZEMODE_xxxx (it should be based entirely on the
    //         passed MEASURE_xxxx flags set in CalcSize) (brendand)
    if (pci->_smMode == SIZEMODE_NATURAL && fViewChanged)
    {
        pFlowLayout->SizeContentDispNode(CSize(GetMaxWidth(), GetHeight()));

        // If our contents affects our size, ask our parent to initiate a re-size
        if (    pFlowLayout->GetAutoSize()
            ||  pFlowLayout->_fContentsAffectSize)
        {
            pFlowLayout->ElementOwner()->ResizeElement();
        }
    }

    if (    pci->_smMode == SIZEMODE_NATURAL
        ||  pci->_smMode == SIZEMODE_SET
        ||  pci->_smMode == SIZEMODE_FULLSIZE)
    {
        if (pFlowLayout->_fContainsRelative)
            UpdateRelDispNodeCache(NULL);

        AdjustDispNodes(pme->_pDispNodePrev, NULL);

        pFlowLayout->NotifyMeasuredRange(GetFlowLayoutElement()->GetFirstCp(), GetMaxCpCalced());
    }

    PerfDbgLog1(tagRecalc, this, "CDisplay::RecalcLine() - Done. Recalced down to line #%d", LineCount());

    if(!fDone)                      // if not done, do rest in background
    {
        StartBackgroundRecalc(pci->_grfLayout);
    }
    else if (fAllowBackgroundRecalc)
    {
        Assert((!CanHaveBgRecalcInfo() || BgRecalcInfo()) && "Should have a BgRecalcInfo");
        if (HasBgRecalcInfo())
        {
            CBgRecalcInfo * pBgRecalcInfo = BgRecalcInfo();
            pBgRecalcInfo->_yWait = -1;
            pBgRecalcInfo->_cpWait = -1;
        }
#if DBG==1
        if (!(uiMode & MEASURE_MINWIDTH))
            CheckLineArray();
#endif
        _fLineRecalcErr = FALSE;

        RecalcMost();
    }

    // cache min/max only if there are no sites inside !
    if (pxMinLineWidth && !pFlowLayout->ContainsChildLayout())
    {
        _xMaxWidth      = _xWidth + GetCaret();
        _fMinMaxCalced  = TRUE;
    }

    // adjust for caret only when are calculating for MMWIDTH or MINWIDTH
    if (pci->_smMode == SIZEMODE_MMWIDTH || pci->_smMode == SIZEMODE_MINWIDTH)
    {
        if (pci->_smMode == SIZEMODE_MMWIDTH)
            _xMinWidth = max(_xMinWidth, RecalcLinePtr._xMaxRightAlign);
        _xMinWidth      += GetCaret();  // adjust for caret only when are calculating for MMWIDTH
    }

    // NETSCAPE: If there is no text or special characters, treat the site as
    //           empty. For example, with an empty BLOCKQUOTE tag, _xWidth will
    //           not be zero while the site should be considered empty.
    if(_fNoContent)
    {
        _xWidth =
        _xMinWidth = lPadding[PADDING_LEFT] + lPadding[PADDING_RIGHT];
    }

    return TRUE;

err:
    TraceTag((tagError, "CDisplay::RecalcLines() failed"));

    if(!_fLineRecalcErr)
    {
        _dcpCalcMax   = pme->GetCp() - GetFirstCp();
        _yCalcMax = yHeightDisplay;
    }

    return FALSE;
}

/*
 *  CDisplay::RecalcLines(cpFirst, cchOld, cchNew, fBackground, pled)
 *
 *  @mfunc
 *      Recompute line breaks after text modification
 *
 *  @rdesc
 *      TRUE if success
 */


BOOL CDisplay::RecalcLines (
    CCalcInfo * pci,
    LONG cpFirst,               //@parm Where change happened
    LONG cchOld,                //@parm Count of chars deleted
    LONG cchNew,                //@parm Count of chars added
    BOOL fBackground,           //@parm This method called as background process
    CLed *pled,                 //@parm Records & returns impact of edit on lines (can be NULL)
    BOOL fHack)                 //@parm This comes from WaitForRecalc ... don't mess with BgRecalcInfo
{
#ifdef SWITCHES_ENABLED
    if (IsSwitchNoRecalcLines())
        return FALSE;
#endif

    CSaveCalcInfo       sci(pci);
    CElement::CLock     Lock(GetFlowLayoutElement(), CElement::ELEMENTLOCK_RECALC);

    CFlowLayout * pFlowLayout = GetFlowLayout();
    CElement    * pElementFL  = pFlowLayout->ElementOwner();
    CElement    * pElementContent  = pFlowLayout->ElementContent();

    LONG        cchEdit;
    LONG        cchSkip = 0;
    INT         cliBackedUp = 0;
    LONG        cliWait = g_cExtraBeforeLazy;
    BOOL        fDone = TRUE;
    BOOL        fFirstInPara = TRUE;
    BOOL        fAllowBackgroundRecalc;
    CLed        led;
    LONG        lT = 0;         // long Temporary
    CLine *     pliNew;
    CLinePtr    rpOld(this);
    CLine *     pliCur;
    LONG        xWidth;
    LONG        yHeight, yAlignDescent=0;
    LONG        cpLast = GetLastCp();
    long        cpLayoutFirst = GetFirstCp();
    UINT        uiFlags = MEASURE_BREAKATWORD;
    BOOL        fReplaceResult = TRUE;
    BOOL        fTryForMatch = TRUE;
    BOOL        fNeedToBackUp = TRUE;
    int         diNonBlankLine = -1;
    int         iOldLine;
    int         iNewLine;
    int         iMinimumLinesToRecalc = 4;   // Guarantee some progress
    LONG        yHeightDisplay = 0;
    LONG        yHeightMax;
    CLineArray  aryNewLines;
    long        lPadding[PADDING_MAX];

#if DBG==1
    LONG        cp;
#endif

    if (pFlowLayout->FExternalLayout())
        return FALSE;

    if (!pFlowLayout->ElementOwner()->IsInMarkup())
    {
        return TRUE;
    }

    SwitchesBegTimer(SWITCHES_TIMER_RECALCLINES);

    // we should not be measuring hidden stuff.
    Assert(!pElementFL->IsDisplayNone() || pElementFL->Tag() == ETAG_BODY);

    // If no lines, this routine should not be called
    // Call the other RecalcLines above instead !
    Assert(LineCount() > 0);

    // Set up the CCalcInfo to the correct mode and parent size
    if (pci->_smMode == SIZEMODE_SET)
    {
        pci->_smMode = SIZEMODE_NATURAL;
    }

    // Init measurer at cpFirst
    CLSMeasurer     me(this, cpFirst, pci);

    CRecalcLinePtr  RecalcLinePtr(this, pci);

    if (!me._pLS)
        goto err;

    if (!pled)
        pled = &led;

    GetPadding(pci, lPadding);
    _yBottomMargin = lPadding[PADDING_BOTTOM];

#if DBG==1
    if(cpFirst > GetMaxCpCalced())
        TraceTag((tagError, "cpFirst %ld, CalcMax %ld", cpFirst, GetMaxCpCalced()));

    AssertSz(cpFirst <= GetMaxCpCalced(),
        "CDisplay::RecalcLines Caller didn't setup RecalcLines()");
#endif

    if (    GetWordWrap()
        &&  GetWrapLongLines())
    {
        uiFlags |= MEASURE_BREAKLONGLINES;
    }

    // Determine if background recalc is allowed
    fAllowBackgroundRecalc = AllowBackgroundRecalc(pci, fBackground);

    Assert(!fBackground || HasBgRecalcInfo());
    if (    !fBackground
        &&  !fHack
        &&  fAllowBackgroundRecalc)
    {
        if (SUCCEEDED(EnsureBgRecalcInfo()))
        {
            BgRecalcInfo()->_yWait  = pFlowLayout->GetClientBottom();
            BgRecalcInfo()->_cpWait = -1;
        }
        else
        {
            fAllowBackgroundRecalc = FALSE;
            AssertSz(FALSE, "CDisplay::RecalcLines - Could not create BgRecalcInfo");
        }
    }

    // Init line pointer on old CLineArray
    rpOld.RpSetCp(cpFirst, FALSE, FALSE);

    pliCur = rpOld.CurLine();

    while(pliCur->IsClear() ||
          (pliCur->IsFrame() && !pliCur->_fFrameBeforeText))
    {
        if(!rpOld.AdjustBackward())
            break;

        pliCur = rpOld.CurLine();
    }

    // If the current line has single site
    if(pliCur->_fSingleSite)
    {
        // If we are in the middle of the current line (some thing changed
        // in a table cell or 1d-div, or marquee)
        if(rpOld.RpGetIch() && rpOld.GetCchRemaining() != 0)
        {
            // we dont need to back up
            if(rpOld > 0 && rpOld[-1]._fForceNewLine)
            {
                fNeedToBackUp = FALSE;
                cchSkip = rpOld.RpGetIch();
            }
        }
    }

    iOldLine = rpOld;

    if(fNeedToBackUp)
    {
        if(!rpOld->IsBlankLine())
        {
            cchSkip = rpOld.RpGetIch();
            if(cchSkip)
                rpOld.RpAdvanceCp(-cchSkip);
        }

        // find the first previous non blank line, if the first non blank line
        // is a single site line or a line with EOP, we don't need to skip.
        while(rpOld + diNonBlankLine >= 0 &&
                rpOld[diNonBlankLine].IsBlankLine())
            diNonBlankLine--;

        // (srinib) - if the non text line does not have a line break or EOP, or
        // a force newline or is single site line and the cp at which the characters changed is
        // ambiguous, we need to back up.

        // if the single site line does not force a new line, we do need to
        // back up. (bug #44346)
        if(rpOld + diNonBlankLine >= 0)
        {
            pliCur = &rpOld[diNonBlankLine];
            if(!pliCur->_fSingleSite || !pliCur->_fForceNewLine || cchSkip == 0)
            {
                // Skip past all the non text lines
                while(diNonBlankLine++ < 0)
                    rpOld.AdjustBackward();

                // Skip to the begining of the line
                cchSkip += rpOld.RpGetIch();
                rpOld.RpAdvanceCp(-rpOld.RpGetIch());

                // we want to skip all the lines that do not force a
                // newline, so find the last line that does not force a new line.
                long diTmpLine = -1;
                long cchSkipTemp = 0;
                while(rpOld + diTmpLine >=0 &&
                        ((pliCur = &rpOld[diTmpLine]) != 0) &&
                        !pliCur->_fForceNewLine)
                {
                    if(!pliCur->IsBlankLine())
                        cchSkipTemp += pliCur->_cch;
                    diTmpLine--;
                }
                if(cchSkipTemp)
                {
                    cchSkip += cchSkipTemp;
                    rpOld.RpAdvanceCp(-cchSkipTemp);
                }
            }
        }

        // back up further if the previous lines are either frames inserted
        // by aligned sites at the beginning of the line or auto clear lines
        while(rpOld && ((pliCur = &rpOld[-1]) != 0) &&
              // frame line before the actual text or
              (pliCur->_fClearBefore ||
               (pliCur->IsFrame() && pliCur->_fFrameBeforeText)) &&
              rpOld.AdjustBackward());
    }

    cliBackedUp = iOldLine - rpOld;

    // Need to get a running start.
    me.Advance(-cchSkip);
    RecalcLinePtr.InitPrevAfter(&me._fLastWasBreak, rpOld);

    cchEdit = cchNew + cchSkip;         // Number of chars affected by edit

    Assert(cchEdit <= GetLastCp() - long(me.GetCp()) );
    if (cchEdit > GetLastCp() - long(me.GetCp()))
    {
        // BUGBUG (istvanc) this of course shouldn't happen !!!!!!!!
        cchEdit = GetLastCp() - me.GetCp();
    }

    // Determine whether we're on first line of paragraph
    if(rpOld > 0)
    {
        int iLastNonFrame = -1;

        // frames do not hold any info, so go back past all frames.
        while(rpOld + iLastNonFrame && rpOld[iLastNonFrame].IsFrame())
            iLastNonFrame--;
        if(rpOld + iLastNonFrame >= 0)
        {
            CLine *pliNew = &rpOld[iLastNonFrame];

            fFirstInPara = pliNew->IsNextLineFirstInPara();
        }
        else
        {
            // we are at the Beginning of a document
            fFirstInPara = TRUE;
        }
    }

    yHeight = YposFromLine(rpOld, pci);
    yHeightDisplay = yHeight;
    yAlignDescent = 0;

    // Update first-affected and pre-edit-match lines in pled
    pled->_iliFirst = rpOld;
    pled->_cpFirst  = pled->_cpMatchOld = me.GetCp();
    pled->_yFirst   = pled->_yMatchOld  = yHeight;
    AssertSz(pled->_yFirst >= 0, "CDisplay::RecalcLines _yFirst < 0");

    PerfDbgLog2(tagRecalc, this, "Start recalcing from line #%d, cp=%d",
              pled->_iliFirst, pled->_cpFirst);

    // In case of error, set both maxes to where we are now
    _yCalcMax   = yHeight;
    _dcpCalcMax = me.GetCp() - cpLayoutFirst;

    if (pled->_cpFirst == 16 && pled->_iliFirst == 1 && pled->_yFirst == 31)
    {
        AssertSz(FALSE, "Stop!!!");
    }
    //
    // find the display node the corresponds to cpStart
    //
    me._pDispNodePrev = pled->_iliFirst
                            ? GetPreviousDispNode(pled->_cpFirst)
                            : 0;

    // If we are past the requested area to recalc and background recalc is
    // allowed, then just go directly to background recalc.
    if (    fAllowBackgroundRecalc
        &&  yHeight > YWait()
        &&  (LONG) me.GetCp() > CPWait())
    {
        // Remove all old lines from here on
        rpOld.RemoveRel(-1, AF_KEEPMEM);

        // Start up the background recalc
        StartBackgroundRecalc(pci->_grfLayout);
        pled->SetNoMatch();

        // Update the relative line cache.
        if (pFlowLayout->_fContainsRelative)
            UpdateRelDispNodeCache(NULL);

        AdjustDispNodes(me._pDispNodePrev, pled);

        goto cleanup;
    }

    aryNewLines.Clear(AF_KEEPMEM);
    pliNew = NULL;

    iOldLine = rpOld;
    RecalcLinePtr.Init((CLineArray *)this, iOldLine, &aryNewLines);

    // recalculate margins
    RecalcLinePtr.RecalcMargins(iOldLine, iOldLine, yHeight, 0);

    Assert ( cchEdit <= GetLastCp() - long(me.GetCp()) );

    if(iOldLine)
        RecalcLinePtr.SetupMeasurerForBeforeSpace(&me);

    // The following loop generates new lines for each line we backed
    // up over and for lines directly affected by edit
    while(cchEdit > 0)
    {
        LONG iTLine;
        LONG cpTemp;
        pliNew = RecalcLinePtr.AddLine();       // Add one new line
        if (!pliNew)
        {
            Assert(FALSE);
            goto err;
        }

        // Stuff text into new line
        uiFlags &= ~MEASURE_FIRSTINPARA;
        uiFlags |= (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

        PerfDbgLog1(tagRecalc, this, "Measuring new line from cp = %d", me.GetCp());

        // measure can add lines for aligned sites
        iNewLine = iOldLine + aryNewLines.Count() - 1;

        // save the index of the new line to added
        iTLine = iNewLine;

#if DBG==1
        {
            // Just so we can see the damn text.
            const TCHAR *pch;
            long cchInStory = (long)GetFlowLayout()->GetContentTextLength();
            long cp = (long)me.GetCp();
            long cchRemaining =  cchInStory - cp;
            CTxtPtr tp(GetMarkup(), cp);
            pch = tp.GetPch(cchRemaining);
#endif

        cpTemp = me.GetCp();

        if (cpTemp == pFlowLayout->GetContentLastCp() - 1)
        {
            uiFlags |= MEASURE_EMPTYLASTLINE;
        }

        if (!RecalcLinePtr.MeasureLine(me, uiFlags,
                                       &iNewLine, &yHeight, &yAlignDescent,
                                       NULL, NULL))
        {
            goto err;
        }

        pliNew = iNewLine >=0 ? RecalcLinePtr[iNewLine] : NULL;

        fFirstInPara = (iNewLine >= 0)
                            ? pliNew->IsNextLineFirstInPara()
                            : TRUE;

#if DBG==1
        }
#endif
        // fix for bug 16519 (srinib)
        // Keep track of the line that contributes to the max height, ex: if the
        // last lines top margin is negative.
        if(yHeightDisplay < yHeight)
            yHeightDisplay = yHeight;

        if (iNewLine >= 0)
            NoteMost(RecalcLinePtr[iNewLine]);

        // If we have added any clear lines, iNewLine points to a textline
        // which could be < iTLine
        if(iNewLine >= iTLine)
            cchEdit -= me.GetCp() - cpTemp;

        if(cchEdit <= 0)
            break;

        --iMinimumLinesToRecalc;
        if(iMinimumLinesToRecalc < 0 &&
           fBackground && (GetTickCount() >= BgndTickMax())) // GetQueueStatus(QS_ALLEVENTS))
        {
            fDone = FALSE;                      // took too long, stop for now
            goto no_match;
        }

        if (    fAllowBackgroundRecalc
            &&  yHeight > YWait()
            &&  (LONG) me.GetCp() > CPWait()
            &&  cliWait-- <= 0)
        {
            // Not really done, just past region we're waiting for
            // so let background recalc take it from here
            fDone = FALSE;
            goto no_match;
        }
    }                                           // while(cchEdit > 0) { }

    PerfDbgLog1(tagRecalc, this, "Done recalcing edited text. Created %d new lines", aryNewLines.Count());

    // Edit lines have been exhausted.  Continue breaking lines,
    // but try to match new & old breaks

    while( (LONG) me.GetCp() < cpLast )
    {
        // Assume there are no matches to try for
        BOOL frpOldValid = FALSE;
        BOOL fChangedLineSpacing = FALSE;

        // If we run out of runs, then there is not match possible. Therefore,
        // we only try for a match as long as we have runs.
        if (fTryForMatch)
        {
            // We are trying for a match so assume that there
            // is a match after all
            frpOldValid = TRUE;

            // Look for match in old line break CArray
            lT = me.GetCp() - cchNew + cchOld;
            while (rpOld.IsValid() && pled->_cpMatchOld < lT)
            {
                if(rpOld->_fForceNewLine)
                    pled->_yMatchOld += rpOld->_yHeight;
                pled->_cpMatchOld += rpOld->_cch;

                BOOL fDone=FALSE;
                BOOL fSuccess = TRUE;
                while (!fDone)
                {
                    if( !rpOld.NextLine(FALSE,FALSE) )     // NextRun()
                    {
                        // No more line array entries so we can give up on
                        // trying to match for good.
                        fTryForMatch = FALSE;
                        frpOldValid = FALSE;
                        fDone = TRUE;
                        fSuccess = FALSE;
                    }
                    if (!rpOld->IsFrame() ||
                            (rpOld->IsFrame() && rpOld->_fFrameBeforeText))
                        fDone = TRUE;
                }
                if (!fSuccess)
                    break;
            }
        }

        // skip frame in old line array
        if (rpOld->IsFrame() && !rpOld->_fFrameBeforeText)
        {
            BOOL fDone=FALSE;
            while (!fDone)
            {
                if (!rpOld.NextLine(FALSE,FALSE))
                {
                    // No more line array entries so we can give up on
                    // trying to match for good.
                    fTryForMatch = FALSE;
                    frpOldValid = FALSE;
                    fDone = TRUE;
                }
                if (!rpOld->IsFrame())
                    fDone = TRUE;
            }
        }

        pliNew = aryNewLines.Count() > 0 ? aryNewLines.Elem(aryNewLines.Count() - 1) : NULL;

        // If perfect match, stop
        if(frpOldValid && rpOld.IsValid() && pled->_cpMatchOld == lT
            && rpOld->_cch != 0 && (pliNew && pliNew->_fForceNewLine &&
            yHeight - pliNew->_yHeight > RecalcLinePtr._marginInfo._yBottomLeftMargin &&
            yHeight - pliNew->_yHeight > RecalcLinePtr._marginInfo._yBottomRightMargin))
        {
            BOOL fFoundMatch = TRUE;

            if(rpOld->_xLeftMargin || rpOld->_xRightMargin)
            {
                // we might have found a match based on cp, but if an
                // aligned site is resized to a smaller size. There might
                // be lines that used to be aligned to the aligned site
                // which are not longer aligned to it. If so, recalc those lines.
                RecalcLinePtr.RecalcMargins(iOldLine, iOldLine + aryNewLines.Count(), yHeight,
                                                rpOld->_yBeforeSpace);
                if(RecalcLinePtr._marginInfo._xLeftMargin != rpOld->_xLeftMargin ||
                    (rpOld->_xLeftMargin + rpOld->_xLineWidth +
                        RecalcLinePtr._marginInfo._xRightMargin) < pFlowLayout->GetMaxLineWidth())
                {
                    fFoundMatch = FALSE;
                }
            }

            // There are cases where we've matched characters, but want to continue
            // to recalc anyways. This requires us to instantiate a new measurer.
            if (fFoundMatch && !fChangedLineSpacing && rpOld < LineCount() &&
                (rpOld->_fFirstInPara || pliNew->_fHasEOP))
            {
                BOOL                  fInner;
                const CParaFormat *   pPF;
                CLSMeasurer           tme(this, me.GetCp(), pci);

                if (!tme._pLS)
                    goto err;

                if(pliNew->_fHasEOP)
                {
                    rpOld->_fFirstInPara = TRUE;
                }
                else
                {
                    rpOld->_fFirstInPara = FALSE;
                    rpOld->_fHasBulletOrNum = FALSE;
                    rpOld->_fNewList = FALSE;
                }

                // If we've got a <DL COMPACT> <DT> line. For now just check to see if
                // we're under the DL.

                pPF = tme.CurrBranch()->GetParaFormat();

                fInner = SameScope(tme.CurrBranch(), pElementContent);

                if (pPF->HasCompactDL(fInner))
                {
                    // If the line is a DT and it's the end of the paragraph, and the COMPACT
                    // attribute is set.
                    fChangedLineSpacing = TRUE;
                }

                // Check to see if the line height is the same. This is necessary
                // because there are a number of block operations that can
                // change the prespacing of the next line.
                else
                {
                    // We'd better not be looking at a frame here.
                    Assert (!rpOld->IsFrame());

                    // Make it possible to check the line space of the
                    // current line.
                    tme.InitLineSpace (&me, rpOld);

                    RecalcLinePtr.CalcInterParaSpace (&tme,
                            pled->_iliFirst + aryNewLines.Count() - 1, FALSE);

                    if (rpOld->_yBeforeSpace != tme._li._yBeforeSpace ||
                        rpOld->_fHasBulletOrNum != tme._li._fHasBulletOrNum ||
                        rpOld->_fNewList != tme._li._fNewList)
                    {
                        rpOld->_fHasBulletOrNum = tme._li._fHasBulletOrNum;
                        rpOld->_fNewList = tme._li._fNewList;

                        fChangedLineSpacing = TRUE;
                    }
                }
            }
            else
                fChangedLineSpacing = FALSE;


            if (fFoundMatch && !fChangedLineSpacing)
            {
                PerfDbgLog1(tagRecalc, this, "Found match with old line #%d", rpOld.GetLineIndex());
                pled->_iliMatchOld = rpOld;

                // Replace old lines by new ones
                lT = rpOld - pled->_iliFirst;
                rpOld = pled->_iliFirst;

                fReplaceResult = rpOld.Replace(lT, &aryNewLines);
                if (!fReplaceResult)
                {
                    Assert(FALSE);
                    goto err;
                }

                frpOldValid =
                    rpOld.SetRun( rpOld.GetIRun() + aryNewLines.Count(), 0 );

                aryNewLines.Clear(AF_DELETEMEM);           // Clear aux array
                iOldLine = rpOld;

                // Remember information about match after editing
                Assert((cp = rpOld.GetCp() + cpLayoutFirst) == (LONG) me.GetCp());
                pled->_yMatchNew = yHeight;
                pled->_iliMatchNew = rpOld;
                pled->_cpMatchNew = me.GetCp();

                _dcpCalcMax = me.GetCp() - cpLayoutFirst;

                // Compute height and cp after all matches
                if( frpOldValid && rpOld.IsValid() )
                {
                    do
                    {
                        if(rpOld->_fForceNewLine)
                        {
                            yHeight += rpOld->_yHeight;
                            // fix for bug 16519
                            // Keep track of the line that contributes to the max height, ex: if the
                            // last lines top margin is negative.
                            if(yHeightDisplay < yHeight)
                                yHeightDisplay = yHeight;
                        }
                        else if(rpOld->IsFrame())
                        {
                            yAlignDescent = yHeight + rpOld->_yHeight;
                        }


                        _dcpCalcMax += rpOld->_cch;
                    }
                    while( rpOld.NextLine(FALSE,FALSE) ); // NextRun()
                }

                // Make sure _dcpCalcMax is sane after the above update
                AssertSz(GetMaxCpCalced() <= cpLast,
                    "CDisplay::RecalcLines match extends beyond EOF");

                // We stop calculating here.Note that if _dcpCalcMax < size
                // of text, this means a background recalc is in progress.
                // We will let that background recalc get the arrays
                // fully in sync.
                AssertSz(GetMaxCpCalced() <= cpLast,
                        "CDisplay::Match less but no background recalc");

                if (GetMaxCpCalced() != cpLast)
                {
                    // This is going to be finished by the background recalc
                    // so set the done flag appropriately.
                    fDone = FALSE;
                }

                goto match;
            }
        }

        // Add a new line
        pliNew = RecalcLinePtr.AddLine();
        if (!pliNew)
        {
            Assert(FALSE);
            goto err;
        }

        PerfDbgLog1(tagRecalc, this, "Measuring new line from cp = %d", me.GetCp());

        // measure can add lines for aligned sites
        iNewLine = iOldLine + aryNewLines.Count() - 1;

        uiFlags = MEASURE_BREAKATWORD |
                    (yHeight == 0 ? MEASURE_FIRSTLINE : 0) |
                    (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

        if ( GetWordWrap() && GetWrapLongLines() )
        {
            uiFlags |= MEASURE_BREAKLONGLINES;
        }

        if (long(me.GetCp()) == pFlowLayout->GetContentLastCp() - 1)
        {
            uiFlags |= MEASURE_EMPTYLASTLINE;
        }

        if (!RecalcLinePtr.MeasureLine(me, uiFlags,
                                 &iNewLine, &yHeight, &yAlignDescent,
                                 NULL, NULL))
        {
            goto err;
        }

        fFirstInPara = (iNewLine >= 0)
                        ? RecalcLinePtr[iNewLine]->IsNextLineFirstInPara()
                        : TRUE;

        // fix for bug 16519
        // Keep track of the line that contributes to the max height, ex: if the
        // last lines top margin is negative.
        if(yHeightDisplay < yHeight)
            yHeightDisplay = yHeight;

        if (iNewLine >= 0)
            NoteMost(RecalcLinePtr[iNewLine]);

        --iMinimumLinesToRecalc;
        if(iMinimumLinesToRecalc < 0 &&
           fBackground &&  (GetTickCount() >= (DWORD)BgndTickMax())) // GetQueueStatus(QS_ALLEVENTS))
        {
            fDone = FALSE;          // Took too long, stop for now
            break;
        }

        if (    fAllowBackgroundRecalc
            &&  yHeight > YWait()
            &&  (LONG) me.GetCp() > CPWait()
            &&  cliWait-- <= 0)
        {                           // Not really done, just past region we're
            fDone = FALSE;          //  waiting for so let background recalc
            break;                  //  take it from here
        }
    }                               // while(me < cpLast ...

no_match:
    // Didn't find match: whole line array from _iliFirst needs to be changed
    // Indicate no match
    pled->SetNoMatch();

    // Update old match position with previous total height of the document so
    // that UpdateView can decide whether to invalidate past the end of the
    // document or not
    pled->_iliMatchOld = LineCount();
    pled->_cpMatchOld  = cpLast + cchOld - cchNew;
    pled->_yMatchOld   = _yHeightMax;

    // Update last recalced cp
    _dcpCalcMax = me.GetCp() - cpLayoutFirst;

    // Replace old lines by new ones
    rpOld = pled->_iliFirst;

    // We store the result from the replace because although it can fail the
    // fields used for first visible must be set to something sensible whether
    // the replace fails or not. Further, the setting up of the first visible
    // fields must happen after the Replace because the lines could have
    // changed in length which in turns means that the first visible position
    // has failed.
    fReplaceResult = rpOld.Replace(-1, &aryNewLines);
    if (!fReplaceResult)
    {
        Assert(FALSE);
        goto err;
    }

    // Adjust first affected line if this line is gone
    // after replacing by new lines

    if(pled->_iliFirst >= LineCount() && LineCount() > 0)
    {
        Assert(pled->_iliFirst == LineCount());
        pled->_iliFirst = LineCount() - 1;
        Assert(!Elem(pled->_iliFirst)->IsFrame());
        pled->_yFirst -= Elem(pled->_iliFirst)->_yHeight;
        AssertSz(pled->_yFirst >= 0, "CDisplay::RecalcLines _yFirst < 0");
        pled->_cpFirst -= Elem(pled->_iliFirst)->_cch;
    }

match:

    // If there is a background on the paragraph, we have to make sure to redraw the
    // lines to the end of the paragraph.
    for (;pled->_iliMatchNew < LineCount();)
    {
        pliNew = Elem(pled->_iliMatchNew);
        if (pliNew)
        {
            if (!pliNew->_fHasBackground)
                break;

            pled->_iliMatchOld++;
            pled->_iliMatchNew++;
            pled->_cpMatchOld += pliNew->_cch;
            pled->_cpMatchNew += pliNew->_cch;
            me.Advance(pliNew->_cch);
            if (pliNew->_fForceNewLine)
            {
                pled->_yMatchOld +=  pliNew->_yHeight;
                pled->_yMatchNew +=  pliNew->_yHeight;
            }
            if (pliNew->_fHasEOP)
                break;
        }
        else
            break;
    }

    _fRecalcDone = fDone;
    _yCalcMax = yHeightDisplay;

    PerfDbgLog1(tagRecalc, this, "CDisplay::RecalcLine(tpFirst, ...) - Done. Recalced down to line #%d", LineCount() - 1);

    if (HasBgRecalcInfo())
    {
        CBgRecalcInfo * pBgRecalcInfo = BgRecalcInfo();
        // Clear wait fields since we want caller's to set them up.
        pBgRecalcInfo->_yWait = -1;
        pBgRecalcInfo->_cpWait = -1;
    }

    // We've measured the last line in the document. Do we want an empty lne?
    if ((LONG)me.GetCp() == cpLast)
    {
        LONG ili = LineCount() - 1;

        // If we haven't measured any lines (deleted an empty paragraph),
        // we need to get a pointer to the last line rather than using the
        // last line measured.
        while (ili >= 0)
        {
            pliNew = Elem(ili);
            if(pliNew->IsTextLine())
                break;
            else
                ili--;
        }

        // If the last line has a paragraph break or we don't have any
        // line any more, we need to create a empty line only if we are in design mode
        if (    LineCount() == 0
            ||  (   GetFlowLayout()->IsEditable()
                &&  pliNew
                &&  (   pliNew->_fHasEOP
                    ||  pliNew->_fHasBreak
                    ||  pliNew->_fSingleSite)))
        {
            // Only create an empty line after a table (at the end
            // of the document) if the table is completely loaded.
            if (pliNew == NULL ||
                !pliNew->_fSingleSite ||
                me._pRunOwner->Tag() != ETAG_TABLE ||
                DYNCAST(CTableLayout, me._pRunOwner)->IsCompleted())
            {
                RecalcLinePtr.Init((CLineArray *)this, 0, NULL);
                CreateEmptyLine(&me, &RecalcLinePtr, &yHeight,
                                pliNew ? pliNew->_fHasEOP : TRUE);
                // fix for bug 16519
                // Keep track of the line that contributes to the max height, ex: if the
                // last lines top margin is negative.
                if(yHeightDisplay < yHeight)
                    yHeightDisplay = yHeight;
            }
        }

        // In table cells, Netscape actually adds the interparagraph spacing
        // for any closed tags to the height of the table cell.
        // BUGBUG: This actually applies the spacing to all nested text containers, not just
        //         table cells. Is this correct? (brendand)
        // It's not correct to add the spacing at all, but given that Netscape
        // has done so, and that they will probably do so for floating block
        // elements. Arye.
        else
        {
            int iLineLast = ili;

            // we need to force scrolling when bottom-margin is set on the last block tag
            // in body. (bug 20400)

            // Only do this if we're the last line in the text site.
            // This means that the last line is a text line.
            if (iLineLast == LineCount() - 1)
            {
                while (iLineLast-- > 0 &&   // Lines available
                       !pliNew->_fForceNewLine &&   // Just a dummy line.
                       pliNew->_yHeight == 0 && pliNew->_yExtent == 0)    // Line contains no text.
                    --pliNew;
            }
            _yBottomMargin += RecalcLinePtr.NetscapeBottomMargin(&me, pliNew);
        }
    }

    if (fDone)
    {
        RecalcMost();

        if(fBackground)
        {
            StopBackgroundRecalc();
        }
    }

    xWidth = CalcDisplayWidth();
    yHeightMax = max(yHeightDisplay, yAlignDescent);

    Assert (pled);

    // If the size changed, re-size or update scrollbars as appropriate
    if (    yHeightMax != _yHeightMax
        ||  yHeightDisplay != _yHeight
        ||  xWidth != _xWidth)
    {
        _xWidth       = xWidth;
        _yHeight      = yHeightDisplay;
        _yHeightMax   = yHeightMax;

        pFlowLayout->SizeContentDispNode(CSize(GetMaxWidth(), GetHeight()));

        // If our contents affects our size, ask our parent to initiate a re-size
        if (    pFlowLayout->GetAutoSize()
            ||  pFlowLayout->_fContentsAffectSize)
        {
            pFlowLayout->ElementOwner()->ResizeElement();
        }
    }

    // Update the relative line cache.
    if (pFlowLayout->_fContainsRelative)
        UpdateRelDispNodeCache(pled);

    AdjustDispNodes(me._pDispNodePrev, pled);

    pFlowLayout->NotifyMeasuredRange(pled->_cpFirst, me.GetCp());

    if (pled->_cpMatchNew != MAXLONG && (pled->_yMatchNew - pled->_yMatchOld))
    {
        CSize size(0, pled->_yMatchNew - pled->_yMatchOld);

        pFlowLayout->NotifyTranslatedRange(size, pled->_cpMatchNew, cpLayoutFirst + _dcpCalcMax);
    }

    // If not done, do the rest in background
    if(!fDone && !fBackground)
        StartBackgroundRecalc(pci->_grfLayout);

    if(fDone)
    {
        CheckLineArray();
        _fLineRecalcErr = FALSE;
    }

cleanup:

    SwitchesEndTimer(SWITCHES_TIMER_RECALCLINES);

    return TRUE;

err:
    if(!_fLineRecalcErr)
    {
        _dcpCalcMax = me.GetCp() - cpLayoutFirst;
        _yCalcMax   = yHeightDisplay;
        _fLineRecalcErr = FALSE;            //  fix up CArray & bail
    }

    TraceTag((tagError, "CDisplay::RecalcLines() failed"));

    pled->SetNoMatch();

    if(!fReplaceResult)
    {
        FlushRecalc();
    }

    // Update the relative line cache.
    if (pFlowLayout->_fContainsRelative)
        UpdateRelDispNodeCache(pled);

    AdjustDispNodes(me._pDispNodePrev, pled);

    return FALSE;
}

/*
 *  CDisplay::UpdateView(&tpFirst, cchOld, cchNew, fRecalc)
 *
 *  @mfunc
 *      Recalc lines and update the visible part of the display
 *      (the "view") on the screen.
 *
 *  @devnote
 *      --- Use when in-place active only ---
 *
 *  @rdesc
 *      TRUE if success
 */
BOOL CDisplay::UpdateView(
    CCalcInfo * pci,
    LONG cpFirst,   //@parm Text ptr where change happened
    LONG cchOld,    //@parm Count of chars deleted
    LONG cchNew)   //@parm No recalc need (only rendering change) = FALSE
{
    if (GetFlowLayout()->FExternalLayout())
        {
        // incremental updates are done by QuillLayout by calling Invalidate
        return TRUE;
        }

    BOOL            fReturn = TRUE;
    BOOL            fNeedViewChange = FALSE;
    RECT            rcView;
    CFlowLayout *   pFlowLayout = GetFlowLayout();
    CElement    *   pElementFL  = pFlowLayout->ElementOwner();
    CRect           rc;
    long            xWidthParent;
    long            yHeightParent;

    BOOL fInvalAll = FALSE;
    CStackDataAry < RECT, 10 > aryInvalRects(Mt(CDisplayUpdateView_aryInvalRects_pv));

    Assert(pci);

    if (_fNoUpdateView)
        return fReturn;

    // If we have no width, don't even try to recalc, it causes
    // crashes in the scrollbar code, and is just a waste of
    // time, anyway.
    // However, since we're not sized, request sizing from the parent
    // of the ped (this will leave the necessary bread-crumb chain
    // to ensure we get sized later) - without this, not all containers
    // of text sites (e.g., 2D sites) will know to size the ped.
    // (This is easier than attempting to propagate back out that sizing
    //  did not occur.)
    if (GetViewWidth() <= 0)
    {
        _xWidth = 0;
        _yHeight = 0;
        _yHeightMax = 0;
        pFlowLayout->ElementOwner()->ResizeElement();
        return TRUE;
    }

    pFlowLayout->GetClientRect((CRect *)&rcView, 0, pci);

    GetViewWidthAndHeightForChild(pci, &xWidthParent, &yHeightParent);

    //
    // BUGBUG(SujalP, SriniB and BrendanD): These 2 lines are really needed here
    // and in all places where we instantiate a CI. However, its expensive right
    // now to add these lines at all the places and hence we are removing them
    // from here right now. We have also removed them from CTxtSite::CalcTextSize()
    // for the same reason. (Bug#s: 58809, 62517, 62977)
    //

    pci->SizeToParent(xWidthParent, yHeightParent);

    // If we get here, we are updating some general characteristic of the
    // display and so we want the cursor updated as well as the general
    // change otherwise the cursor will land up in the wrong place.
    _fUpdateCaret = TRUE;

    if (!LineCount())
    {
        // No line yet, recalc everything
        RecalcView(pci, TRUE);

        // Invalidate entire view rect
        fInvalAll = TRUE;
        fNeedViewChange = TRUE;
    }
    else
    {
        CLed            led;

        if( cpFirst > GetMaxCpCalced())
        {
            // we haven't even calc'ed this far, so don't bother with updating
            // here.  Background recalc will eventually catch up to us.
            return TRUE;
        }

        AssertSz(cpFirst <= GetMaxCpCalced(),
                 "CDisplay::UpdateView(...) - cpFirst > GetMaxCpCalced()");

        if (!RecalcLines(pci, cpFirst, cchOld, cchNew, FALSE, &led))
        {
            // we're in deep crap now, the recalc failed
            // let's try to get out of this with our head still mostly attached
            fReturn = FALSE;
            fInvalAll = TRUE;
            fNeedViewChange = TRUE;
        }

        if (!fInvalAll)
        {
            // Invalidate all the individual rectangles necessary to work around
            // any embedded images. Also, remember if this was a simple or a complex
            // operation so that we can avoid scrolling in difficult places.
            CLine * pLine;
            int     iLine, iLineLast;
            long    xLineLeft, xLineRight, yLineTop, yLineBottom;
            long    yTop;
            long    dy = led._yMatchNew - led._yMatchOld;
            BOOL    fSkip       = FALSE;

            // Start out with a zero area rectangle.
            yTop      =
            rc.bottom =
            rc.top    = led._yFirst;
            rc.left   = MAXLONG;
            rc.right  = 0;

            // Need this to adjust for negative line heights.
            // Note that this also initializes the counter for the
            // for loop just below.
            iLine = led._iliFirst;

            // Accumulate rectangles of lines and invalidate them.
            iLineLast = min(led._iliMatchNew, LineCount());

            // Invalidate only the lines that have been touched by the recalc
            for (; iLine < iLineLast; iLine++)
            {
                pLine = Elem(iLine);

                if (pLine == NULL)
                    break;

                // Get the left and right edges of the line.
                if(!_fRTL)
                {
                    xLineLeft  = pLine->_xLeftMargin;
                    xLineRight = pLine->_xLeftMargin + pLine->_xLineWidth;
                }
                else
                {
                    xLineLeft  = pLine->_xRightMargin + pLine->_xLineWidth;
                    xLineRight = pLine->_xRightMargin;
                }

                // Get the top and bottom edges of the line
                yLineTop    = yTop + pLine->GetYLineTop();
                yLineBottom = yTop + pLine->GetYLineBottom();

                // First line of a new rectangle
                if (rc.right < rc.left)
                {
                    rc.left  = xLineLeft;
                    rc.right = xLineRight;
                }

                // Different margins, output the old one and restart.
                else if (rc.left != xLineLeft || rc.right != xLineRight)
                {
                    // if we have multiple chunks in the same line
                    if( rc.right  == xLineLeft &&
                        rc.top    == yLineTop  &&
                        rc.bottom == yLineBottom )
                    {
                        rc.right = xLineRight;
                    }
                    else
                    {
                        IGNORE_HR(aryInvalRects.AppendIndirect(&rc));

                        fNeedViewChange = TRUE;

                        // Zero height.
                        rc.top    =
                        rc.bottom = yTop;

                        // Just the width of the given line.
                        rc.left  = xLineLeft;
                        rc.right = xLineRight;
                    }
                }

                // Negative line height.
                rc.top = min(rc.top, yLineTop);

                rc.bottom = max(rc.bottom, yLineBottom);

                // Otherwise just accumulate the height.
                if(pLine->_fForceNewLine)
                {
                    yTop  += pLine->_yHeight;

                    // Stop when reaching the bottom of the visible area
                    if (rc.bottom > rcView.bottom)
                        break;
                }
            }

// BUBUG (srinib) - This is a temporary hack to handle the
// scrolling case until the display tree provides the functionality
// to scroll an rc in view. If the new content height changed then
// scroll the content below the change by the dy. For now we are just
// to the end of the view.
            if(dy)
            {
                rc.left   = rcView.left;
                rc.right  = rcView.right;
                rc.bottom = rcView.bottom;
            }

            // Get the last one.
            if (rc.right > rc.left && rc.bottom > rc.top)
            {
                IGNORE_HR(aryInvalRects.AppendIndirect(&rc));
                fNeedViewChange = TRUE;
            }

            // There might be more stuff which has to be
            // invalidated because of lists (numbers might change etc)
            if (UpdateViewForLists(&rcView,   cpFirst,
                                   iLineLast, rc.bottom, &rc))
            {
                IGNORE_HR(aryInvalRects.AppendIndirect(&rc));
                fNeedViewChange = TRUE;
            }

            if (    led._yFirst >= rcView.top
                &&  (   led._yMatchNew <= rcView.bottom
                    ||  led._yMatchOld <= rcView.bottom))
            {
                WaitForRecalcView(pci);
            }
        }
    }

    {
        // Now do the invals
        if (fInvalAll)
        {
            pFlowLayout->Invalidate(NULL, INVAL_CHILDWINDOWS);
        }
        else
        {
            int nIndex = 0;

            for (nIndex = 0; nIndex < aryInvalRects.Size(); nIndex++)
            {
                pFlowLayout->Invalidate(&aryInvalRects[nIndex], 0L);
            }
        }
    }

    if (fNeedViewChange)
    {
        pFlowLayout->ViewChange(FALSE);
    }

    CheckView();

#ifdef DBCS
    UpdateIMEWindow();
#endif

    return fReturn;
}

/*
 *  CDisplay::CalcDisplayWidth()
 *
 *  @mfunc
 *      Computes and returns width of this display by walking line
 *      CArray and returning the widest line.  Used for
 *      horizontal scroll bar routines
 *
 *  @todo
 *      This should be done by a background thread
 */

LONG CDisplay::CalcDisplayWidth ()
{
    LONG    ili = LineCount();
    CLine * pli;
    LONG    xWidth = 0;

    if(ili)
    {
        // Note: pli++ breaks array encapsulation (pli = &(*this)[ili] doesn't)
        pli = Elem(0);
        for(xWidth = 0; ili--; pli++)
        {
            // calc line width based on the direction
            if(!_fRTL)
            {
                xWidth = max(xWidth, pli->GetTextRight() + pli->_xRight);
            }
            else
            {
                xWidth = max(xWidth, pli->GetRTLTextLeft() + pli->_xLeft);
            }
        }
    }

    return xWidth;
}


/*
 *  CDisplay::StartBackgroundRecalc()
 *
 *  @mfunc
 *      Starts background line recalc (at _dcpCalcMax position)
 *
 *  @todo
 *      ??? CF - Should use an idle thread
 */
void CDisplay::StartBackgroundRecalc(DWORD grfLayout)
{
    // We better be in the middle of the job here.
    Assert (LineCount() > 0);

    Assert(CanHaveBgRecalcInfo());

    Assert(!GetFlowLayout()->FExternalLayout());

    // StopBack.. kills the recalc task, *if it exists*
    StopBackgroundRecalc () ;

    EnsureBgRecalcInfo();

    if(!RecalcTask() && GetMaxCpCalced() < GetLastCp())
    {
        BgRecalcInfo()->_pRecalcTask = new CRecalcTask (this, grfLayout) ;
        if (RecalcTask())
        {
            _fRecalcDone = FALSE;
        }
        // BUGBUG (sujalp): what to do if we fail on mem allocation?
        // One solution is to create this task as blocked when we
        // construct CDisplay, and then just unblock it here. In
        // StopBackgroundRecalc just block it. In CDisplay destructor,
        // destruct the CTask too.

        GetFlowLayout()->Doc()->_lsCache.SetAutoDispose(FALSE);
    }
}


/*
 *  CDisplay::StepBackgroundRecalc()
 *
 *  @mfunc
 *      Steps background line recalc (at _dcpCalcMax position)
 *      Called by timer proc
 *
 *  @todo
 *      ??? CF - Should use an idle thread
 */
VOID CDisplay::StepBackgroundRecalc (DWORD dwTimeOut, DWORD grfLayout)
{
    LONG cch = GetLastCp() - (GetMaxCpCalced());

    Assert(!GetFlowLayout()->FExternalLayout());

    // don't try recalc when processing OOM or had an error doing recalc or
    // if we are asserting.
    if(_fInBkgndRecalc || _fLineRecalcErr )
    {
#if DBG==1
        if(_fInBkgndRecalc)
            PerfDbgLog(tagRecalc, this, "avoiding reentrant background recalc");
        else
            PerfDbgLog(tagRecalc, this, "OOM: not stepping recalc");
#endif
        return;
    }

    _fInBkgndRecalc = TRUE;

    // Background recalc is over if no more characters or we are no longer
    // active.
    if(cch <= 0)
    {
        if (RecalcTask())
        {
            StopBackgroundRecalc();
        }

        CheckLineArray();

        goto Cleanup;
    }

    {
        CFlowLayout *   pFlowLayout = GetFlowLayout();
        CElement    *   pElementFL = pFlowLayout->ElementOwner();
        LONG            cp = GetMaxCpCalced();

        if (    !pElementFL->IsDisplayNone()
            ||   pElementFL->Tag() == ETAG_BODY)
        {
            CCalcInfo   CI;
            CLed        led;
            long        xParentWidth;
            long        yParentHeight;

            // Setup the amount of time we have this time around
            Assert(BgRecalcInfo() && "Supposed to have a recalc info in stepbackgroundrecalc");
            BgRecalcInfo()->_dwBgndTickMax = dwTimeOut ;

            CI.Init(pFlowLayout);
            GetViewWidthAndHeightForChild(
                &CI,
                &xParentWidth,
                &yParentHeight,
                CI._smMode == SIZEMODE_MMWIDTH);
            CI.SizeToParent(xParentWidth, yParentHeight);

            CI._grfLayout = grfLayout;

            RecalcLines(&CI, cp, cch, cch, TRUE, &led);
        }
        else
        {
            CNotification  nf;

            // Kill background recalc, if the layout is hidden
            StopBackgroundRecalc();

            // BUGBUG (MohanB) Need to use ElementContent()?
            // calc the rest by accumulating a dirty range.
            nf.CharsResize(GetMaxCpCalced(), cch, pElementFL->GetFirstBranch());
            GetMarkup()->Notify(&nf);
        }
    }

Cleanup:
    _fInBkgndRecalc = FALSE;

    return;
}


/*
 *  CDisplay::StopBackgroundRecalc()
 *
 *  @mfunc
 *      Steps background line recalc (at _dcpCalcMax position)
 *      Called by timer proc
 *
 */
VOID CDisplay::StopBackgroundRecalc()
{
    if (HasBgRecalcInfo())
    {
        if (RecalcTask())
        {
            RecalcTask()->Terminate () ;
            RecalcTask()->Release () ;
            _fRecalcDone = TRUE;
        }
        // BUGBUG for now ...
        // pFlowLayout->Doc()->_pInPlace->_pFrame->SetStatusText(_T("Done"));

        DeleteBgRecalcInfo();
        GetFlowLayout()->Doc()->_lsCache.SetAutoDispose(TRUE);
    }
}

/*
 *  CDisplay::WaitForRecalc(cpMax, yMax, pDI)
 *
 *  @mfunc
 *      Ensures that lines are recalced until a specific character
 *      position or ypos.
 *
 *  @rdesc
 *      success
 */
BOOL CDisplay::WaitForRecalc(
    LONG cpMax,     //@parm Position recalc up to (-1 to ignore)
    LONG yMax,      //@parm ypos to recalc up to (-1 to ignore)
    CCalcInfo * pci)

{
    CFlowLayout *   pFlowLayout = GetFlowLayout();
    BOOL            fReturn = TRUE;
    LONG            cch;
    CCalcInfo       CI;

    if (GetFlowLayout()->FExternalLayout())
        return TRUE;

    // do not measure any hidden stuff
    if(pFlowLayout->IsDisplayNone() && pFlowLayout->Tag() != ETAG_BODY)
        return fReturn;

    // Prepare the CCalcInfo
    if(!pci)
    {
        CI.Init(pFlowLayout);
        pci = &CI;
    }
    else
    {
        pci->EnsureFilterOpen();
    }

    if (!pFlowLayout->TestLock(CElement::ELEMENTLOCK_RECALC))
    {
        pFlowLayout->CommitChanges(pci);
    }

    Assert(cpMax < 0 || (cpMax >= GetFirstCp() && cpMax <= GetLastCp()));

    if((yMax < 0 || yMax >= _yCalcMax) &&
       (cpMax < 0 || cpMax >= GetMaxCpCalced()))
    {

        if (pFlowLayout->TestLock(CElement::ELEMENTLOCK_RECALC))
            return FALSE;

        cch = GetLastCp() - GetMaxCpCalced();
        if(cch > 0 || LineCount() == 0)
        {

            HCURSOR     hcur = NULL;
            CDoc *      pDoc = pFlowLayout->Doc();

            if (EnsureBgRecalcInfo() == S_OK)
            {
                CBgRecalcInfo * pBgRecalcInfo = BgRecalcInfo();
                Assert(pBgRecalcInfo && "Should have a BgRecalcInfo");
                pBgRecalcInfo->_cpWait = cpMax;
                pBgRecalcInfo->_yWait  = yMax;
            }

            if (pDoc && pDoc->State() >= OS_INPLACE)
            {
                hcur = SetCursorIDC(IDC_WAIT);
            }
            TraceTag((tagWarning, "Lazy recalc"));

            if(GetMaxCpCalced() == GetFirstCp() )
            {
                fReturn = RecalcLines(pci);
            }
            else
            {
                CLed led;

                fReturn = RecalcLines(pci, GetMaxCpCalced(), cch, cch, FALSE, &led, TRUE);
            }

            SetCursor(hcur);
        }
    }

    return fReturn;
}

#ifdef WIN16
#pragma code_seg ("DISP2_TEXT")
#endif

/*
 *  CDisplay::WaitForRecalcIli
 *
 *  @mfunc
 *      Wait until line array is recalculated up to line <p ili>
 *
 *  @rdesc
 *      Returns TRUE if lines were recalc'd up to ili
 */


#pragma warning(disable:4702)   //  Ureachable code

BOOL CDisplay::WaitForRecalcIli (
    LONG ili,       //@parm Line index to recalculate line array up to
    CCalcInfo * pci)
{
    Assert(!GetFlowLayout()->FExternalLayout());

    // BUGBUG for now (istvanc)
    return ili < LineCount();

    LONG cchGuess;

    while(!_fRecalcDone && ili >= LineCount())
    {
        cchGuess = 5 * (ili - LineCount() + 1) * Elem(0)->_cch;
        if(!WaitForRecalc(GetMaxCpCalced() + cchGuess, -1, pci))
            return FALSE;
    }
    return ili < LineCount();
}

//+----------------------------------------------------------------------------
//
//  Member:     WaitForRecalcView
//
//  Synopsis:   Calculate up through the bottom of the visible content
//
//  Arguments:  pci - CCalcInfo to use
//
//  Returns:    TRUE if all Ok, FALSE otherwise
//
//-----------------------------------------------------------------------------
BOOL
CDisplay::WaitForRecalcView(CCalcInfo * pci)
{
    return WaitForRecalc(-1, GetFlowLayout()->GetClientBottom(), pci);
}


#pragma warning(default:4702)   //  re-enable unreachable code

//===================================  View Updating  ===================================


/*
 *  CDisplay::SetViewSize(rcView)
 *
 *  Purpose:
 *      Set the view size and return whether a full recalc is needed
 *
 *  Returns:
 *      TRUE if a full recalc is needed
 */
BOOL CDisplay::SetViewSize(const RECT &rcView, BOOL fViewUpdate)
{
    BOOL    fRecalc = ((rcView.right - rcView.left) != _xWidthView);

    _xWidthView  = rcView.right  - rcView.left;
    _yHeightView = rcView.bottom - rcView.top;

    // don't look at line count if external rendering is used
    if (!GetFlowLayout()->FExternalLayout())
    {
        return fRecalc || !LineCount();
    }
    else
    {
        return fRecalc ||
               (GetFlowLayout()->GetQuillGlue() && !GetFlowLayout()->GetQuillGlue()->HasLines());
    }
}

/*
 *  CDisplay::RecalcView
 *
 *  Sysnopsis:  Recalc view and update first visible line
 *
 *  Arguments:
 *      fFullRecalc - TRUE if recalc from first line needed, FALSE if enough
 *                    to start from _dcpCalcMax
 *
 *  Returns:
 *      TRUE if success
 */
BOOL CDisplay::RecalcView(CCalcInfo * pci, BOOL fFullRecalc)
{
    CFlowLayout * pFlowLayout = GetFlowLayout();
    BOOL          fAllowBackgroundRecalc;
    CLayout     * pLayout;
    DWORD         dw;

    // If we have no width, don't even try to recalc, it causes
    // crashes in the scrollbar code, and is just a waste of time
    if (GetViewWidth() <= 0 && !pFlowLayout->_fContentsAffectSize)
    {
        _xWidth     = 0;
        _yHeight    = 0;
        _yHeightMax = 0;
        return TRUE;
    }

    fAllowBackgroundRecalc = AllowBackgroundRecalc(pci);

    // If a full recalc (from first line) is not needed
    // go from current _dcpCalcMax on
    if (!fFullRecalc)
    {
        return (fAllowBackgroundRecalc
                        ? WaitForRecalcView(pci)
                        : WaitForRecalc(GetLastCp(), -1, pci));
    }

    // Else do full recalc
    BOOL  fRet = TRUE;

    // If all that the element is likely to have is a single line of plain text,
    // use a faster mechanism to compute the lines. This is great for perf
    // of <INPUT>
    if (pFlowLayout->Tag() == ETAG_INPUT)
    {
        fRet = RecalcPlainTextSingleLine(pci);
        pFlowLayout->_fChildWidthPercent  =
        pFlowLayout->_fChildHeightPercent = FALSE;
    }
    else
    {
        // full recalc lines
        if(!RecalcLines(pci))
        {
            // we're in deep crap now, the recalc failed
            // let's try to get out of this with our head still mostly attached
            fRet = FALSE;
            goto Done;
        }

        // Note if any contained sites have a percentage width or height
        // (This check is usually perform as a by-product of positioning the sites.
        //  However, when RecalcView is used, such as during min/max calculations,
        //  that code is not called; thus it is duplicated here.)
        pFlowLayout->_fChildWidthPercent  =
        pFlowLayout->_fChildHeightPercent = FALSE;

        for (pLayout = pFlowLayout->GetFirstLayout(&dw);
             pLayout;
             pLayout = pFlowLayout->GetNextLayout(&dw))
        {
            pFlowLayout->_fChildWidthPercent  = (pFlowLayout->_fChildWidthPercent  ||
                                                        pLayout->PercentWidth());
            pFlowLayout->_fChildHeightPercent = (pFlowLayout->_fChildHeightPercent ||
                                                        pLayout->PercentHeight());
        }
        pFlowLayout->ClearLayoutIterator(dw);
    }
    CheckView();

Done:
    return fRet;
}

void
SetLineWidth(long xWidthMax, CLine * pli)
{
    pli->_xLineWidth = max(xWidthMax, pli->_xLeft + pli->_xWidth +
                                        pli->_xLineOverhang + pli->_xRight);
}

//+---------------------------------------------------------------
//
//  Member:     CDisplay::RecalcLineShift
//
//  Synopsis:   Run thru line array and adjust line shift
//
//---------------------------------------------------------------


void CDisplay::RecalcLineShift(CCalcInfo * pci, DWORD grfLayout)
{
    CFlowLayout * pFlowLayout = GetFlowLayout();
    LONG        lCount = LineCount();
    LONG        ili;
    LONG        iliFirstChunk = 0;
    BOOL        fChunks = FALSE;
    CLine *     pli;
    long        xShift;
    long        xWidthMax = GetMaxPixelWidth();

    Assert (pFlowLayout->_fSizeToContent ||
            (_fMinMaxCalced && !pFlowLayout->ContainsChildLayout()));

    for(ili = 0, pli = Elem(0); ili < lCount; ili++, pli++)
    {
        // if the current line does not force a new line, then
        // find a line that forces the new line and distribute
        // width.

        if(!fChunks && !pli->_fForceNewLine && !pli->IsFrame())
        {
            iliFirstChunk = ili;
            fChunks = TRUE;
        }

        if(pli->_fForceNewLine)
        {
            if(!fChunks)
                iliFirstChunk = ili;
            else
                fChunks = FALSE;

            if(pli->_fJustified && long(pli->_fJustified) != JUSTIFY_FULL )
            {
                // for pre whitespace is already include in _xWidth
                xShift = xWidthMax - pli->_xLeftMargin - pli->_xLeft - pli->_xWidth - pli->_xLineOverhang -
                         pli->_xRight - pli->_xRightMargin - GetCaret();

                xShift = max(xShift, 0L);           // Don't allow alignment to go < 0
                                                    // Can happen with a target device
                if(long(pli->_fJustified) == JUSTIFY_CENTER)
                    xShift /= 2;

                while(iliFirstChunk < ili)
                {
                    pli = Elem(iliFirstChunk++);
                    if(!_fRTL)
                        pli->_xLeft += xShift;
                    else
                        pli->_xRight += xShift;

                    pli->_xLineWidth = pli->_xLeft + pli->_xWidth +
                                        pli->_xLineOverhang + pli->_xRight;
                }
                pli = Elem(iliFirstChunk++);
                if(!_fRTL)
                    pli->_xLeft += xShift;
                else
                    pli->_xRight += xShift;

            }
            SetLineWidth(xWidthMax, pli);
        }
    }

    if(pFlowLayout->_fContainsRelative)
    {
        VoidRelDispNodeCache();
        UpdateRelDispNodeCache(NULL);
    }
}


/*
 *  CDisplay::CreateEmptyLine()
 *
 *  @mfunc
 *      Create an empty line
 *
 *  @rdesc
 *      TRUE - worked <nl>
 *      FALSE - failed
 *
 */
BOOL CDisplay::CreateEmptyLine(CLSMeasurer * pMe,
    CRecalcLinePtr * pRecalcLinePtr,
    LONG * pyHeight, BOOL fHasEOP )
{
    UINT uiFlags;
    LONG yAlignDescent;
    INT  iNewLine;

    // Make sure that this is being called appropriately
    AssertSz(!pMe || GetLastCp() == long(pMe->GetCp()),
        "CDisplay::CreateEmptyLine called inappropriately");

    // Assume failure
    BOOL    fResult = FALSE;

    // Add one new line
    CLine *pliNew = pRecalcLinePtr->AddLine();

    if (!pliNew)
    {
        Assert(FALSE);
        goto err;
    }

    iNewLine = pRecalcLinePtr->Count() - 1;

    Assert (iNewLine >= 0);

    uiFlags = fHasEOP ? MEASURE_BREAKATWORD |
                        MEASURE_FIRSTINPARA :
                        MEASURE_BREAKATWORD;

    // Make sure we're positioned at the correct block element
    // and that we remain there. AdjustForInsert will put us
    // in the right place, MEASURE_EMPTYLASTLINE will keep us
    // there while the measurer runs.

#ifdef MERGEFUN // rtp
    pMe->AdjustForInsert();
#endif

    uiFlags |= MEASURE_EMPTYLASTLINE;

    // If this is the first line in the document.
    if (*pyHeight == 0)
        uiFlags |= MEASURE_FIRSTLINE;

    if (!pRecalcLinePtr->MeasureLine(*pMe, uiFlags,
                                   &iNewLine, pyHeight, &yAlignDescent,
                                   NULL, NULL))
    {
        goto err;
    }

    // If we made it to here, everything worked.
    fResult = TRUE;

err:

    return fResult;
}


//====================================  Rendering  =======================================


/*
 *  CDisplay::Draw(CFormDrawInfo *pDI)
 *  Purpose:
 *      General drawing method
 *      To be called in response to IViewObject::Draw() or WM_PAINT
 *
 *  Returns
 *      HRESULT
 */
HRESULT CDisplay::Draw(
    CFormDrawInfo * pDI,
    RECT *          prcView)
{
    CFlowLayout * pFlowLayout = GetFlowLayout();
    CDoc        * pDoc        = pFlowLayout->Doc();
    BOOL    fInBrowseMode     = !pDoc->_pElemCurrent->IsEditable();
    HRESULT hr = S_OK;
    CPeerHolder * pPeerHolder = pFlowLayout->ElementOwner()->GetRenderPeerHolder();

    Assert(prcView);

    if (pPeerHolder && pPeerHolder->TestRenderFlags(BEHAVIORRENDERINFO_BEFORECONTENT))
        IGNORE_HR(pPeerHolder->Draw(pDI, pFlowLayout, BEHAVIORRENDERINFO_BEFORECONTENT));

    if (!(pPeerHolder && pPeerHolder->TestRenderFlags(BEHAVIORRENDERINFO_DISABLECONTENT)))
    {
        // Now render
        Render(pDI, *prcView, *pDI->ClipRect());
    }

    if (pPeerHolder && pPeerHolder->TestRenderFlags(BEHAVIORRENDERINFO_AFTERCONTENT))
        IGNORE_HR(pPeerHolder->Draw(pDI, pFlowLayout, BEHAVIORRENDERINFO_AFTERCONTENT));

    RRETURN(hr);
}

//====================================  Rendering  =======================================


/*
 *  CDisplay::Render(rcView, rcRender)
 *
 *  @mfunc
 *      Searches paragraph boundaries around a range
 *
 *  returns: the lowest yPos
 */
LONG
CDisplay::Render (
    CFormDrawInfo * pDI,
    const RECT &rcView,     //@parm View RECT
    const RECT &rcRender,   //@parm RECT to render (must be container in
    LONG iliStop )          //@parm optional last line to render (for print)
{
#ifdef SWITCHES_ENABLED
    if (IsSwitchNoRenderLines())
        return 0;
#endif

    CFlowLayout * pFlowLayout = GetFlowLayout();
    CElement    * pElementFL  = pFlowLayout->ElementOwner();
    LONG    ili;
    LONG    iliBgDrawn = -1;
    POINT   pt;
    LONG    cp;
    LONG    yLine;
    LONG    yLi = 0;
    CLine * pli = NULL;
    RECT    rcClip;
    LONG    lCount;
    BOOL    fLineIsPositioned;
    LONG    yRet = 0;

    if (   !pFlowLayout->ElementOwner()->IsInMarkup()
        || (   !pFlowLayout->IsEditable()
            && pFlowLayout->IsDisplayNone()
           )
       )
        return 0;

//    SwitchesBegTimer(SWITCHES_TIMER_RENDERLINES);

    // Create renderer
    CLSRenderer lsre(this, pDI);
    if (!lsre._pLS)
        goto endrender;

    //
    // Ensure the list cache is up-to-date
    //

    ResyncListIndex(pFlowLayout->GetYScroll());

    WHEN_DBG( LONG cpLi; )

    // Calculate line and cp to start the display at
    ili = LineFromPos(rcRender, &yLine, &cp);

    if(LineCount() <= 0 || ili < 0)
        goto ret;

    lCount = (iliStop < 0) ? LineCount() : iliStop + 1;
    lCount = min( LineCount(), lCount );

    // Calculate the point where the text will start being displayed
    pt.x = 0;
    pt.y = yLine;

    rcClip = rcRender;

    // Prepare renderer

    if(!CALLREFUNC(StartRender, (rcView, rcRender)))
        goto done;

    // Perpare the List Index cache.
    PrepareListIndexForRender();

    // If we're just painting the inset, don't check all the lines.
    if (rcRender.right <= rcView.left ||
        rcRender.left >= rcView.right ||
        rcRender.bottom <= rcView.top ||
        rcRender.top >= rcView.bottom)
        goto endrender;

    // Init renderer at the start of the first line to render
    CALLREFUNC(SetCurPoint, (pt));
    CALLREFUNC(SetCp, (cp, NULL));

    WHEN_DBG(cpLi = (LONG)(CALLREFUNC(GetCp, ()));)

    yLi = pt.y;

    // Check if this line begins BEFORE the previous line ended ...
    // Would happen with negative line heights:
    //
    //           ----------------------- <-----+---------- Line 1
    //                                         |
    //           ======================= <-----|-----+---- Line 2
    //  yBeforeSpace__________^                |     |
    //                        |                |     |
    //  yLi ---> -------------+--------- <-----+     |
    //                                               |
    //                                               |
    //           ======================= <-----------+
    //
    RecalcMost();

    // Render each line in the update rectangle

    for (; ili < lCount; ili++)
    {
        // current line
        pli = Elem(ili);

#ifdef MERGEFUN // Neither AdjustForInsert nor AdjustRunForMeasure exists
        // If this is the last line, and it's empty, we
        // need to tell the run adjuster.
        if (long(CALLREFUNC(GetCp, ())) == pFlowLayout->GetLastCp()-1 && ili == LineCount() - 1)
            CALLREFUNC(AdjustForInsert, ());
        else
            CALLREFUNC(AdjustRunForMeasure, (0));
#endif

        // Compute the list index if appropriate.
        if ( pli->_fHasBulletOrNum )
        {
            SetListIndex( lsre.GetCp(), ili );
        }

        // if the most negative line is out off the view from the current
        // yOffset, don't look any further, we have rendered all the lines
        // in the inval'ed region
        if (yLi + min(long(0), pli->GetYTop()) + _yMostNeg >= rcClip.bottom)
        {
            break;
        }

        fLineIsPositioned = FALSE;

        //
        // if the current line is interesting (ignore aligned, clear,
        // hidden and blank lines).
        //
        if(pli->_cch && !pli->_fHidden)
        {
            //
            // if the current line is relative get its y offset and
            // zIndex
            //
            if(pli->_fRelative)
            {
                fLineIsPositioned = TRUE;
            }
        }

        //
        // now check to see if the current line is in view
        //
        if( (yLi + min(long(0), pli->GetYTop())) > rcClip.bottom ||
            (yLi + pli->GetYLineBottom() < rcClip.top))
        {
            //
            // line is not in view, so skip it
            //
            CALLREFUNC(SkipCurLine, (pli));
        }
        else
        {
            //
            // current line is in view, so render it
            //
            // Note: we have to render the background on a relative line,
            // the parent of the relative element might have background.(srinib)
            // fix for #51465
            //
            // if the paragraph has background or borders then compute the bounding rect
            // for the paragraph and draw the background and/or border
            if(iliBgDrawn < ili &&
               (pli->_fHasParaBorder || // if we need to draw borders
                (pli->_fHasBackground && pElementFL->Doc()->PaintBackground())))
            {
                DrawBackgroundAndBorder(lsre.GetCp(), lsre.GetDrawInfo(), ili, lCount,
                                        &iliBgDrawn, yLi, rcView, rcClip, 0);

                //
                // N.B. (johnv) Lines can be added by as
                // DrawBackgroundAndBorders (more precisely,
                // RegionFromElement, which it calls) waits for a
                // background recalc.  Recompute our cached line pointer.
                //
                pli = Elem(ili);
            }

            // if the current line has is positioned it will be taken care
            // of through the SiteDrawList and we shouldn't draw it here.
            //
            if (fLineIsPositioned)
            {
                CALLREFUNC(SkipCurLine,(pli));
            }
            else
            {
                //
                // Finally, render the current line
                //
                lsre.RenderLine(*pli);
            }
        }

        Assert(pli == Elem(ili));

        //
        // update the yOffset for the next line
        //
        if(pli->_fForceNewLine)
            yLi += pli->_yHeight;

        WHEN_DBG( cpLi += pli->_cch; )

        AssertSz( (LONG)CALLREFUNC(GetCp,()) == cpLi,
                  "CDisplay::Render() - cp out of sync. with line table");
        AssertSz( CALLREFUNC(GetCurPoint,()).y == yLi,
                  "CDisplay::Render() - y out of sync. with line table");

    }

    if (lsre._lastTextOutBy != CLSRenderer::DB_LINESERV)
    {
        lsre._lastTextOutBy = CLSRenderer::DB_NONE;
        SetTextAlign(lsre._hdc, TA_TOP | TA_LEFT);
    }

endrender:
    CALLREFUNC(EndRender,());

done:
    yRet = CALLREFUNC(GetCurPoint,()).y;

ret:
//    SwitchesEndTimer(SWITCHES_TIMER_RENDERLINES);
    return yRet;
}

//+---------------------------------------------------------------------------
//
// Member:      DrawBackgroundAndBorders()
//
// Purpose:     Draw background and borders for elements on the current line,
//              and all the consecutive lines that have background.
//
//----------------------------------------------------------------------------
void
CDisplay::DrawBackgroundAndBorder(
            long cpIn,
            CFormDrawInfo *pDI,
            LONG ili, LONG lCount,
            LONG * piliDrawn, LONG yLi, RECT rcView, RECT rcClip,
            long cpLim)
{
    const CCharFormat  * pCF;
    const CFancyFormat * pFF;
    const CParaFormat  * pPF;
    CStackPtrAry < CTreeNode *, 8 > aryNodesWithBgOrBorder(Mt(CDisplayDrawBackgroundAndBorder_aryNodesWithBgOrBorder_pv));
    CDataAry <RECT> aryRects(Mt(CDisplayDrawBackgroundAndBorder_aryRects_pv));
    CFlowLayout *   pFlowLayout = GetFlowLayout();
    CElement    *   pElementFL  = pFlowLayout->ElementContent();
    CMarkup     *   pMarkup = pFlowLayout->GetContentMarkup();
    BOOL            fPaintBackground = pElementFL->Doc()->PaintBackground();
    CTreeNode *     pNodeCurrBranch;
    CTreeNode *     pNode;
    CTreePos  *     ptp;
    long            ich;
    long            cpClip = cpIn;
    long            cp;
    long            lSize;


    if (!GetFlowLayout()->FExternalLayout())
    {
        // find the consecutive set of lines that have background
        while (ili < lCount && yLi + _yMostNeg < rcClip.bottom)
        {
            CLine * pli = Elem(ili++);

            // if the current line has borders or background then
            // continue otherwise return.
            if (!(pli->_fHasBackground && fPaintBackground) &&
                !pli->_fHasParaBorder)
            {
                break;
            }

            if (pli->_fForceNewLine)
            {
                yLi += pli->_yHeight;
            }

            cpClip += pli->_cch;
        }

        if(cpIn != cpClip)
            *piliDrawn = ili - 1;
    }
    else
    {
        cpClip = cpLim;
    }

    // initialize the tree pos that corresponds to the begin cp of the
    // current line
    ptp = pMarkup->TreePosAtCp(cpIn, &ich);

    cp = cpIn - ich;

    // first draw any backgrounds extending into the current region
    // from the previous line.

    pNodeCurrBranch = ptp->GetBranch();

    if(DifferentScope(pNodeCurrBranch, pElementFL))
    {
        pNode = pNodeCurrBranch;

        // run up the current branch and find the ancestors with background
        // or border
        while(pNode)
        {
            if(pNode->HasLayout())
            {
                CLayout * pLayout = pNode->GetLayoutDirect();

                if(pLayout == pFlowLayout)
                    break;

                Assert(pNode == pNodeCurrBranch);
            }
            else
            {
                // push this element on to the stack
                aryNodesWithBgOrBorder.Append(pNode);
            }
            pNode = pNode->Parent();
        }

        Assert(pNode);

        // now that we have all the elements with background or borders
        // for the current branch render them.
        for(lSize = aryNodesWithBgOrBorder.Size(); lSize > 0; lSize--)
        {
            CTreeNode * pNode = aryNodesWithBgOrBorder[lSize - 1];

            if(pNode->IsRelative())
            {
                pNodeCurrBranch = pNode;
                break;
            }
            else
            {
                pCF = pNode->GetCharFormat();
                pFF = pNode->GetFancyFormat();
                pPF = pNode->GetParaFormat();

                if (!pCF->IsVisibilityHidden() && !pCF->IsDisplayNone())
                {
                    BOOL fDrawBorder = pPF->_fPadBord  && pFF->_fBlockNess;
                    BOOL fDrawBackground = fPaintBackground &&
                                            (pFF->_lImgCtxCookie ||
                                             pFF->_ccvBackColor.IsDefined());

                    if (fDrawBackground || fDrawBorder)
                    {
                        DrawElemBgAndBorder(
                            pNode->Element(), &aryRects,
                            &rcView, &rcClip,
                            pDI, NULL,
                            fDrawBackground, fDrawBorder,
                            cpIn, -1);
                    }
                }
            }
        }

        if(pNodeCurrBranch->HasLayout() || pNode->IsRelative())
        {
            CTreePos * ptpBegin;

            pNodeCurrBranch->Element()->GetTreeExtent(&ptpBegin, &ptp);

            cp = ptp->GetCp();
        }

        cp += ptp->GetCch();
        ptp = ptp->NextTreePos();
    }


    // now draw the background of all the elements comming into scope of
    // in the cpRange
    while(ptp && cpClip >= cp)
    {
        if(ptp->IsBeginElementScope())
        {
            pNode = ptp->Branch();
            pCF   = pNode->GetCharFormat();

            // Background and border for a relative element or an element
            // with layout are drawn when the element is hit with a draw.
            if(pNode->HasLayout() || pCF->_fRelative)
            {
                if(DifferentScope(pNode, pElementFL))
                {
                    CTreePos * ptpBegin;

                    pNode->Element()->GetTreeExtent(&ptpBegin, &ptp);
                    cp = ptp->GetCp();
                }
            }
            else
            {
                pCF = pNode->GetCharFormat();
                pFF = pNode->GetFancyFormat();
                pPF = pNode->GetParaFormat();

                if (!pCF->IsVisibilityHidden() && !pCF->IsDisplayNone())
                {
                    BOOL fDrawBorder = pPF->_fPadBord  && pFF->_fBlockNess;
                    BOOL fDrawBackground = fPaintBackground &&
                                            (pFF->_lImgCtxCookie ||
                                             pFF->_ccvBackColor.IsDefined());

                    if (fDrawBackground || fDrawBorder)
                    {
                        DrawElemBgAndBorder(
                                pNode->Element(), &aryRects,
                                &rcView, &rcClip,
                                pDI, NULL,
                                fDrawBackground, fDrawBorder,
                                cp, -1);
                    }
                }
            }
        }

        cp += ptp->GetCch();
        ptp = ptp->NextTreePos();
    }
}

//+----------------------------------------------------------------------------
//
// Function:    BoundingRectForAnArrayOfRectsWithEmptyOnes
//
// Synopsis:    Find the bounding rect that contains a given set of rectangles
//              It does not ignore the rectangles that have left=right, top=bottom
//              or both. It still ignores the rects that have left=right=top=bottom=0
//
//-----------------------------------------------------------------------------

void
BoundingRectForAnArrayOfRectsWithEmptyOnes(RECT *prcBound, CDataAry<RECT> * paryRects)
{
    RECT *  prc;
    LONG    iRect;
    LONG    lSize = paryRects->Size();
    BOOL    fFirst = TRUE;

    SetRectEmpty(prcBound);

    for(iRect = 0, prc = *paryRects; iRect < lSize; iRect++, prc++)
    {
        if((prc->left <= prc->right && prc->top <= prc->bottom) &&
            (prc->left != 0 || prc->right != 0 || prc->top != 0 || prc->bottom != 0) )
        {
            if(fFirst)
            {
                *prcBound = *prc;
                fFirst = FALSE;
            }
            else
            {
                if(prcBound->left > prc->left) prcBound->left = prc->left;
                if(prcBound->top > prc->top) prcBound->top = prc->top;
                if(prcBound->right < prc->right) prcBound->right = prc->right;
                if(prcBound->bottom < prc->bottom) prcBound->bottom = prc->bottom;
            }
        }
    }
}


//+----------------------------------------------------------------------------
//
// Function:    BoundingRectForAnArrayOfRects
//
// Synopsis:    Find the bounding rect that contains a given set of rectangles
//
//-----------------------------------------------------------------------------

void
BoundingRectForAnArrayOfRects(RECT *prcBound, CDataAry<RECT> * paryRects)
{
    RECT *  prc;
    LONG    iRect;
    LONG    lSize = paryRects->Size();

    SetRectEmpty(prcBound);

    for(iRect = 0, prc = *paryRects; iRect < lSize; iRect++, prc++)
    {
        if(!IsRectEmpty(prc))
        {
            UnionRect(prcBound, prcBound, prc);
        }
    }
}


//+----------------------------------------------------------------------------
//
// Member:      DrawElementBackground
//
// Synopsis:    Draw the background for a an element, given the region it
//              occupies in the display
//
//-----------------------------------------------------------------------------

void
CDisplay::DrawElementBackground(CTreeNode * pNodeContext,
                                CDataAry <RECT> * paryRects, RECT * prcBound,
                                const RECT * prcView, const RECT * prcClip,
                                CFormDrawInfo * pDI)
{
    RECT    rcDraw;
    RECT    rcBound;
    RECT *  prc;
    LONG    lSize;
    LONG    iRect;
    SIZE    sizeImg;
    RECT    rcImg;
    LONG    offsetY = 0;
    LONG    offsetX;
    BACKGROUNDINFO bginfo;

    const CFancyFormat *    pFF = pNodeContext->GetFancyFormat();

    Assert(pFF->_lImgCtxCookie || pFF->_ccvBackColor.IsDefined());

// BUGBUG: RTL issue (brendand)
    offsetX = 0;

    CDoc *    pDoc    = GetFlowLayout()->Doc();
    CImgCtx * pImgCtx = (pFF->_lImgCtxCookie
                            ? pDoc->GetUrlImgCtx(pFF->_lImgCtxCookie)
                            : 0);

    if (pImgCtx && !(pImgCtx->GetState(FALSE, &sizeImg) & IMGLOAD_COMPLETE))
        pImgCtx = NULL;

    // if the background image is not loaded yet and there is no background color
    // return (we dont have anything to draw)
    if(!pImgCtx && !pFF->_ccvBackColor.IsDefined())
        return;

    // now given the rects for a given element
    // draw its background

    // if we have a background image, we need to compute its origin

    lSize = paryRects->Size();

    if(lSize == 0)
        return;

    memset(&bginfo, 0, sizeof(bginfo));

    bginfo.pImgCtx       = pImgCtx;
    bginfo.lImgCtxCookie = pFF->_lImgCtxCookie;
    bginfo.crTrans       = COLORREF_NONE;
    bginfo.crBack        = pFF->_ccvBackColor.IsDefined()
        ? pFF->_ccvBackColor.GetColorRef()
        : COLORREF_NONE;

    if (pImgCtx)
    {
        if(!prcBound)
        {
            // compute the bounding rect for the element.
            BoundingRectForAnArrayOfRects(&rcBound, paryRects);
        }
        else
            rcBound = *prcBound;

        if(!IsRectEmpty(&rcBound))
        {
            SIZE sizeBound;

            sizeBound.cx = rcBound.right - rcBound.left;
            sizeBound.cy = rcBound.bottom - rcBound.top;

            CalcBgImgRect(pFF, pDI, &sizeBound, &sizeImg, &rcImg);
            OffsetRect(&rcImg, rcBound.left + offsetX, rcBound.top + offsetY);

            bginfo.ptBackOrg.x = rcImg.left;
            bginfo.ptBackOrg.y = rcImg.top;
        }
    }

    // splash the background now

    prc = *paryRects;
    rcDraw = *prc++;
    for(iRect = 1; iRect <= lSize; iRect++, prc++)
    {
        if(iRect == lSize || !IsRectEmpty(prc))
        {
            if( iRect!= lSize &&
                prc->left == rcDraw.left &&
                prc->right == rcDraw.right &&
                prc->top == rcDraw.bottom)
            {
                // add the current rect
                rcDraw.bottom = prc->bottom;
            }
            else
            {
                OffsetRect(&rcDraw, offsetX, offsetY);

                IntersectRect(&rcDraw, prcClip, &rcDraw);
                IntersectRect(&bginfo.rcImg, &rcImg, &rcDraw);

                if(!IsRectEmpty(&rcDraw))
                {
                    GetFlowLayout()->DrawBackground(pDI, &bginfo, &rcDraw);
                }

                if(iRect != lSize)
                    rcDraw = *prc;
            }
        }
    }
}

//+----------------------------------------------------------------------------
//
// Function:    DrawElementBorder
//
// Synopsis:    Find the bounding rect that contains a given set of rectangles
//
//-----------------------------------------------------------------------------

void
CDisplay::DrawElementBorder(CTreeNode * pNodeContext,
                                CDataAry <RECT> * paryRects, RECT * prcBound,
                                const RECT * prcView, const RECT * prcClip,
                                CFormDrawInfo * pDI)
{
    CBorderInfo borderInfo;
    CElement *  pElement = pNodeContext->Element();

    if (pNodeContext->GetCharFormat()->IsVisibilityHidden())
        return;

    if ( !pElement->_fDefinitelyNoBorders &&
         FALSE == (pElement->_fDefinitelyNoBorders = !GetBorderInfoHelper(pNodeContext, pDI, &borderInfo, TRUE ) ) )
    {
        RECT rcBound;

        if(!prcBound)
        {
            if(paryRects->Size() == 0)
                return;

            // compute the bounding rect for the element.
            BoundingRectForAnArrayOfRects(&rcBound, paryRects);
        }
        else
            rcBound = *prcBound;

#ifndef DISPLAY_TREE
// BUGBUG: Fix this (brendand)
        if(!_fRTL)
        {
            OffsetRect(&rcBound, prcView->left - GetXScrollEx(),
                prcView->top - GetYScroll());
        }
        else
        {
            OffsetRect(&rcBound, prcView->right + GetXScrollEx(),
                prcView->top - GetYScroll());
        }
#endif

        DrawBorder(pDI, &rcBound, &borderInfo);
    }
}


// =============================  Misc  ===========================================

//+--------------------------------------------------------------------------------
//
// Synopsis: return true if this is the last text line in the line array
//---------------------------------------------------------------------------------
BOOL
CDisplay::IsLastTextLine(LONG ili)
{
    Assert(ili >= 0 && ili < LineCount());

    for(LONG iliT = ili + 1; iliT < LineCount(); iliT++)
    {
        if(Elem(iliT)->IsTextLine())
            return FALSE;
    }
    return TRUE;
}



//+---------------------------------------------------------------------------
//
//  Member:     SetListIndex( re, ili )
//
//  Purpose:    Computes the list index for the current line (index ili) if
//              it is a numbered paragraph.  When the 'current' cache is
//              valid, the index is computed from that.  If it isn't, it
//              attempts to compute it from the 'top' cache, which maintains
//              the valid indicies at the top of the page.  If the 'top'
//              cache is not valid, the value is computed and stored.
//
//  Arguements: re is the current CRenderer, from which we get a CRichTxtPtr
//              ili is the current line index
//
//  Returns:    The new numbering index.
//
//----------------------------------------------------------------------------

HRESULT
CDisplay::SetListIndex(
    LONG cpIn,
    LONG ili )
{
    CElement    *pElementFL = GetFlowLayout()->ElementContent();
    const        CParaFormat * pPF;
    BOOL         fInner;
    HRESULT      hr = S_OK;
    struct       CListIndex LI;
    CLinePtr     rp(this);
    WORD         wLevel;
    BOOL         fContinue;
    CListing     Listing;
    CTreeNode   *pNode;

    // If we don't already have a list cache, make sure to cook one up.
    // Note that once you cook one up, it doesn't die until the display does.

    hr = THR( EnsureListCache() );
    if (hr)
        goto Cleanup;

    rp.RpSet( ili, 0 );

    pNode = FormattingNodeForLine(cpIn, NULL, rp->_cch, NULL, NULL);
    pPF = pNode->GetParaFormat();
    fInner = SameScope(pNode, pElementFL);

    Listing = pPF->GetListing(fInner);

    AssertSz( Listing.GetType() != CListing::NONE,
              "We should be in some kind of list here, dudes." );

    LI.lValue = 0;
    LI.style = styleListStyleTypeNotSet;

    wLevel = Listing.GetLevel();

    if (rp->_fNewList)
    {
        LI.lValue = pPF->GetNumberingStart(fInner);
        LI.style = Listing.GetStyle();

        // If this is a naked LI, make it a simple bullet.
        if (LI.style == styleListStyleTypeNotSet)
        {
            LI.style = styleListStyleTypeDisc;
        }
    }
    else if (CListCache::Valid( wLevel, INDEXCACHE_CURRENT ))
    {
        // If the current cache is valid, all we need to do is
        // either get the value in the PF or increment the 'current'
        // index cache.

        LI = CListCache::GetAt( wLevel, INDEXCACHE_CURRENT );

        if (Listing.IsValueValid())
        {
            LI.lValue = pPF->GetNumberingStart(fInner);
        }
        else
        {
            LI.lValue += 1L;
        }

        if (Listing.IsStyleValid())
        {
            LI.style = Listing.GetStyle();
        }
    }
    else
    {
        // The current cache is not valid: If the top cache is valid,
        // we need to count up to that.  If the top cache is not valid,
        // we need to count to the top of the list.  If we count to the
        // top of the list, we can then set the top cache.

        const LONG iliPreFirstVisibleLine = GetFirstVisibleLine() - 1;
        const BOOL fTopCacheGood = CListCache::Valid( wLevel,
            INDEXCACHE_TOP );
        LONG lCacheOffset = 1;
        BOOL fValueFound = FALSE;
        BOOL fStyleFound = FALSE;
        BOOL fReachedTop = FALSE;
        struct CListIndex LITop;

        do
        {
            if (iliPreFirstVisibleLine == rp.GetLineIndex())
            {
                fReachedTop = TRUE;

                // If the 'top' cache is good, we need not go further.
                if (fTopCacheGood)
                {
                    LITop = CListCache::GetAt( wLevel, INDEXCACHE_TOP );
                    LI.lValue += LITop.lValue;
                    if (!fStyleFound)
                    {
                        LI.style = LITop.style;
                    }
                    break;
                }
                else
                {
                    LITop = LI;
                }

            }

            if (rp->_fHasBulletOrNum || rp->_fNewList)
            {
                pNode = FormattingNodeForLine(cpIn, NULL, rp->_cch, NULL, NULL);
                pPF = pNode->GetParaFormat();
                fInner = SameScope(pNode, pElementFL);

                Listing = pPF->GetListing(fInner);

                WHEN_DBG( DWORD w = rp->_dwFlags() );

                if (Listing.GetType() != CListing::NONE || rp->_fNewList)
                {
                    WORD wCurLevel = Listing.GetLevel();

                    AssertSz( wCurLevel >= wLevel,
                              "Levels screwed up. Missing fliNewList?" );

                    if (wCurLevel == wLevel)
                    {
                        if (!fValueFound)
                        {
                            if ((rp->_fNewList) ||
                                (Listing.IsValueValid()))
                            {
                                LI.lValue += pPF->GetNumberingStart(fInner);
                                if (!rp->_fHasBulletOrNum)
                                {
                                    LI.lValue--;
                                }
                                fValueFound = TRUE;
                            }
                            else
                            {
                                LI.lValue += 1L;
                            }
                        }
                        if (!fStyleFound &&
                            (rp->_fNewList || Listing.IsStyleValid()))
                        {
                            LI.style = Listing.GetStyle();
                            fStyleFound = TRUE;
                        }
                    }
                }
            }

            // We want to point to the beginning of the previous line.
            fContinue = rp.PrevLine(TRUE, FALSE);
            cpIn -= rp->_cch;

        } while ((!fValueFound || !fStyleFound) && fContinue);

        // If the 'top' cache previously was invalid, we are now
        // prepared to validate it.

        if (!fTopCacheGood && fReachedTop && fValueFound && fStyleFound)
        {
            // the entry to the cache should be less than our new
            // value by the lCacheOffset amount, which we remember
            // from traversing to the top of the list.

            LITop.lValue = LI.lValue - LITop.lValue;
            LITop.style = LI.style;
            hr = CListCache::Set( wLevel, &LITop, INDEXCACHE_TOP );
        }
    }

    hr = CListCache::Set( wLevel, &LI, INDEXCACHE_CURRENT );

Cleanup:

    RRETURN(hr);
}

//+---------------------------------------------------------------------------
//
//  Member:     ResyncListIndex
//
//  Purpose:    It is expensive to recompute the list index cache when long
//              ordered lists are created, if we need to do this from scratch.
//              This function attempts to update the cache after a scrolling
//              operation.
//
//----------------------------------------------------------------------------

void
CDisplay::ResyncListIndex(
    long    yScroll)
{
    CElement *pElementFL = GetFlowLayout()->ElementContent();

    long        lDelta;
    CListing    Listing;
    HRESULT     hr;

    // No lists so far in the document, don't bother.

    if (!CListCache::Instantiated())
        return;

    if (CListCache::IsCurrent(yScroll))
        return;

    lDelta = yScroll - CListCache::GetYScroll();

    // If the cache for the old first visible line was empty, don't bother
    // trying to cache for the new first visible line.

    if (!CListCache::Valid(1, INDEXCACHE_TOP))
        goto Clear;
    else
    {
        CLinePtr           rp(this);
        CRect              rc;
        const CParaFormat *pPF;
        BOOL               fInner;
        WORD               wLevel;
        WORD               wLastLevel = 0;
        LONG               ili, iliStart, iliStop, iDirection;
        CListIndex         LI;
        BOOL               fStyleFound = FALSE;
        LONG               cpLI;
        CTreeNode         *pNode;

        GetFlowLayout()->GetClientRect(&rc);

        rc.MoveTo(0, CListCache::GetYScroll());
        iliStart   = LineFromPos(rc, LFP_IGNOREALIGNED);

        rc.MoveTo(0, yScroll);
        iliStop    = LineFromPos(rc, LFP_IGNOREALIGNED);

        iDirection = (yScroll < CListCache::GetYScroll()
                        ? -1
                        : 1);

        CListCache::SetYScroll(yScroll);

        cpLI = CpFromLine(iliStart, NULL);
        rp.RpSet( iliStart, 0 );

        // Here's the hack.  If were going up, we need to know the 'level'
        // of the old first visible, as all levels higher are invalid.

        if (iDirection < 0)
        {
            WORD wLevelStop = CListCache::Depth(INDEXCACHE_TOP);

            pNode = FormattingNodeForLine(cpLI, NULL, rp->_cch, NULL, NULL);
            pPF = pNode->GetParaFormat();
            fInner = SameScope(pNode, pElementFL);

            Listing = pPF->GetListing(fInner);

            if ( Listing.HasAdornment() || rp->_fNewList )
            {
                wLevel = Listing.GetLevel();

                // If the last visible listitem had a GETVALUE, then
                // that level is also now invalid.

                if ((rp->_fNewList) ||
                    (Listing.IsValueValid() ||
                     Listing.IsStyleValid()))
                {
                    --wLevel;
                }
            }
            else
            {
                wLevel = 0;
            }

            // If we're clearing the entire cache, don't bother trying to
            // make it correct.

            if (wLevel == 0)
                goto Clear;

            // Invalidate all entries greater than wLevel.  Note that we loop
            // <= Count() because the array is 0-based but levels are 1-based.

            while (++wLevel <= wLevelStop)
            {
                CListCache::Invalidate( wLevel, INDEXCACHE_TOP );
            }

            // Advance (or is it retreat?)  Note we're adjusting iliStart.

            rp.SetRun( --iliStart, 0 );
            cpLI -= rp->_cch;
        }

        for (ili = iliStart; ili != iliStop;)
        {
            if (rp->_fHasBulletOrNum || rp->_fNewList)
            {
                pNode = FormattingNodeForLine(cpLI, NULL, rp->_cch, NULL, NULL);
                pPF = pNode->GetParaFormat();
                fInner = SameScope(pNode, pElementFL);

                Listing = pPF->GetListing(fInner);

                // If we've run out of number/bullet list paragraphs, don't
                // bother continuing.

                if ( Listing.HasAdornment() )
                    goto Clear;

                wLevel = Listing.GetLevel();

                if (wLevel != wLastLevel)
                {
                    fStyleFound = FALSE;
                    wLastLevel = wLevel;
                }

                // If we encounter a PFNF_GETSTYLE, we know we can
                // validate a cache entry as soon as we know the VALUE.

                if ((rp->_fNewList) ||
                    (Listing.IsStyleValid()))
                {
                    LI.style = Listing.GetStyle();
                    fStyleFound = TRUE;
                }

                // If we encounter a PFNF_GETVALUE, we should go ahead and
                // stash it in the cache.  If we don't, we can only dec-
                // rement the cache value if we knew it's previous value.

                if ((rp->_fNewList) ||
                    (Listing.IsValueValid()))
                {
                    // If we encounter the beginning of a list, and we're
                    // scrolling up, we will no longer have a valid cache
                    // entry, since a new list may start above it and I
                    // can't know its proper index without traversing
                    // further up.

                    if (iDirection < 0 && rp->_fNewList)
                    {
                        if (wLevel <= CListCache::Depth(INDEXCACHE_TOP))
                        {
                            CListCache::Invalidate( wLevel, INDEXCACHE_TOP );
                        }
                        fStyleFound = FALSE;
                    }
                    else
                    {
                        LI.lValue = pPF->GetNumberingStart(fInner);

                        // We have value, if we can find a valid style,
                        // we can validate the top cache.  On the other

                        if (fStyleFound ||
                            CListCache::Valid( wLevel, INDEXCACHE_TOP ))
                        {
                            if (!fStyleFound)
                            {
                                LI.style = CListCache::GetAt(wLevel,INDEXCACHE_TOP).style;
                            }

                            hr = THR(CListCache::Set(wLevel, &LI, INDEXCACHE_TOP) );
                            if (hr)
                                goto Clear;
                        }
                    }
                }
                else
                {
                    if (CListCache::Valid( wLevel, INDEXCACHE_TOP ))
                    {
                        if (!fStyleFound)
                        {
                            LI.style = CListCache::GetAt(wLevel, INDEXCACHE_TOP).style;
                        }

                        LI.lValue = CListCache::GetAt(wLevel, INDEXCACHE_TOP).lValue
                                    + iDirection;
                        hr = THR( CListCache::Set( wLevel, &LI, INDEXCACHE_TOP) );
                        if (hr)
                            goto Clear;
                    }
                }
            }

            // Move the CLinePtr and the CRchTxtPtr.

            if (iDirection > 0)
            {
                cpLI += rp->_cch;
                rp.SetRun( ++ili, 0 );
            }
            else
            {
                rp.SetRun( --ili, 0 );
                cpLI -= rp->_cch;
            }
        }
    }

    return;

Clear:
    ClearListIndexCache();
    return;
}

//+---------------------------------------------------------------------------
//
//  Member:     FormattingNodeForLine
//
//  Purpose:    Returns the node which controls the formatting at the BOL. This
//              is needed because the first char on the line may not necessarily
//              be the first char in the paragraph.
//
//----------------------------------------------------------------------------
CTreeNode *
CDisplay::FormattingNodeForLine(
    LONG         cpForLine,    // IN
    CTreePos    *ptp,          // IN
    LONG         cchLine,      // IN
    LONG        *pcch,         // OUT
    CTreePos   **pptp) const   // OUT
{
    CFlowLayout  *pFlowLayout = GetFlowLayout();
    BOOL          fIsEditable = pFlowLayout->IsEditable();
    CElement     *pElementFL  = pFlowLayout->ElementContent();
    CMarkup      *pMarkup     = pFlowLayout->GetContentMarkup();
    CTreeNode    *pNode       = NULL;
    CElement     *pElement;
    LONG          lNotNeeded;
    LONG          cch = cchLine;
    BOOL          fSawOpenLI  = FALSE;

    if (!ptp)
    {
        ptp = pMarkup->TreePosAtCp(cpForLine, &lNotNeeded);
        Assert(ptp);
    }
    else
    {
        Assert(ptp->GetCp() <= cpForLine);
        Assert(ptp->IsPointer() || ptp->GetCp() + ptp->GetCch() > cpForLine);
    }
    while(cch)
    {
        if (ptp->IsPointer())
        {
            ptp = ptp->NextTreePos();
            continue;
        }

        if (ptp->IsText())
            break;

        Assert(ptp->IsNode());

        if (fIsEditable && ptp->ShowTreePos())
            break;

        pNode = ptp->Branch();
        pElement = pNode->Element();

        if (ptp->IsBeginElementScope())
        {
            const CCharFormat *pCF = pNode->GetCharFormat();
            if (pCF->IsDisplayNone())
            {
                pElement->GetTreeExtent(NULL, &ptp);
                cch -= pElement->GetElementCch() + 1;
            }
            else if (pNode->HasLayout())
            {
                break;
            }
            else if (pElement->Tag() == ETAG_BR)
            {
                break;
            }
            else if (pElement->Tag() == ETAG_LI)
            {
                fSawOpenLI = TRUE;
            }
        }
        else if (ptp->IsEndNode())
        {
            if (   fSawOpenLI                    // Skip over the close LI, unless we saw an open LI,
                && ptp->IsEdgeScope()            // which would imply that we have an empty LI.  An
                && pElement->Tag() == ETAG_LI    // empty LI gets a bullet, so we need to break.
               )
                break;
            pNode = pNode->Parent();
        }

        cch--;
        ptp = ptp->NextTreePos();
    }

    if (pcch)
    {
        *pcch = cchLine - cch;
    }

    if (pptp)
    {
        *pptp = ptp;
    }

    if (!pNode)
    {
        pNode = ptp->GetBranch();
        if (ptp->IsEndNode())
            pNode = pNode->Parent();
    }

    return pNode;
}

long
ComputeLineShift(htmlAlign  atAlign,
                 BOOL       fRTLDisplay,
                 BOOL       fRTLLine,
                 BOOL       fMinMax,
                 long       xWidthMax,
                 long       xWidth,
                 UINT *     puJustified)
{
    long xShift = 0;

    switch(atAlign)
    {
    case htmlAlignNotSet:
        if(fRTLDisplay == fRTLLine)
            *puJustified = JUSTIFY_LEAD;
        else
            *puJustified = JUSTIFY_TRAIL;
        break;

    case htmlAlignLeft:
        if(!fRTLDisplay)
        {
            *puJustified = JUSTIFY_LEAD;
        }
        else
        {
            *puJustified = JUSTIFY_TRAIL;
        }
        break;

    case htmlAlignRight:
        if(!fRTLDisplay)
        {
            *puJustified = JUSTIFY_TRAIL;
        }
        else
        {
            *puJustified = JUSTIFY_LEAD;
        }
        break;

    case htmlAlignCenter:
        *puJustified = JUSTIFY_CENTER;
        break;

    case htmlBlockAlignJustify:
        *puJustified = JUSTIFY_FULL;
        break;

    default:
        AssertSz(FALSE, "Did we introduce new type of alignment");
        break;
    }
    if (    !fMinMax
        &&  *puJustified
        &&  *puJustified != JUSTIFY_FULL)
    {
        // for pre whitespace is already include in _xWidth
        xShift = xWidthMax - xWidth;

        xShift = max(xShift, 0L);           // Don't allow alignment to go < 0
                                            // Can happen with a target device
        if(atAlign == htmlAlignCenter)
            xShift /= 2;
    }

    return xShift;
}
