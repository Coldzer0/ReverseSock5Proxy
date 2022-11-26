@echo off
cd ..
if exist Build (
  rmdir Build /Q /S
)
mkdir Build
if exist cmake (
  rmdir cmake /Q /S
)
mkdir cmake

cd cmake
conan install .. -s build_type=Release --build missing -s compiler.runtime=MT -s compiler="Visual Studio" -s compiler.cppstd=20 -if ../cmake
if %ERRORLEVEL% GEQ 1 EXIT /B 1

cd ../Build
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B .
if %ERRORLEVEL% GEQ 1 EXIT /B 1
cmake --build . --target SOCK5Server -j 12 --config Release
if %ERRORLEVEL% GEQ 1 EXIT /B 1
cd ../scripts

pause
