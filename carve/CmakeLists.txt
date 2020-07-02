cmake_minimum_required (VERSION 3.0)

set(FILES_SRC
    carve.h
    carve.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/libcarve
    ${CMAKE_CURRENT_SOURCE_DIR}/libcarve
)

if (EMSCRIPTEN)
    add_executable(carve ${FILES_SRC})
else (EMSCRIPTEN)
    add_library(carve SHARED ${FILES_SRC})
endif (EMSCRIPTEN)

target_link_libraries(carve libcarve)
