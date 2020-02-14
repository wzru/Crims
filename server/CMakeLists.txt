cmake_minimum_required (VERSION 3.15)
project (crims_server)

aux_source_directory(./src DIR_SRCS)
include_directories(./include)

add_compile_options(-O2 -Wall -g -fgnu89-inline --std=c11 -static)

#set(CMAKE_EXE_LINKER_FLAGS -static)

#add_definitions(-DOPJ_STATIC)
link_directories("${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
add_executable(crims_server ${DIR_SRCS})

target_link_libraries(crims_server WS2_32)
target_link_libraries(crims_server parser.lib)

add_custom_target(parser)
add_custom_command(
    TARGET parser
    PRE_BUILD
    COMMAND cd "${PROJECT_SOURCE_DIR}/sql-parser" && chdir && make static
    COMMAND ${CMAKE_COMMAND} -E 
        copy_if_different  
        ${PROJECT_SOURCE_DIR}/sql-parser/ast.h
        ${PROJECT_SOURCE_DIR}/include/ast.h
    COMMAND ${CMAKE_COMMAND} -E 
        copy_if_different  
        ${PROJECT_SOURCE_DIR}/sql-parser/parser.lib
        ${PROJECT_SOURCE_DIR}/lib/parser.lib
    #COMMAND cd "${PROJECT_SOURCE_DIR}/sql_parser" && make clean
)
add_dependencies(crims_server parser)

# add_custom_command(OUTPUT parser
#     COMMAND echo "Creating parser"
#     # Display the given message before the commands are executed at build time
#     COMMENT "Creating parser"
# )