cmake_minimum_required(VERSION 2.8)

project(skinning)

#engine
include_directories(../../nenuzhno-engine)
add_library(nenuzhno-engine STATIC IMPORTED)
set_property(TARGET nenuzhno-engine PROPERTY IMPORTED_LOCATION ../../nenuzhno-engine/libnenuzhno-engine.a)
#add_subdirectory(../../nenuzhno-engine ../../build-nenuzhno-engine-Desktop32-Default)

#glfw
add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION ../../../../Libs/gl/glfw-3.1.2/lib-mingw/libglfw3.a)

#glew
include_directories(../../../../Libs/gl/glew-2.0.0/include)
add_library(glew STATIC IMPORTED)
set_property(TARGET glew PROPERTY IMPORTED_LOCATION ../../../../Libs/gl/glew-2.0.0/lib/libglew32.a)

#glm
include_directories(../../../../Libs/gl/glm-0.9.8.4/glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} nenuzhno-engine glfw glew OpenGL32)
