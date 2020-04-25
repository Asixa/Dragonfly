@echo off

IF EXIST a.ll (
    goto a
) ELSE (
    cd ../bin/
    IF EXIST a.ll (
    cd ../tools/
    goto b
    ) ELSE ( 
    echo a.ll not found, pls make sure it is compiled and in ../bin folder.
    quit
    )
)
:a
llc -filetype=obj a.ll
lld-link -out:a.exe -nologo a.obj -defaultlib:libcmt "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905\lib\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64"

goto end

:b
llc -filetype=obj ../bin/a.ll
lld-link -out:a.exe -nologo ../bin/a.obj -defaultlib:libcmt "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905\lib\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64"
:end
echo --------------------
a.exe
echo --------------------
pause