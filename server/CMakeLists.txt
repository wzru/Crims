cmake_minimum_required (VERSION 3.16)

project (crims)

aux_source_directory(./src DIR_SRCS)

include_directories(./include)

add_compile_options(-O2 -Wall -g -fgnu89-inline --std=c11)

# add_definitions(-DOPJ_STATIC)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

add_executable(crims ${DIR_SRCS})

set(CMAKE_PREFIX_PATH "./lib")

target_link_libraries(crims WS2_32)
# target_link_libraries(crims pthreadVC2)
# target_link_libraries(crims libpthreadGC2.a)