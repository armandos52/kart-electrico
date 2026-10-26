@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
set "CONTROL=fuentes\07_DESARROLLO\firmware\esp32_trasero\lib\kart_control"
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /I%CONTROL% fuentes\07_DESARROLLO\firmware\esp32_trasero\tests\control_test.cpp %CONTROL%\kart_control.cpp /Fe:baseline.exe > build_baseline.log 2>&1
if errorlevel 1 exit /b 1
baseline.exe > baseline.log
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /I%CONTROL% diagnostico_dev3.cpp %CONTROL%\kart_control.cpp /Fe:diagnostico_dev3.exe > build_dev3.log 2>&1
if errorlevel 1 exit /b 1
diagnostico_dev3.exe > diagnostico_dev3.log
rem Este codigo 1 se espera mientras los requisitos propuestos no se implementen.
echo dev3_exit=%errorlevel% > resultados_exit.txt
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 diagnostico_v12.cpp /Fe:diagnostico_v12.exe > build_v12.log 2>&1
if errorlevel 1 exit /b 1
diagnostico_v12.exe > diagnostico_v12.log
echo v12_exit=%errorlevel% >> resultados_exit.txt
