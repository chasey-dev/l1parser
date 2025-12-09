cmake_minimum_required(VERSION 3.10)

project(l1parser-ucode)

# look for ucode library
FIND_LIBRARY(ucode_library NAMES ucode)

# find header directory
# find ucode include dir
FIND_PATH(ucode_include_dir ucode/module.h)

IF(NOT ucode_library OR NOT ucode_include_dir)
    MESSAGE(FATAL_ERROR "ucode library or headers not found")
ENDIF()

# include ucode
include_directories(${ucode_include_dir})
# include l1parser
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# compiling options refer to ucode src repo
add_definitions(-Os -Wall -Werror -g3)

# add l1parser-ucode library, MODULE as a plugin
add_library(l1parser-ucode MODULE l1parser.cpp)

# remove 'lib' prefix in .so
set_target_properties(l1parser-ucode PROPERTIES OUTPUT_NAME l1parser PREFIX "")

# optimize linking
set_target_properties(l1parser-ucode PROPERTIES LINK_FLAGS "-Wl,--gc-sections")

# link l1parser, ucode explicitly
target_link_libraries(l1parser-ucode l1parser ${ucode_library})

# install
install(TARGETS l1parser-ucode LIBRARY DESTINATION lib/ucode)