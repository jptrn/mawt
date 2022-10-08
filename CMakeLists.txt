cmake_minimum_required (VERSION 3.16.3 FATAL_ERROR)
project (ma)

message (STATUS " [[ MA Wavelet Tree Construction ]] ")

# Set project settings

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)


# Test presence of arch flags

if (NOT ARCH_FLAGS_SUCCESS) 
    message (STATUS
    "!! Attention !! \n"
    "Please make sure that your CPU supports the architecture flags listed in the file ArchFlags.txt\n"
    "If your CPU does not support those flags, please use the Intel Software Development Emulator (https://www.intel.com/content/www/us/en/developer/articles/tool/software-development-emulator.html) to run this project!")
endif () 

# Arch Flags

if (NOT ARCH_FLAGS)
    set (ARCH_FLAGS "-march='native'")
endif()
set (CONSTEXPR_LIMIT "4294967296")
set (GCC_WARNINGS "-Wall -Wextra -Wno-ignored-attributes")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -fconstexpr-ops-limit=${CONSTEXPR_LIMIT} ${ARCH_FLAGS} ${GCC_WARNINGS}")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Enable testing

enable_testing ()

# Set Build Configuration Type (Debug / Release)

if (NOT CMAKE_BUILD_TYPE)
    message (STATUS "CMAKE_BUILD_TYPE not defined - setting to Debug.")
    set (CMAKE_BUILD_TYPE "Debug")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message (STATUS "CMAKE_BUILD_TYPE is Release")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message (STATUS "CMAKE_BUILD_TYPE is Debug")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message (STATUS "CMAKE_BUILD_TYPE is RelWithDebInfo")
else ()
    message (FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

# Add subdirectories to build

add_subdirectory (benchmark)
add_subdirectory (demo)
add_subdirectory (deps)
add_subdirectory (test)
