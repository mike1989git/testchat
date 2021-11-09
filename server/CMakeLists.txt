cmake_minimum_required (VERSION 2.8)

set (PROJECT server)
set (SRC_LIST server.cpp)


project (${PROJECT})

include_directories(. ${CMAKE_BINARY_DIR} /usr/local/include/nlohmann)


add_executable (${PROJECT} ${SRC_LIST} )
add_dependencies (${PROJECT} stdlib )

install (TARGETS ${PROJECT} RUNTIME DESTINATION ../)
set_target_properties (${PROJECT} PROPERTIES COMPILE_FLAGS "-std=c++11 -O0 -g -fPIC")

