cmake_minimum_required(VERSION 2.8)

project (torrent_client)

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/../jasl")
set (CMAKE_INCLUDE_PATH "/usr/local/include")
set (CMAKE_LIBRARY_PATH "/usr/local/lib")

LINK_DIRECTORIES(${PROJECT_BINARY_DIR})

set (CMAKE_CXX_COMPLILER "g++")
set (CMAKE_CXX_FLAGS "-std=c++0x -ggdb3 -Wall")

set (TARGET main)

set (SOURCE_LIST main.cpp

                ../jasl/common/sub_string.cpp

                bedecoder/be_token.cpp
                bedecoder/be_parser.cpp

                torrent_file.cpp

    )

add_executable (${TARGET} ${SOURCE_LIST})
target_link_libraries(${TARGET}
                        pthread
                        ssl
                        crypto
                     )



