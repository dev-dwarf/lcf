rem @echo off
@set LIBS=-I C:\Code\ -I C:\Code\raylib\src
@set DISABLED= -wd4201 -wd4100 -wd4189 -wd4244 -wd4456 -wd4457 -wd4245
@set FLAGS= -nologo -GR- -Oi -Zi -W4 %DISABLED%

@IF NOT EXIST build mkdir build

@for %%i in (programs\*.c) do call :build %%i
@goto :eof

:build
@pushd build
cl "..\%1" %LIBS% %FLAGS% -link "C:\Code\raylib\raylib.lib" %DISABLE_CONSOLE%
@popd
@goto :eof
