@echo off
cd %~dp0
echo "working"
set githash=fatal
FOR /F "usebackq tokens=* delims=" %%I IN (`git.exe rev-parse --short^=16 HEAD`) DO (
	SET githash=%%I
)

echo #pragma once > ".\src\gitinfo.h"
REM echo #ifdef _DEBUG >> ".\src\gitinfo.h"

SET fatalStr=fatal
ECHO %githash% | FINDSTR /C:"%fatalStr%" >NUL

IF %ERRORLEVEL%==0 (
	echo static const wchar_t GIT_HASH[] = L"\0"; >> ".\src\gitinfo.h"
) ELSE (
	echo static const wchar_t GIT_HASH[] = L"%githash%"; >> ".\src\gitinfo.h"
)

REM echo #else >> ".\src\gitinfo.h"

REM set githash=fatal
REM FOR /F "usebackq tokens=* delims=" %%I IN (`git.exe rev-parse --abbrev-ref HEAD`) DO (
REM 	SET githash=%%I
REM )

REM SET fatalStr=fatal
REM ECHO %githash% | FINDSTR /C:"%fatalStr%" >NUL

REM IF %ERRORLEVEL%==0 (
REM 	echo static const wchar_t GIT_HASH[] = L"\0"; >> ".\src\gitinfo.h"
REM ) ELSE (
REM 	echo static const wchar_t GIT_HASH[] = L"%githash%"; >> ".\src\gitinfo.h"
REM )

REM echo #endif // _DEBUG >> ".\src\gitinfo.h"