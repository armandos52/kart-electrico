@echo off
cd /d "%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /Od /utf-8 /I"../../../07_DESARROLLO/firmware/stm32_frontal/tests/stubs" /I"../../../07_DESARROLLO/firmware/stm32_frontal/Core/Inc" /I"../../../07_DESARROLLO/firmware/esp32_trasero/lib/kart_control" banco_host.cpp "../../../07_DESARROLLO/firmware/nucleo_f411_banco/src/bench_logic.cpp" "../../../07_DESARROLLO/firmware/nucleo_f411_banco/src/backend.cpp" /Fe:banco_host.exe
if errorlevel 1 exit /b 1
banco_host.exe
exit /b %errorlevel%
