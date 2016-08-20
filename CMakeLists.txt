cmake_minimum_required(VERSION 2.8)
project(statlerbot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

set(SOURCE_FILES main.cpp oauth.cpp oauth.h token_storage.cpp token_storage.h event_receiver.cpp event_receiver.h route_set.cpp route_set.h message_router.cpp message_router.h)
add_executable(statlerbot ${SOURCE_FILES})
target_link_libraries(statlerbot ${CONAN_LIBS})