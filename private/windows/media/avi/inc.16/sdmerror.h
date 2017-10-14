/***MD sdmerror.h - System Detection Manager Error Codes
 *
 *  This module contains System Detection Manager error codes.
 *
 *  Copyright (c) 1992,1993 Microsoft Corporation
 *  Author:	Michael Tsang (MikeTs)
 *  Created	03/14/94
 *
 *  MODIFICATION HISTORY
 */


#ifndef _INC_SDERROR
#define _INC_SDERROR


/*** System Detection Manager Errors (return values)
 */

#define IDSERR_NONE		0		//no error (success)
#define IDSERR_INVALID_PARAM	0x0001		//invalid service parameter
#define IDSERR_OUTOFMEM 	0x0002		//out of memory
#define IDSERR_REG_ACCESS	0x0003		//error accessing registry
#define IDSERR_SD_ALREADYOPEN	0x0004		//system detection already open
#define IDSERR_SD_NOTOPEN	0x0005		//system detection not yet open
#define IDSERR_HOOK_R0CODE	0x0006		//fail to hook Ring0 code
#define IDSERR_DUPINF		0x0007		//inf already registered
#define IDSERR_OPENINF		0x0008		//cannot open inf file
#define IDSERR_INF_LISTSECT	0x0009		//cannot find inflist section
#define IDSERR_INF_LISTLINE	0x000a		//invalid inflist line
#define IDSERR_INF_MODSECT	0x000b		//cannot find module section
#define IDSERR_INF_MODLINE	0x000c		//invalid module line
#define IDSERR_INF_MODNAME	0x000d		//module name too long
#define IDSERR_INF_FUNCSECT	0x000e		//cannot find function section
#define IDSERR_INF_FUNCLINE	0x000f		//invalid function line
#define IDSERR_LOADMOD		0x0010		//error loading module
#define IDSERR_INF_CLASSNAME	0x0011		//inf err in class name
#define IDSERR_INF_DANGERSECT	0x0012		//cannot find danger section
#define IDSERR_INF_DANGERLINE	0x0013		//invalid danger line
#define IDSERR_INF_DANGERFORMAT 0x0014		//invalid danger format
#define IDSERR_DANGER_INVALID	0x0015		//invalid danger regions
#define IDSERR_IOMEM_INVALID	0x0016		//invalid iomem regions
#define IDSERR_IOMEM_CONFLICT	0x0017		//iomem resource conflict
#define IDSERR_IRQDMA_INVALID	0x0018		//invalid irqdma resources
#define IDSERR_IRQDMA_CONFLICT	0x0019		//irqdma resource conflict
#define IDSERR_INF_BUSTYPE	0x001a		//invalid detection bus type
#define IDSERR_DETFUNC_DMSERR	0x001b		//DMS error in detection
#define IDSERR_DETFUNC_REGERR	0x001c		//reg error in detection code
#define IDSERR_DETFUNC_ERROR	0x001d		//detection function error
#define IDSERR_GENINSTALL	0x001e		//GenInstallEx error
#define IDSERR_NOSUCH_DEV	0x001f		//device not found
#define IDSERR_BUFF_TOOSMALL	0x0020		//buffer too small
#define IDSERR_DEV_ALREADYREG	0x0021		//device ID already registered
#define IDSERR_NOT_EISA 	0x0022		//not an EISA system
#define IDSERR_GETEISACONFIG	0x0023		//error getting EISA config
#define IDSERR_INF_CLASS	0x0024		//error in getting INF class
#define IDSERR_COPYLOGCONFIG	0x0025		//error in copying logconfig
#define IDSERR_DUPFUNC		0x0026		//function already registered
#define IDSERR_NOT_MCA		0x0027		//not a MCA system
#define IDSERR_ABORT		0x0028		//detection is aborted
#define IDSERR_GETFUNCADDR	0x0029		//fail to get function entry
#define IDSERR_HOOKINT86X	0x002a		//fail to hook Int86x worker
#define IDSERR_QUERYDOSDEV	0x002b		//fail to query dos dev info.
#define IDSERR_INF_RISKCAT	0x002c		//invalid risk category
#define IDSERR_NODOSDEV 	0x002d		//no such dos device
#define IDSERR_IOMEM_NOTFOUND	0x002e		//iomem block not found
#define IDSERR_IRQDMA_NOTFOUND	0x002f		//irqdma block not found
#define IDSERR_OPENCRASHLOG	0x0030		//failed to open detect log
#define IDSERR_NODOSTSR 	0x0031		//no such dos TSR
#define IDSERR_REGHW_DUP	0x0032		//duplicate hw registration
#define IDSERR_CALLRMPROC	0x0033		//call real mode proc error
#define IDSERR_NEEDREBOOT	0x0034		//need reboot after detection

