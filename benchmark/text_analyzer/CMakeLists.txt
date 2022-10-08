## Text Analyzer

add_executable (text_analyzer text_analyzer.cpp)
target_include_directories (text_analyzer PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories (text_analyzer PRIVATE ${PROJECT_SOURCE_DIR}/benchmark)

target_link_libraries (text_analyzer tlx)
target_include_directories (text_analyzer PRIVATE ${PROJECT_SOURCE_DIR}/deps/tlx)

add_custom_target (run_text_analyzer)
add_custom_command (
    TARGET run_text_analyzer
    DEPENDS text_analyzer
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/get_text_info.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target (run_text_analyzer_ru)
add_custom_command (
    TARGET run_text_analyzer_ru
    DEPENDS text_analyzer
    COMMAND 
        BUILD_TYPE=${CMAKE_BUILD_TYPE};
        BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}; 
        ${CMAKE_CURRENT_SOURCE_DIR}/get_text_info_ru.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)