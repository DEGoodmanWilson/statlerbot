cmake_minimum_required(VERSION 2.8)
project(statlerbot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

set(SOURCE_FILES main.cpp event_receiver.cpp event_receiver.h logging.h beep_boop_persist.cpp beep_boop_persist.h team_info.cpp team_info.h)
add_executable(statlerbot ${SOURCE_FILES})
target_link_libraries(statlerbot ${CONAN_LIBS})