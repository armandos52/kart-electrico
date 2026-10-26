@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
cd /d "%~dp0fuentes_dev8\stm32_frontal\tests"
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /Istubs /I..\Core\Inc /I..\..\esp32_trasero\lib\kart_control "C:\kart proyecto\06_TRABAJO\evidencias\marchas_2026-10-21\reproducir_bloqueo.cpp" ..\..\esp32_trasero\lib\kart_control\kart_control.cpp /Fe:"C:\kart proyecto\06_TRABAJO\evidencias\marchas_2026-10-21\reproducir.exe"
if errorlevel 1 exit /b 1
"C:\kart proyecto\06_TRABAJO\evidencias\marchas_2026-10-21\reproducir.exe"
