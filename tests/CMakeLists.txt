cmake_minimum_required(VERSION 3.5)

project(XR-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(XR-tests
    openxr/main.cpp
    openxr/OpenGLRenderer.cpp
    openxr/oxr.cpp
    )

target_include_directories(XR-tests PUBLIC "/usr/include/openxr")

target_link_libraries(XR-tests glut GLU freeimage glfw ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} OpenGL::GL ${X11_LIBRARIES} openxr_loader)


 
