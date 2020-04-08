@echo off
llc -filetype=obj a.ll
lld-link -out:a.exe -nologo a.obj -defaultlib:libcmt "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905\lib\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64" "-libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64"
echo --------------------
a.exe
echo --------------------
pause