@echo off

WHERE cl >nul 2>nul
IF %ERRORLEVEL% EQU 0 (GOTO clfound)
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
:clfound

set ReleaseOBJ= -O2 -Zi -Zo -Gm-
set Release= -O2 -Zo
set DebugPDB= -Od -Zi -Gm
set DebugOBJ= -Od -Z7

set Current= %Release%
rem set Current= %DebugOBJ%
set KHLIB="..\src\lib"
set ASSIMPDIR="w:\tools\assimp"

set STBWarnings= -wd4244 -wd4701 -wd4703
set NoErrors= -wd4505 -wd4201 -wd4100 -wd4189 -wd4127 %STBWarnings%
set CompilerOptions= %Current% -MTd -nologo -Gm- -GR- -EHa- -EHsc -Oi -fp:fast -fp:except- -WX -W4 -FC %NoErrors%
set LinkerOptions= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib %ASSIMPDIR%\lib64\assimp.lib -DEBUG

IF NOT EXIST .\..\..\build mkdir .\..\..\build
cd .\..\..\build

IF NOT EXIST %ASSIMPDIR% GOTO END

cl %CompilerOptions% -I%KHLIB% -I%ASSIMPDIR%\include -D_CRT_SECURE_NO_WARNINGS /Fepack_asset.exe .\..\tools\asset_packaging\kh_asset_packaging.cpp /link %LinkerOptions%

:END

cd .\..\tools\asset_packaging\