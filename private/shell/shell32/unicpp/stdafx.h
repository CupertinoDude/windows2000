// Precompiled header for UNICPP

#ifndef _UNICPP_PCH__
#define _UNICPP_PCH__

#ifdef WINNT
extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <ntexapi.h>
}
#endif

#undef STRICT

#include <shellprv.h>

#include <shlobj.h>

#include <shlwapi.h>
#include <shellp.h>
#include <shellids.h>
#include <shguidp.h>
#include <shlwapip.h>
#include <shsemip.h>
#include <desktopp.h>

#include <InetReg.h>
#include <cplext.h>
#include <dbt.h>        
#include <devioctl.h>
#include <fsmenu.h>
#include <hliface.h>
#include <iethread.h>
#include <inetreg.h>
#include <intshcut.h>
#include <mmhelper.h>
#include <mshtmdid.h>
#include <mshtml.h>

#include <multimon.h>
#include <objsafe.h>
#include <oleauto.h>
#include <olectl.h>

#include <regstr.h>
#include <stdarg.h>
#include <stdio.h>
#include <trayp.h>
#include <urlmon.h>
#include <webcheck.h>

#include "debug.h"
#include "shellp.h"
#include "shlguid.h"
#include "shguidp.h"
#include "shobjprv.h"

#include "local.h"
#include "deskstat.h"
#include "dutil.h"

#include "admovr2.h"
#include "advanced.h"
#include "clsobj.h"
#include "cmdfile.h"
#include "dback.h"
#include "dbackp.h"
#include "dcomp.h"
#include "dcompp.h"
#include "deskcls.h"
#include "deskhtm.h"
#include "deskhtml.h"
#include "deskstat.h"
#include "desktop.h"
#include "dsubscri.h"
#include "dutil.h"
#include "editpat.h"
#include "expdsprt.h"
#include "hnfblock.h"
#include "local.h"
#include "mruex.h"
#include "msstkppg.h"
#include "options.h"
#include "pattern.h"
#include "resource.h"
#include "schedule.h"
#include "sdspatch.h"
#include "utils.h"
#include "comcat.h"
#include "netview.h"
#include "ids.h"
#include "fldset.h"
#include "recdocs.h"
#include "brfcasep.h"
#include "startids.h"
#include "defview.h"
#include "htmlhelp.h"
#include "uemapp.h"
#include "expdsprt.h"

// The W version of this API has been implemented in shlwapi, so we save code
// and use that version.  If we include w95wraps.h we'll get this definition
// for us, but shell32 isn't single binary yet so we don't use it.
#define ShellMessageBoxW ShellMessageBoxWrapW

#endif
