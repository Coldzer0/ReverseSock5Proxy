cmake_minimum_required(VERSION 3.22)

project ("SOCK5Server" CXX)

set(CMAKE_CXX_STANDARD 20)

# MSVC Static linking.
if(MSVC)
	if (${CMAKE_BUILD_TYPE} MATCHES "Release")
	  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
	elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()	
endif()

# set the path to our cmake modules & generated cmake files from Conan.
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(imgui CONFIG)
find_package(glfw3 CONFIG)
find_package(glew CONFIG)

include_directories( "src" "src/UI" "src/UI/include" "src/bindings/imgui_backend" "src/bindings/stb" "src/bindings/mINI/src/mini")

# Locate all needed source & header files.
file(GLOB source_files CONFIGURE_DEPENDS
		"src/UI/*.cpp"
		"src/UI/*.h"
		"src/UI/Font/*.h"
        "src/UI/src/*.cpp"
        "src/UI/include/*.h"
        "src/UI/src/Sock5/*.cpp"
        "src/UI/include/Sock5/*.h"
		"src/bindings/imgui_backend/*.cpp"
		"src/bindings/imgui_backend/*.h"
)
# Add source to this project's executable.
add_executable (SOCK5Server WIN32 
	"src/SOCK5Server.cpp" 
	"src/SOCK5Server.h"     
	${source_files}
	"src/bindings/mINI/src/mini/ini.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/info.rc")

target_sources(SOCK5Server PRIVATE dpi-aware.manifest)

# Linker Options
target_compile_definitions(SOCK5Server PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(SOCK5Server imgui::imgui GLEW::glew_s glfw::glfw ws2_32)

# Static linking libgcc, libstdc++ and winpthread.
if(NOT MSVC)
	if (${CMAKE_BUILD_TYPE} MATCHES "Release")
		# -fuse-ld=lld
		target_link_options(SOCK5Server PRIVATE -static-libgcc -static-libstdc++ -static -lwinpthread -dynamic -pthread -O3)
	elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_options(SOCK5Server PRIVATE -static-libgcc -static-libstdc++ -static -lwinpthread -dynamic -pthread -O0)
	endif()	
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SOCK5Server PROPERTY CXX_STANDARD 20)
endif()
