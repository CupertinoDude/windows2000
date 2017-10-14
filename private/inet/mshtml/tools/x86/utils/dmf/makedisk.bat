@echo off

IF '%1'=='' GOTO NO_PARAM

ECHO �
ECHO NOTE: MakeDisk.bat generates DMF formatted floppies only.
ECHO       Any floppies formatted 1.44 should be copied seperately.
ECHO �
ECHO WARNING!: Floppies may not be requested in numerical order.
ECHO �

FOR %%I IN (DISK*.DMF) DO CALL image.bat %%I %1
GOTO DONE

:NO_PARAM
ECHO �
ECHO Usage: "makedisk <drive_letter>:"
ECHO          e.g. makedisk A:
ECHO �
GOTO DONE

:DONE
ECHO �

