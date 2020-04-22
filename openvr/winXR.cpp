#include <GL/glew.h>

#include "oxr.h"
//#include "glScene.h"

#include <iostream>
#include <string>
#include <functional>

using namespace std;

OvXR xr{};
//Scene scene{};

int frames = 0;
int fps = 0;
float alphaZ = 0.f;

void __stdcall GlDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	std::cout << "OpenGL says: \"" << std::string(message) << "\"" << std::endl;
}


void handleResize(int w, int h) {
	glutPostRedisplay();
	return;
	//Tell OpenGL how to convert from coordinates to pixel values
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); //Switch to setting the camera perspective
	//Set the camera perspective
	glLoadMatrixf(glm::value_ptr(glm::perspective(45.0f, (float)w / (float)h, 1.f, 100.f)));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //Reset the camera

}

void displayCB() {
	xr.beginFrame();

	for (int i = 0; i < 2; i++)
	{
		xr.lockSwapchain(i);

		//scene.renderScene();
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		glClearColor(r, g, b, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		xr.unlockSwapchain(i);
	}

	xr.endFrame();

	frames++;

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
	glDebugMessageCallback((GLDEBUGPROC)GlDebugCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glutDisplayFunc(displayCB);
	glutReshapeFunc(handleResize);
	glutTimerFunc(1000, timerCallback, 0);

	xr.init();

	XrQuaternionf quat;
	quat.x = quat.y = quat.z = 0.f;
	quat.w = 1.f;
	XrVector3f pos;
	pos.x = pos.y = pos.z = 0.f;
	XrPosef identityPose;
	identityPose.orientation = quat;
	identityPose.position = pos;
	cout << xr.setReferenceSpace(identityPose, XR_REFERENCE_SPACE_TYPE_LOCAL) << endl;

	xr.beginSession();

	std::function<void(XrEventDataBuffer)> cb = [](XrEventDataBuffer runtimeEvent) {
		std::cout << "EVENT: session state changed ";
		XrEventDataSessionStateChanged* event =
			(XrEventDataSessionStateChanged*)&runtimeEvent;
		XrSessionState state = event->state;

		// it would be better to handle each state change
		bool isVisible = event->state <= XR_SESSION_STATE_FOCUSED;
		std::cout << "to " << state << " Visible: " << isVisible;
		if (event->state >= XR_SESSION_STATE_STOPPING) {
			std::cout << std::endl << "Session is in state stopping...";
		}
		std::cout << std::endl;
	};

	//xr.setCallback(XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED, cb);
	
	cout << "risoluzione: " << xr.getHmdIdealHorizRes() << "x" << xr.getHmdIdealVertRes() << endl;
	cout << "Runtime name: " << xr.getRuntimeName() << endl;
	cout << "Manufacturer name: " << xr.getManufacturerName() << endl;

	//scene.init();

	glutMainLoop();


	xr.free();

	return 0;
}
