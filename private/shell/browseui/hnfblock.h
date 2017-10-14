#ifndef HNFBLOCK_H_
#define HNFBLOCK_H_

#include <iethread.h>

DECLARE_HANDLE(HNFBLOCK);

STDAPI_(HNFBLOCK) ConvertNFItoHNFBLOCK(IETHREADPARAM* pInfo, LPCTSTR pszPath, DWORD dwProcId);
STDAPI_(IETHREADPARAM *) ConvertHNFBLOCKtoNFI(HNFBLOCK hBlock);


#endif // HNFBLOCK_H_
