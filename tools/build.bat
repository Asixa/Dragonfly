@echo off

IF EXIST Program.ll (
    goto a
) ELSE (
    cd ../bin/
    IF EXIST Program.ll (
    cd ../tools/
    goto b
    ) ELSE ( 
    echo Program.ll not found, pls make sure it is compiled and in ../bin folder.
    quit
    )
)
:a
llc -filetype=obj Program.ll
lld-link -out:Program.exe -nologo Program.obj -defaultlib:libcmt "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905\lib\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64"

goto end

:b
llc -filetype=obj ../bin/Program.ll
lld-link -out:Program.exe -nologo ../bin/Program.obj    -defaultlib:libcmt -defaultlib:cuda "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905\lib\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" "-libpath:C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.1\lib\x64"
:end
echo --------------------
Program.exe
echo --------------------
pause