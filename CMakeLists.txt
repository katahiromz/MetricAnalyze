# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008" .
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles" .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# enable CTest
enable_testing()

# project name and language
project(MetricAnalyze CXX)

# use FreeType
find_package(FreeType REQUIRED)

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

##############################################################################

add_executable(MetricAnalyze MetricAnalyze.cpp)
target_include_directories(MetricAnalyze PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(MetricAnalyze ${FREETYPE_LIBRARIES})

##############################################################################
