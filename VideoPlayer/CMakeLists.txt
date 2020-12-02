cmake_minimum_required(VERSION 3.17)
project(Testing)

set(CMAKE_CXX_STANDARD 11)

add_executable(Testing main.cpp BitStream.cpp Golomb.cpp Predictor.cpp)


set(OpenCV_DIR D:\\Programs\\opencv\\opencv\\mingw-build2)
find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(Testing ${OpenCV_LIBS})