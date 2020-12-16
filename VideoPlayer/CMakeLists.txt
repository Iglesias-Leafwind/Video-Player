cmake_minimum_required(VERSION 3.17)
project(VideoPlayer)

set(CMAKE_CXX_STANDARD 11)

add_executable(VideoPlayer BitStream.cpp Golomb.cpp Predictor.cpp main.cpp)
add_compile_options(-xO3)

set(OpenCV_DIR D:\\Programs\\opencv\\opencv\\mingw-build2)
find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(VideoPlayer ${OpenCV_LIBS})