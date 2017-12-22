cmake_minimum_required(VERSION 3.8)
project(proj2)

set(CMAKE_CXX_STANDARD 11)

include_directories(/home/ales/Dropbox/MIT/BMS/proj2)
link_directories(/home/ales/Dropbox/MIT/BMS/proj2)

set(SOURCE_FILES bms2A.cpp)
set(SRC_FL bms2B.cpp)
set(SC integ.cpp)

add_executable(modulator ${SOURCE_FILES})
add_executable(demodulator ${SRC_FL})
add_executable(integ ${SC})

target_link_libraries(modulator libsndfile.a)
target_link_libraries(demodulator libsndfile.a)