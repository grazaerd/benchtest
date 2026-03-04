cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

project(matrix)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(HAVE_STD_REGEX ON)
set(RUN_HAVE_STD_REGEX 1)
set(BENCHMARK_ENABLE_TESTING       OFF CACHE BOOL "Disable benchmark tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS   OFF CACHE BOOL "Disable gtest tests" FORCE)
set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "Disable assembly tests" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "Don't download deps" FORCE)
set(BENCHMARK_ENABLE_DOXYGEN       OFF CACHE BOOL "Disable doxygen" FORCE)
set(BENCHMARK_INSTALL_DOCS         OFF CACHE BOOL "Disable doc install" FORCE)
set(BENCHMARK_ENABLE_INSTALL       OFF CACHE BOOL "Disable installation" FORCE)

set(CMAKE_C_FLAGS "-O3 -ffast-math -mfma -mavx2 -falign-functions=32 -falign-loops=32 -mavx -mbranches-within-32B-boundaries -msse4 -mlzcnt -mbmi -fuse-ld=lld -fno-common -fno-record-gcc-switches -pthread -DNDEBUG -static -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "-O3 -ffast-math -mfma -mavx2 -falign-functions=32 -falign-loops=32 -mbranches-within-32B-boundaries -mavx -msse4 -mlzcnt -mbmi -fuse-ld=lld -fno-common -fno-record-gcc-switches -pthread -DNDEBUG -static -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fno-rtti -fdata-sections -ffunction-sections -Wno-c2y-extensions")

add_executable(matrix src/main.cpp)

set(benchmark "${CMAKE_CURRENT_SOURCE_DIR}/lib/benchmark")
set(dxmath "${CMAKE_CURRENT_SOURCE_DIR}/lib/DirectXMath")

add_subdirectory(${benchmark})
add_subdirectory(${dxmath})

target_include_directories(matrix SYSTEM PRIVATE ${benchmark})
target_include_directories(matrix SYSTEM PRIVATE ${dxmath})

target_link_libraries(matrix PRIVATE benchmark)
