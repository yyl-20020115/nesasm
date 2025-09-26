cmake_minimum_required(VERSION 3.10)
project(nesasm C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(argp-standalone)

# Add source files
set(SOURCES 
    source/assemble.c
    source/code.c
    source/command.c
    source/crc.c
    source/defs.h
    source/expr.c
    source/expr.h
    source/externs.h
    source/func.c
    source/input.c
    source/inst.h
    source/macro.c
    source/main.c
    source/nes.h
    source/nes.c
    source/output.c
    source/proc.c
    source/protos.h
    source/symbol.c
    source/vars.h
)
include_directories(. argp-standaline)
add_executable(nesasm ${SOURCES})

if(MSVC)
	target_compile_options(nesasm PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(nesasm argp)

# Optional: Set output directory
set_target_properties(nesasm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
