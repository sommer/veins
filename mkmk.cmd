@echo off
cd %~dp0
if exist ..\omnetpp\setenv-vc71.bat call ..\omnetpp\setenv-vc71.bat
nmake /? >nul 2>nul || echo *** ERROR: nmake.exe is not in the PATH *** && goto end
call opp_test >nul 2>nul || echo *** ERROR: OMNeT++/bin is not in the PATH *** && goto end

set OMNETPP_ROOT=C:\omnetpp-3.3
set ROOT=%~dp0

echo "Generating makefiles..."
nmake /nologo ROOT=%ROOT% OMNETPP_ROOT=%OMNETPP_ROOT% -f Makefile.win || echo "*** ERROR GENERATING MAKEFILES ***" && goto end

echo "Adding dependencies to makefiles..."
nmake /nologo -f Makefile.vc depend || echo "*** ERROR ADDING DEPENDENCIES TO MAKEFILES ***" && goto end

:end
