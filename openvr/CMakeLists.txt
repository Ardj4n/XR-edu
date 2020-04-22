cmake_minimum_required(VERSION 3.5)

project(openvr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(openvr xr.cpp fbo.cpp oxr.cpp OpenGLRenderer.cpp)

target_include_directories(openvr PUBLIC "/usr/include/openxr")
target_include_directories(openvr PUBLIC "../dependencies/openvr/include")

target_link_libraries(openvr glut glfw ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} OpenGL::GL ${X11_LIBRARIES} openxr_loader)


 
