echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles%\Windows Kits\8.1\bin\x86";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles(x86)%\Windows Kits\8.1\bin\x86";%PATH%

:start

devenv.com system1.vcproj /Rebuild Release
devenv.com system2.vcproj /Rebuild Release
devenv.com system3.vcproj /Rebuild Release
devenv.com prog_omake.vcproj /Rebuild Release

pushd Release
for /r %%i in (*.exe) do mt.exe /manifest ..\vista.manifest -outputresource:%%i;1
popd

pause
echo on
