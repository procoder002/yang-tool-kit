cmake_minimum_required(VERSION 2.8)

project(yang_tool_kit)

#set(LIBYANG_DIR ...3rd_party/libyang/build)

# Compiler and flags
set(CMAKE_CXX_STANDARD 11)  # Or a more recent standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g") # Add -Wall and -Wextra

include_directories(${LIBYANG_DIR} ./include)
link_directories(${LIBYANG_DIR})


set(SOURCES
    src/ytk_code_gen.cpp
    src/ytk_parser.cpp
    src/ytk_codec.cpp
    src/ytk_generate.cpp
    src/utility.cpp
)

add_executable(yang_tool_kit ${SOURCES})

target_link_libraries(yang_tool_kit yang)
