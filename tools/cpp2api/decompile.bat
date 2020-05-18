@echo off
clang_.exe -S -emit-llvm test.cpp
llc.exe test.ll -march=cpp -o api.cpp
pause