@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /Ifuentes\stm32_frontal\tests\stubs /Ifuentes\stm32_frontal\Core\Inc /Ifuentes\esp32_trasero\lib\kart_control replay.cpp fuentes\esp32_trasero\lib\kart_control\kart_control.cpp /Fe:replay.exe
if errorlevel 1 exit /b 1
replay.exe
exit /b %errorlevel%
