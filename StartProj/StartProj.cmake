add_executable(StartProj
        StartProj/src/main.cpp
)
set_target_properties(StartProj PROPERTIES OUTPUT_NAME StartProj)
set_target_properties(StartProj PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/StartProj
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/StartProj
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/StartProj
)
add_dependencies(StartProj
        GraphicsProgrammingWithCppSSE4_2
        GraphicsProgrammingWithCppAVX2
        GraphicsProgrammingWithCppAVX512
)
target_include_directories(StartProj PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/StartProj/src
)
set_target_properties(StartProj PROPERTIES
        POSITION_INDEPENDENT_CODE False
        INTERPROCEDURAL_OPTIMIZATION ${LTO_ENABLED}
)
target_compile_options(StartProj PRIVATE
        ${CMAKE_CONFIG_FLAGS}
        ${Polly_FLAGS}
)
target_link_options(StartProj PRIVATE
        ${Polly_LINK_FLAGS}
)
