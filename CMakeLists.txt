project(RenderLayers)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_C_COMPILER gcc-5)
set(CMAKE_CXX_COMPILER g++-5)

SET(GCC_COVERAGE_LINK_FLAGS "-lassimp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl")
#SET(GCC_COVERAGE_COMPILE_FLAGS "-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl")

SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

SET(CMAKE_CXX_FLAGS "--std=c++11")
set(CUDA_PROPAGATE_HOST_FLAGS ON)
set(CMAKE_CXX_STANDARD 14)

set(GLFW_INCLUDE_DIR "/usr/include/GLFW" )
set(GLFW_LIBRARIES "/usr/lib/x86_64-linux-gnu/libglfw.so" )

find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

#include_directories(${X11_INCLUDE_DIR})
#link_directories(${X11_LIBRARIES})

## find OpenCV
find_package(OpenCV 3.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_DIR}/lib)

## find OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    message("opengl found")
endif (OPENGL_FOUND)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
#if (GLFW_FOUND)
#    message(STATUS "dirs = ${GLFW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}")
    #include_directories(${GLFW_INCLUDE_DIR})
#endif (GLFW_FOUND)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/glad/include ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR})

file(GLOB glad_SRCS ${PROJECT_SOURCE_DIR}/glad/src/glad.c ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_NAME} main.cpp ${glad_SRCS})

target_link_libraries(${PROJECT_NAME} /usr/lib/x86_64-linux-gnu/libdl.so /usr/local/lib/libassimp.so ${X11_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${OpenCV_LIBS})

