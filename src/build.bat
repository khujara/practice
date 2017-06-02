@echo off

rem ctime -begin practice.ctm
rem echo tg

WHERE cl >nul 2>nul
IF %ERRORLEVEL% EQU 0 (GOTO clfound)
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

:clfound

rem Ox ??
set ReleaseOBJ= -O2 -Zi -Zo -Gm-DKH_IN_DEVELOPMENT=1-
set Release= -O2 -Zo
set DebugPDB= -Od -Zi -Gm
set DebugOBJ= -Od -Z7
rem -Z7
rem -MTd
rem -EHsc

set KHLIB="..\src\lib"

set Current= %Release%
rem set Current= %ReleaseOBJ%
rem set Current= %DebugPDB%
rem set Current= %DebugOBJ%

rem set LinkerASM= kh_asm.obj

set CompilerOptions= %Current% -nologo -MP -MTd -EHa- -EHsc -Oi /FAcs -fp:fast -WX -W4  -FC
set NoErrors= -wd4505 -wd4201 -wd4100 -wd4189 -wd4127 -wd4244 -wd4101 -wd4577
set CompilerOptions= %CompilerOptions% %NoErrors%%
set LinkerOptions= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib -DEBUG
set CopyFiles=0

IF NOT EXIST .\..\build\ mkdir .\..\build
cd .\..\build
del *.pdb > NUL 2> NUL

set EntryFunc= /subsystem:windows /entry:WinMainCRTStartup

set SampleBegin=..\src\samples\kh_
set SampleEnd=_sample.cpp

set Sample=basic_mesh
cl %CompilerOptions% -I%KHLIB% %SampleBegin%%Sample%%SampleEnd% -Fm -LD /link %LinkerASM% -incremental:no -opt:ref -PDB:%Sample%_%random%.pdb -EXPORT:frame_update 
set Sample=basic_scene
cl %CompilerOptions% -I%KHLIB% %SampleBegin%%Sample%%SampleEnd% -Fm -LD /link %LinkerASM% -incremental:no -opt:ref -PDB:%Sample%_%random%.pdb -EXPORT:frame_update 
set Sample=basic_animation
cl %CompilerOptions% -I%KHLIB%  %SampleBegin%%Sample%%SampleEnd% -Fm -LD /link %LinkerASM% -incremental:no -opt:ref -PDB:%Sample%_%random%.pdb -EXPORT:frame_update 

cl %CompilerOptions% -I%KHLIB% ..\src\practice_win32.cpp -Fmpractice_win32.map /link %LinkerOptions% %EntryFunc%
rem set LastError=%ERRORLEVEL%

:END
cd ..\src\

rem ctime -end practice.ctm %LastError%