cmake_minimum_required (VERSION 3.10)
project (crims_server)

#set(CMAKE_C_COMPILER "C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin\gcc.exe")
set(CMAKE_SH "CMAKE_SH-NOTFOUND")
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

aux_source_directory(./src DIR_SRCS)
include_directories(./include)
include_directories(./parser)
link_directories("${PROJECT_SOURCE_DIR}/parser") #必须在add_executable之前

add_compile_options(-g -fgnu89-inline --std=c11 -static)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG) 
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  add_compile_options(-O2)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

add_executable(crims_server ${DIR_SRCS})
if(WIN32)
  message("[INFO]: Build on WIN32...")
  target_link_libraries(crims_server WS2_32)
  target_link_libraries(crims_server parser.lib)
  add_custom_target(parser)
  add_custom_command(
      TARGET parser
      PRE_BUILD
      COMMAND make -C "${PROJECT_SOURCE_DIR}/parser" static
  )
  add_dependencies(crims_server parser)
else()
  message("Not build on WIN32...")
  target_link_libraries(crims_server libparser.a)
  target_link_libraries(crims_server pthread)
  add_custom_target(parser)
  add_dependencies(crims_server parser)
  add_custom_command(
      TARGET parser
      PRE_BUILD
      COMMAND make -C "${PROJECT_SOURCE_DIR}/parser" static
  )
endif()