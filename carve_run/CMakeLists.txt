cmake_minimum_required (VERSION 3.0.0)

set(FILES_SRC
    3DFileReader.h
    3DFileReader.cpp
    main.cpp
)

add_executable(carve_run ${FILES_SRC})
target_include_directories(carve_run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Carve)
target_include_directories(carve_run PRIVATE ${CMAKE_SOURCE_DIR}/Carve)

target_link_libraries(carve_run carve)
