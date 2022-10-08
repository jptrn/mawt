# For all external dependencies

### Implementation Libraries 

### Benchmarking Libraries ###

# TLX

add_library (tlx
  tlx/tlx/cmdline_parser.cpp
  tlx/tlx/string/parse_si_iec_units.cpp)

target_include_directories (tlx SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/deps/tlx)

# Malloc_Count

add_library (malloc_count
  ${PROJECT_SOURCE_DIR}/deps/malloc_count/malloc_count.c
  ${PROJECT_SOURCE_DIR}/deps/malloc_count/stack_count.c)

target_include_directories (malloc_count SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/deps)

### Testing Libraries ###

# GTest
include (FetchContent)
FetchContent_Declare (
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable (googletest)

### WT Implementations ###

# Kurpicz PWM #