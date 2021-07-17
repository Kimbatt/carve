cmake_minimum_required (VERSION 3.0.0)

option (DEBUG "DEBUG" OFF)
option (COMPILE_WITH_CLANG "COMPILE_WITH_CLANG" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    if (COMPILE_WITH_CLANG)
        set(CMAKE_GENERATOR_TOOLSET "clangcl")
    endif (COMPILE_WITH_CLANG)
    # Enable enhanced instruction set:      Advanced Vector Extensions 2 (/arch:AVX2)
    # Enable intrinsic functions:           Yes (/Oi)
    # Favor size or speed:                  Favor fast code (/Ot)
    # Inline function expansion:            Any Suitable (/Ob2)
    set(CARVE_RELEASE_COMPILER_FLAGS   "/arch:AVX2 /Oi /Ot /Ob2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CARVE_RELEASE_COMPILER_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${CARVE_RELEASE_COMPILER_FLAGS}")
else (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".bc")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORT_ALL=1")
    else (EMSCRIPTEN)
        add_definitions(-g -fPIC)
    endif (EMSCRIPTEN)
endif (WIN32)

project (Carve)

add_definitions(-DCARVE_USE_EXACT_PREDICATES)
add_definitions(-DCARVE_MULTITHREADING)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)

add_subdirectory(libcarve)
add_subdirectory(carve)

add_subdirectory(carve_run)
