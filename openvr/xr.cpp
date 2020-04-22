#ifndef _WINDOWS
#include <GL/glew.h>

#include "oxr.h"

#include <iostream>
#include <string>
#include <functional>

using namespace std;

OvXR xr{};
int frames = 0;
int fps = 0;
float alphaZ = 0.f;

void  DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
   std::cout << "OpenGL says: \"" << std::string(message) << "\"" << std::endl;
}


void handleResize(int w, int h) {
    //Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); //Switch to setting the camera perspective
    //Set the camera perspective
        glLoadMatrixf(glm::value_ptr(glm::perspective(45.0f, (float) w / (float) h, 1.f, 100.f)));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); //Reset the camera

}

void drawScene(int eye) {

    glClearColor(.0f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glMatrixMode(GL_PROJECTION);
    //glm::mat4 persp = xr.getProjMatrix(eye, 1.f, 100.f);
    glm::mat4 persp = glm::perspective(45.f, 1.f, 1.f, 100.f);
    glLoadMatrixf(glm::value_ptr(persp));

    glMatrixMode(GL_MODELVIEW);
    glm::mat4 trans = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -10.f});
    glm::mat4 rotZ = glm::rotate(glm::mat4{1.f}, glm::degrees(alphaZ), glm::vec3{0.f, 0.f, 1.f});

    //glm::mat4 camera = xr.getEye2HeadMatrix(eye);
    glm::mat4 camera = glm::mat4{1.f};
    glLoadMatrixf(glm::value_ptr(camera * trans * rotZ));

    //glColor3f(1.f, 1.f, 1.f);
    //glutSolidTeacup(5.f);

    glBegin(GL_TRIANGLES); //triangolo bianco
    glColor3f(1.f, 1.f, 1.f);
    glVertex3f(-0.5f, 0.5f, 1.0f);
    glVertex3f(-1.0f, 1.5f, 1.0f);
    glVertex3f(-1.5f, 0.5f, 1.0f);
    glEnd(); //End triangle coordinates

    glBegin(GL_TRIANGLES); //triangolo rosso
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.5f, 0.5f, .0f);
    glVertex3f(-1.0f, 1.5f, .0f);
    glVertex3f(-1.5f, 0.5f, .0f);
    glEnd(); //End triangle coordinates
}

void displayCB() {
    xr.beginFrame();

    for (int i = 0; i < 2; i++)
    {
        xr.lockSwapchain(i);

        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        glClearColor(r, g, b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        xr.unlockSwapchain(i);
    }

    xr.endFrame();

    frames++;
    //alphaZ+=0.1;

    glutPostRedisplay();
    glutSwapBuffers(); //Send the 3D scene to the screen
}

void timerCallback(int value)
{
    fps = frames;
    cout << "FPS: " << fps << endl;
    frames = 0;

    glutTimerFunc(1000, timerCallback, 0);
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(4, 4);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_DEBUG);

    // Create window:
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutCreateWindow("Minimal OpenVR example");

    // Init all available OpenGL extensions:
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (GLEW_OK != error)
    {
       std::cout << "[ERROR] " << glewGetErrorString(error) << std::endl;
       return -1;
    }
    else
       if (GLEW_VERSION_4_4)
          std::cout << "Driver supports OpenGL 4.4\n" << std::endl;
       else
       {
          std::cout << "[ERROR] OpenGL 4.4 not supported\n" << std::endl;
          return -1;
       }

    // Register OpenGL debug callback:
    glDebugMessageCallback((GLDEBUGPROC) DebugCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);


    glutDisplayFunc(displayCB);
    glutReshapeFunc(handleResize);
    glutTimerFunc(1000, timerCallback, 0);

    XrQuaternionf quat;
    quat.x = quat.y = quat.z = 0.f;
    quat.w = 1.f;
    XrVector3f pos;
    pos.x = pos.y = pos.z = 0.f;
    XrPosef identityPose;
    identityPose.orientation = quat;
    identityPose.position = pos;

    xr.init();
    xr.setReferenceSpace(identityPose, XR_REFERENCE_SPACE_TYPE_LOCAL);
    xr.beginSession();

    std::function<void(XrEventDataBuffer)> cb = [] (XrEventDataBuffer runtimeEvent) {
        std::cout << "EVENT: session state changed ";
        XrEventDataSessionStateChanged* event =
            (XrEventDataSessionStateChanged*)&runtimeEvent;
        XrSessionState state = event->state;

        // it would be better to handle each state change
        bool isVisible = event->state <= XR_SESSION_STATE_FOCUSED;
        std::cout << "to " << state << " Visible: " << isVisible;
        if (event->state >= XR_SESSION_STATE_STOPPING) {
            std::cout <<std::endl << "Session is in state stopping...";
        }
        std::cout << std::endl;
    };

    //xr.setCallback(XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED, cb);

    cout << "risoluzione: " << xr.getHmdIdealHorizRes() << "x" << xr.getHmdIdealVertRes() << endl;
    cout << "Runtime name: " << xr.getRuntimeName() << endl;
    cout << "Manufacture name: " << xr.getManufacturerName() << endl;

    //while(true)
        //displayCB();

    glutMainLoop(); //Start the main loop

    xr.free();

    return 0;
}

#endif
