cmake_minimum_required(VERSION 2.8)

project (torrent_client)

set (JASL "${PROJECT_SOURCE_DIR}/../jasl")

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${JASL}")
set (CMAKE_INCLUDE_PATH "/usr/local/include")
set (CMAKE_LIBRARY_PATH "/usr/local/lib")

LINK_DIRECTORIES(${PROJECT_BINARY_DIR})
LINK_DIRECTORIES(${JASL}/build)

set (CMAKE_CXX_COMPLILER "clang++")
set (CMAKE_CXX_FLAGS "-std=c++0x -ggdb3 -Wall")

set (TARGET main)

set (SOURCE_LIST main.cpp

                init_http_request.cpp
                tracker_response.cpp
                client_logic.cpp
                active_sockets.cpp
                peer_protocol.cpp

                bedecoder/be_token.cpp
                bedecoder/be_parser.cpp

                torrent_file.cpp
                piece_list.cpp
                piece.cpp
                files.cpp
                download_manager.cpp

    )

add_executable (${TARGET} ${SOURCE_LIST})
target_link_libraries(${TARGET}
                        jasl
                        pthread
                        ssl
                        crypto
                     )



