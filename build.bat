@echo off

if not exist win32_build.exe ( 
    cl win32_build.c -nologo -I C:\Code\
)

pushd build
..\win32_build.exe
popd
