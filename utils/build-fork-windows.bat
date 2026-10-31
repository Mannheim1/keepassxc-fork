@echo off
rem Builds the fork with MSVC. Run from any command prompt.
rem Usage: utils\build-fork-windows.bat [deps folder] [build folder]
setlocal
set "DEPS=%~1"
if "%DEPS%"=="" set "DEPS=C:\Dev\kpxc-deps"
set "BUILD=%~2"
if "%BUILD%"=="" set "BUILD=%DEPS%\build"
set "SOURCE=%~dp0.."

for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`) do set "VS=%%i"
call "%VS%\VC\Auxiliary\Build\vcvars64.bat" || exit /b 1

for /d %%q in ("%DEPS%\qt\6.*") do set "QT=%%q\msvc2022_64"

cmake -S "%SOURCE%" -B "%BUILD%" -G Ninja -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE="%DEPS%\vcpkg\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_PREFIX_PATH="%QT%" -DX_VCPKG_APPLOCAL_DEPS_INSTALL=ON -DKPXC_FEATURE_DOCS=OFF -DWITH_TESTS=ON -DWITH_GUI_TESTS=ON || exit /b 1
cmake --build "%BUILD%" || exit /b 1
cmake --install "%BUILD%" --prefix "%BUILD%\dist" || exit /b 1
