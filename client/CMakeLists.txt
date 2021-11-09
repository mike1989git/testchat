cmake_minimum_required (VERSION 2.8)

set (PROJECT client)
set (SRC_LIST client.cpp)


project (${PROJECT})

include_directories(. ${CMAKE_BINARY_DIR} /usr/local/include/nlohmann)

add_executable (${PROJECT} ${SRC_LIST} )
add_dependencies (${PROJECT} stdlib )

set_target_properties (${PROJECT} PROPERTIES COMPILE_FLAGS "-std=c++11 -O0 -g -fPIC")

