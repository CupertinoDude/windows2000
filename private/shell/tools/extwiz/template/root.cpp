// $$root$$.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f ShellExtensionsps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "$$root$$.h"

#include "ShellExtensions_i.c"
$$IF(Icon)
#include "$$ClassType$$EI.h"
$$ENDIF
$$IF(ContextMenu)
#include "$$ClassType$$CM.h"
$$ENDIF
$$IF(PropertySheet)
#include "$$ClassType$$PS.h"
$$ENDIF
$$IF(InfoTip)
#include "$$ClassType$$IT.h"
$$ENDIF


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
$$IF(Icon)
	OBJECT_ENTRY(CLSID_$$ClassType$$EI, C$$ClassType$$EI)
$$ENDIF
$$IF(ContextMenu)
	OBJECT_ENTRY(CLSID_$$ClassType$$CM, C$$ClassType$$CM)
$$ENDIF
$$IF(PropertySheet)
	OBJECT_ENTRY(CLSID_$$ClassType$$PS, C$$ClassType$$PS)
$$ENDIF
$$IF(InfoTip)
	OBJECT_ENTRY(CLSID_$$ClassType$$IT, C$$ClassType$$IT)
$$ENDIF
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}


