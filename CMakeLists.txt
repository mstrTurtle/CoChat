cmake_minimum_required (VERSION 3.11)

project(simple-server-client
  LANGUAGES CXX
  VERSION 1.0.0
)

SET(CMAKE_CXX_STANDARD 23)
SET(CXX clang++)
set(CMAKE_CXX_FLAGS "-fsanitize=address")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

option(CHATROOM_BUILD_SERVER "" ON)
option(CHATROOM_BUILD_SIMPLE_CLIENT "" ON)
option(CHATROOM_BUILD_COMPLEX_CLIENT "" ON)

add_subdirectory(src)
add_subdirectory(libs)