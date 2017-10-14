# Microsoft Visual C++ generated build script - Do not modify

PROJ = panmap
DEBUG = 1
PROGTYPE = 4
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG -dWIN32
R_RCDEFINES = -dNDEBUG -dWIN32
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = \coremap
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = ELSECVT1.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_LIB = /nologo /W3 /Zi /YX /Od /D_DEBUG /DWIN32 /DWINDOWS /MT /D_X86_ /FR /Fp"ELSELIB.PCH"
CFLAGS_R_LIB = /nologo /W3 /Zi /Ox /DWIN32 /DWINDOWS /D_X86_ /FR /Fp"ELSELIB.PCH"
LFLAGS_D_LIB = /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:windows  /MAP
LFLAGS_R_LIB = /SUBSYSTEM:windows /MAP
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = ole32.lib uuid.lib
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_LIB)
LFLAGS = $(LFLAGS_D_LIB)
LIBS = 
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_LIB)
LFLAGS = $(LFLAGS_R_LIB)
LIBS = 
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
# SBRS = ELSECVT1.SBR \
# 		ELSECVT2.SBR \
# 		ELSECVT3.SBR \
# 		ELSECVT4.SBR \
# 		ELSEMAP.SBR \
# 		ELSERES.SBR


PAN1PTBL_DEP = elsepan.h elseuser.h elsetype.h

PAN1PTBL_H_DEP = PAN1PTBL.EXE PAN1PTBL.TXT

ELSEPAN_DEP = $(PAN1PTBLE_DEP) PAN1PTBL.H


all:	$(PROJ).DLL pan1ptbl.exe

PAN1PTBL.EXE:	PAN1PTBL.C $(PAN1PTBL_DEP)
	$(CC) $(CFLAGS) PAN1PTBL.C

ELSEPAN.OBJ:	ELSEPAN.C $(ELSEPAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ELSEPAN.C

PAN1PTBL.H:	PAN1PTBL.EXE $(PAN1PTBL_H_DEP)
	PAN1PTBL PAN1PTBL.TXT pan1ptbl.h

panmap.OBJ:	panmap.CPP $(PAN1PTBL_DEP) panmap.H
	$(CC) $(CFLAGS) /c panmap.Cpp

panmap.RES:    panmap.RC ver.rc 
        $(RC) $(RCDEFINES) panmap.RC

$(PROJ).DLL::	ELSEPAN.OBJ PAN1PTBL.EXE PAN1PTBL.H panmap.obj \
                panmap.res panmap.def
	echo >NUL @<<$(PROJ).CRF
ELSEPAN.OBJ
panmap.OBJ
$(OBJS_EXT)
$(LIBS_EXT)
$(LIBS)
$(PROJ).RES
-DEF:$(PROJ).DEF
-DLL
-MACHINE:ix86
-DEBUGTYPE:cv
-OUT:$@
<<
	if exist $@ del $@
	link $(LFLAGS) @$(PROJ).CRF

$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<

