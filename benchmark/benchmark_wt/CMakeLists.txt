## WT Benchmark

add_executable (benchmark_wt benchmark_wt_main.cpp)
target_include_directories (benchmark_wt PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories (benchmark_wt PRIVATE ${PROJECT_SOURCE_DIR}/benchmark)

# External headers

target_link_libraries (benchmark_wt tlx)
target_include_directories (benchmark_wt PRIVATE ${PROJECT_SOURCE_DIR}/deps/tlx)

target_link_libraries (benchmark_wt malloc_count -ldl)
target_include_directories (benchmark_wt PRIVATE ${PROJECT_SOURCE_DIR}/deps)

# PWM needs OMP
find_package (OpenMP REQUIRED)
target_link_libraries (benchmark_wt OpenMP::OpenMP_CXX)
target_include_directories (benchmark_wt PRIVATE ${PROJECT_SOURCE_DIR}/deps/pwm/include)

# Execution scripts

add_custom_target (run_test_benchmark)
add_custom_command (
    TARGET run_test_benchmark
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_benchmark.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_test_benchmark_pcc)
add_custom_command (
    TARGET run_test_benchmark_pcc
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_benchmark_pcc.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_custom_target (run_test_benchmark_huff_breakdown_random)
add_custom_command (
    TARGET run_test_benchmark_huff_breakdown_random
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_benchmark_huff_breakdown_random.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_test_benchmark_huff_breakdown_pcc)
add_custom_command (
    TARGET run_test_benchmark_huff_breakdown_pcc
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_benchmark_huff_breakdown_pcc.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_test_benchmark_dd_breakdown)
add_custom_command (
    TARGET run_test_benchmark_dd_breakdown
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_benchmark_dd_breakdown.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Final Benchmarks

add_custom_target (run_benchmark_pcc)
add_custom_command (
    TARGET run_benchmark_pcc
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_pcc.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_benchmark_large)
add_custom_command (
    TARGET run_benchmark_large
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_large.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_benchmark_lightweight)
add_custom_command (
    TARGET run_benchmark_lightweight
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_lightweight.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_benchmark_suffix_array)
add_custom_command (
    TARGET run_benchmark_suffix_array
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_suffix_array.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_benchmark_ru_wb)
add_custom_command (
    TARGET run_benchmark_ru_wb
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_ru_wb.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_benchmark_space_measure)
add_custom_command (
    TARGET run_benchmark_space_measure
    DEPENDS benchmark_wt
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_space_measure.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)