@echo off
echo Compiling precompiled header...
g++ -DUNICODE -D_UNICODE pch.h
if %ERRORLEVEL% == 0 (
    echo Done. pch.h.gch generated.
) else (
    echo PCH compilation failed.
)