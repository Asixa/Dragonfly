@echo off
IF EXIST dragonfly_test.exe (
    dragonfly_test.exe
) ELSE (
    cd ../bin/
    IF EXIST dragonfly_test.exe (
    dragonfly_test.exe
    ) ELSE ( 
    echo dragonfly_test.exe not found, pls make sure it is compiled and in /bin folder.
    )
)
pause