@echo off
setlocal

set "VSDEVCMD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" goto have_vsdev

echo Visual Studio 2022 Build Tools was not found:
echo %VSDEVCMD%
echo Check the Build Tools installation path.
pause
exit /b 1

:have_vsdev
call "%VSDEVCMD%" -arch=amd64 -host_arch=amd64
if errorlevel 1 goto env_failed

"C:\Program Files\CMake\bin\cmake.exe" --build "%~dp0build-windows-x64"
if errorlevel 1 goto build_failed

echo Build succeeded.
pause
exit /b 0

:env_failed
echo Failed to initialize the x64 MSVC environment.
pause
exit /b 1

:build_failed
echo Build failed.
pause
exit /b 1
