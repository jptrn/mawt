# For Benchmarks

# Two utility header are provided for measuring time and space, respectively:
# config.hpp, timer.hpp and spacer.hpp
# Note that if the latter is used, it must be linked against malloc_count with
# the following call:
#
# target_link_libraries (benchmark_wt malloc_count -ldl)

add_subdirectory (benchmark_wt)
add_subdirectory (text_analyzer)