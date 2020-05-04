cmake_minimum_required(VERSION 3.5)

project(XR-edu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

set(XR-edu_SRC
    "SupSI-GL/*.cpp"
    "TransformApp/*.cpp"
)
list(REMOVE_ITEM XR-edu_SRC  "SupSI-GL/DirectXRenderer.h")
list(REMOVE_ITEM XR-edu_SRC  "SupSI-GL/DirectXRenderer.cpp")


add_executable(XR-edu
    TransformApp/MainApplication.cpp
    SupSI-GL/Engine.cpp
    SupSI-GL/List.cpp

    SupSI-GL/Camera.cpp
    SupSI-GL/oxr.cpp
    SupSI-GL/Mesh.cpp
    SupSI-GL/Material.cpp
    SupSI-GL/Texture.cpp
    SupSI-GL/Light.cpp
    SupSI-GL/Node.cpp
    SupSI-GL/Object.cpp
    SupSI-GL/OvoReader.cpp
    SupSI-GL/Engine.cpp
    SupSI-GL/Fbo.cpp
    SupSI-GL/Program.cpp
    SupSI-GL/OpenGLRenderer.cpp

    SupSI-GL/Vertex.cpp

    SupSI-GL/Face.cpp

    SupSI-GL/shader.cpp
    )

target_include_directories(XR-edu PUBLIC "SupSI-GL")
target_include_directories(XR-edu PUBLIC "/usr/include/openxr")
target_include_directories(XR-edu PUBLIC "dependencies/openvr/include")

target_link_libraries(XR-edu glut GLU freeimage glfw ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} OpenGL::GL ${X11_LIBRARIES} openxr_loader)


 
