@echo off
rem ctime -begin practice.ctm
rem echo tg

rem WHERE cl >nul 2>nul
rem IF %ERRORLEVEL% EQU 0 (GOTO clfound)
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
rem WHERE cl >nul 2>nul
rem IF %ERRORLEVEL% EQU 0 (GOTO clfound)
rem call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rem :clfound

rem Ox ??
set ReleaseOBJ= -O2 -Zi -Zo -Gm-
set Release= -O2 -Zo
set DebugPDB= -Od -Zi -Gm
set DebugOBJ= -Od -Z7
rem -Z7
rem -MTd
rem -EHsc

set ASSIMPDIR="w:\tools\assimp"
set ASSIMPLIB=%ASSIMPDIR%\lib\assimp-vc140-mt.lib
set ASSIMPINC=%ASSIMPDIR%\include
set KHLIB="..\src\lib"

set Current= %Release%
rem set Current= %ReleaseOBJ%
rem set Current= %DebugPDB%
rem set Current= %DebugOBJ%


rem set LinkerASM= kh_asm.obj

set RENDERDOC_INC="D:\work\ext\profile\renderdoc\renderdoc\api"
set DEBUGLIB=imgui.lib

set CompilerOptions= %Current% -nologo -MP -MTd -EHa- -EHsc -Oi /FAcs -fp:fast -WX -W4  -FC
set NoErrors= -wd4505 -wd4201 -wd4100 -wd4189 -wd4127 -wd4244 -wd4101 -wd4577
set CompilerOptions=%CompilerOptions% %NoErrors%
set CompilerOptions=%CompilerOptions% -DKH_EDITOR_MODE=1
set CompilerOptions=%CompilerOptions% -DKH_IN_DEVELOPMENT=1
set LinkerOptions= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
set CopyFiles=0
set IACA=" "
set INC=-I%IACA% -I%KHLIB% -I%RENDERDOC_INC%

IF NOT EXIST .\..\build\ mkdir .\..\build
cd .\..\build
del *.pdb > NUL 2> NUL

set EntryFunc= /subsystem:windows /entry:WinMainCRTStartup

cl %CompilerOptions% %KHLIB%\ext\imgui.cpp %KHLIB%\ext\imgui_draw.cpp -DIMGUI_API=__declspec(dllexport) -Fm -LD /link -incremental:no -opt:ref

set Sample=
set SampleBegin=..\src\samples\kh_
set SampleEnd=_sample.cpp
	
set Sample=%Sample%;basic_scene
set Sample=%Sample%;basic_mesh
set Sample=%Sample%;basic_animation
set Sample=%Sample%;scene_pathtracer
set Sample=%Sample%;radiance_transfer_view

for %%s in (%Sample%) do ( 
	cl %CompilerOptions% %INC%  %SampleBegin%%%s%SampleEnd% -Fm -LD /link %LinkerASM% -incremental:no -opt:ref -PDB:%%s_%random%.pdb  -EXPORT:frame_update imgui.lib 
)
cl -DWIN32 %CompilerOptions% %INC% ..\src\win32_main.cpp -Fmwin32_practice.map -Fewin32_practice.exe /link %LinkerOptions% %EntryFunc% imgui.lib opengl32.lib -STACK:8000000

set EntryFunc=/subsystem:console
set SDL_DIR="w:\tools\sdl2"
set SDL_INC=%SDL_DIR%\include
set SDL_LIB=%SDL_DIR%\lib\x64
cl -DWIN32 %CompilerOptions% %INC% -D_CRT_SECURE_NO_WARNINGS -I%SDL_INC% ..\src\sdl_main.cpp -Fmsdl_practice.map -Fesdl_practice.exe /link %LinkerOptions% %EntryFunc% %SDL_LIB%\SDL2.lib %SDL_LIB%\SDL2main.lib imgui.lib opengl32.lib -STACK:8000000


set STBWarnings= -wd4244 -wd4701 -wd4703
set MikkWarnings= -wd4456

cl %CompilerOptions% %STBWarnings% %MikkWarnings% -I%KHLIB% -I%ASSIMPINC% -D_CRT_SECURE_NO_WARNINGS /Fepack_asset.exe ..\src\kh_asset_packaging.cpp /link %LinkerOptions% %ASSIMPLIB%

cl %CompilerOptions% %STBWarnings% %MikkWarnings% -I%KHLIB% -I%ASSIMPINC% -D_CRT_SECURE_NO_WARNINGS ..\src\kh_asset_import.cpp -Fm -LD /link %LinkerOptions% %ASSIMPLIB% -incremental:no -opt:ref -EXPORT:init_asset_loader -EXPORT:load_trimesh_file -EXPORT:load_font_file -EXPORT:load_tex2d_file -EXPORT:load_skeleton_file -EXPORT:load_skin_file -EXPORT:load_animation_file -EXPORT:load_animation_for_skin_files

set LastError=%ERRORLEVEL%

:END
cd ..\src\