#define SDMERR_NONE		IDSERR_NONE
#define SDMERR_INVALID_PARAM	(0x80000000 | IDSERR_INVALID_PARAM)
#define SDMERR_OUTOFMEM 	(0x80000000 | IDSERR_OUTOFMEM)
#define SDMERR_REG_ACCESS	(0x80000000 | IDSERR_REG_ACCESS)
#define SDMERR_SD_ALREADYOPEN	(0x80000000 | IDSERR_SD_ALREADYOPEN)
#define SDMERR_SD_NOTOPEN	(0x80000000 | IDSERR_SD_NOTOPEN)
#define SDMERR_HOOK_R0CODE	(0x80000000 | IDSERR_HOOK_R0CODE)
#define SDMERR_DUPINF		(0x80000000 | IDSERR_DUPINF)
#define SDMERR_OPENINF		(0x80000000 | IDSERR_OPENINF)
#define SDMERR_INF_LISTSECT	(0x80000000 | IDSERR_INF_LISTSECT)
#define SDMERR_INF_LISTLINE	(0x80000000 | IDSERR_INF_LISTLINE)
#define SDMERR_INF_MODSECT	(0x80000000 | IDSERR_INF_MODSECT)
#define SDMERR_INF_MODLINE	(0x80000000 | IDSERR_INF_MODLINE)
#define SDMERR_INF_MODNAME	(0x80000000 | IDSERR_INF_MODNAME)
#define SDMERR_INF_FUNCSECT	(0x80000000 | IDSERR_INF_FUNCSECT)
#define SDMERR_INF_FUNCLINE	(0x80000000 | IDSERR_INF_FUNCLINE)
#define SDMERR_LOADMOD		(0x80000000 | IDSERR_LOADMOD)
#define SDMERR_INF_CLASSNAME	(0x80000000 | IDSERR_INF_CLASSNAME)
#define SDMERR_INF_DANGERSECT	(0x80000000 | IDSERR_INF_DANGERSECT)
#define SDMERR_INF_DANGERLINE	(0x80000000 | IDSERR_INF_DANGERLINE)
#define SDMERR_INF_DANGERFORMAT (0x80000000 | IDSERR_INF_DANGERFORMAT)
#define SDMERR_DANGER_INVALID	(0x80000000 | IDSERR_DANGER_INVALID)
#define SDMERR_IOMEM_INVALID	(0x80000000 | IDSERR_IOMEM_INVALID)
#define SDMERR_IOMEM_CONFLICT	(0x80000000 | IDSERR_IOMEM_CONFLICT)
#define SDMERR_IRQDMA_INVALID	(0x80000000 | IDSERR_IRQDMA_INVALID)
#define SDMERR_IRQDMA_CONFLICT	(0x80000000 | IDSERR_IRQDMA_CONFLICT)
#define SDMERR_INF_BUSTYPE	(0x80000000 | IDSERR_INF_BUSTYPE)
#define SDMERR_DETFUNC_DMSERR	(0x80000000 | IDSERR_DETFUNC_DMSERR)
#define SDMERR_DETFUNC_REGERR	(0x80000000 | IDSERR_DETFUNC_REGERR)
#define SDMERR_DETFUNC_ERROR	(0x80000000 | IDSERR_DETFUNC_ERROR)
#define SDMERR_GENINSTALL	(0x80000000 | IDSERR_GENINSTALL)
#define SDMERR_NOSUCH_DEV	(0x80000000 | IDSERR_NOSUCH_DEV)
#define SDMERR_BUFF_TOOSMALL	(0x80000000 | IDSERR_BUFF_TOOSMALL)
#define SDMERR_DEV_ALREADYREG	(0x80000000 | IDSERR_DEV_ALREADYREG)
#define SDMERR_NOT_EISA 	(0x80000000 | IDSERR_NOT_EISA)
#define SDMERR_GETEISACONFIG	(0x80000000 | IDSERR_GETEISACONFIG)
#define SDMERR_INF_CLASS	(0x80000000 | IDSERR_INF_CLASS)
#define SDMERR_COPYLOGCONFIG	(0x80000000 | IDSERR_COPYLOGCONFIG)
#define SDMERR_DUPFUNC		(0x80000000 | IDSERR_DUPFUNC)
#define SDMERR_NOT_MCA		(0x80000000 | IDSERR_NOT_MCA)
#define SDMERR_ABORT		(0x80000000 | IDSERR_ABORT)
#define SDMERR_GETFUNCADDR	(0x80000000 | IDSERR_GETFUNCADDR)
#define SDMERR_HOOKINT86X	(0x80000000 | IDSERR_HOOKINT86X)
#define SDMERR_QUERYDOSDEV	(0x80000000 | IDSERR_QUERYDOSDEV)
#define SDMERR_INF_RISKCAT	(0x80000000 | IDSERR_INF_RISKCAT)
#define SDMERR_NODOSDEV 	(0x80000000 | IDSERR_NODOSDEV)
#define SDMERR_IOMEM_NOTFOUND	(0x80000000 | IDSERR_IOMEM_NOTFOUND)
#define SDMERR_IRQDMA_NOTFOUND	(0x80000000 | IDSERR_IRQDMA_NOTFOUND)
#define SDMERR_OPENCRASHLOG	(0x80000000 | IDSERR_OPENCRASHLOG)
#define SDMERR_NODOSTSR 	(0x80000000 | IDSERR_NODOSTSR)
#define SDMERR_REGHW_DUP	(0x80000000 | IDSERR_REGHW_DUP)
#define SDMERR_CALLRMPROC	(0x80000000 | IDSERR_CALLRMPROC)
#define SDMERR_NEEDREBOOT	(0x80000000 | IDSERR_NEEDREBOOT)

#endif	//_INC_SDERROR
