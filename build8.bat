echo off
if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
goto start
:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
:start

devenv.com crescent.vcproj /Rebuild Release
devenv.com dps.vcproj /Rebuild Release
devenv.com fukei.vcproj /Rebuild Release
devenv.com intruder.vcproj /Rebuild Release
devenv.com maria.vcproj /Rebuild Release
devenv.com prog_omake.vcproj /Rebuild Release
devenv.com sdps.vcproj /Rebuild Release
devenv.com system3.vcproj /Rebuild Release
devenv.com tengu.vcproj /Rebuild Release
devenv.com vampire.vcproj /Rebuild Release

pause
echo on
