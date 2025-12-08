cmake_minimum_required(VERSION 3.10)
project(l1parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include header directories
include_directories(include)

# Build Core Library (libl1parser.so)
add_library(l1parser SHARED 
    lib/l1parser.cpp
    lib/c_wrapper.cpp
)

install(TARGETS l1parser DESTINATION lib)
install(FILES include/l1parser.h DESTINATION include)

# Build CLI Tool (l1util)
add_executable(l1util cli/l1util.cpp)
target_link_libraries(l1util l1parser)
install(TARGETS l1util DESTINATION bin)

# Add ucode binding subdirectory
option(BUILD_UCODE "Build ucode binding" ON)

if(BUILD_UCODE)
    add_subdirectory(ucode)
endif()