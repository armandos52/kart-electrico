@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /I..\lib\kart_control control_test.cpp ..\lib\kart_control\kart_control.cpp /Fe:control_test.exe
if errorlevel 1 exit /b 1
control_test.exe
