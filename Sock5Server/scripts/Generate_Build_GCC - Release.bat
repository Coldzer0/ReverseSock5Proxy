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
del *.txt *.cmake *.json *.lock
conan install .. -s build_type=Release --build missing -s compiler=gcc -s compiler.libcxx=libstdc++11 -s compiler.version=12 -s compiler.cppstd=20 -if ../cmake

cd ../Build
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -S .. -B .
cmake --build . --target SOCK5Server -j 12 --config Release

cd ../scripts
pause