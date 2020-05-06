/**
 * @file		main.cpp
 * @brief	OpenXR kickstart with OGL 4.4
 *
 * @author	Ardjan Doci, Nicolò Rubattu
 */


 //////////////
 // #INCLUDE //
 //////////////

// GLEW:
#include <GL/glew.h>

// FreeGLUT:
#include <GL/freeglut.h>   

// C/C++:
#include <iostream>
#include <string>

// OpenXR:
#include "oxr.h"

#include "MockRenderer.h"

#ifdef _WINDOWS
#include "DirectXRenderer.h"
#else
#include "OpenGLRenderer.h"
#endif // _WINDOWS

PlatformRenderer * platformRendererBuilder()
{
#ifdef _WINDOWS
	return new DirectXRenderer{};
#else
	return new OpenGLRenderer{};
#endif // _WINDOWS
}

#define APP_WINDOWSIZEX 400
#define APP_WINDOWSIZEY 400

// Context information:
int windowId;

bool active = false;

// OpenXR interface:
OvXR *oxr = nullptr;

#define  ASSERT_WITH_MESSAGE(res, msg)				\
	if (!(res) ) {									\
		std::cerr << msg << std::endl;				\
		std::cerr << "TEST FAILED" << std::endl;	\
		abort();									\
    }

void testOpenXR() 
{
	MockRenderer* pr = new MockRenderer{ platformRendererBuilder() };	
    ASSERT_WITH_MESSAGE(!pr->getRenderExtensionName().empty(), "extension name must not be empty")

	oxr = new OvXR();

    ASSERT_WITH_MESSAGE(oxr->getInstane() == XR_NULL_HANDLE, "xrInstance must be null")
    ASSERT_WITH_MESSAGE(oxr->getSession() == XR_NULL_HANDLE, "xrSession must be null")

	oxr->setPlatformRenderer(pr);

    ASSERT_WITH_MESSAGE(oxr->init(), "failed OvXR initialization")

    ASSERT_WITH_MESSAGE(oxr->getInstane() != XR_NULL_HANDLE, "xrInstance must not be null")
    ASSERT_WITH_MESSAGE(oxr->getSession() != XR_NULL_HANDLE, "xrSession must not be null")

	XrQuaternionf quat;
	quat.x = quat.y = quat.z = 0.f;
	quat.w = 1.f;
	XrVector3f pos;
	pos.x = pos.y = pos.z = 0.f;
	XrPosef identityPose;
	identityPose.orientation = quat;
	identityPose.position = pos;

	bool res = oxr->setReferenceSpace(identityPose, XR_REFERENCE_SPACE_TYPE_LOCAL);
    ASSERT_WITH_MESSAGE(res, "failed setReferenceSpace")

	auto cb = [](XrEventDataBuffer) { active = true; };
	oxr->setCallback(XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED, cb);

	res = oxr->beginSession();
    ASSERT_WITH_MESSAGE(res, "failed beginSession")

	for (int i = 0; i < 100; i++) {
		res = oxr->beginFrame();
        ASSERT_WITH_MESSAGE(res, "failed beginFrame")

		for (int j = 0; j < OvXR::EYE_LAST; j++) {
			OvXR::OvEye eye = (OvXR::OvEye) j;

			res = oxr->lockSwapchain(eye);
            ASSERT_WITH_MESSAGE(res, "failed lockSwapchain")

			res = oxr->unlockSwapchain(eye);
            ASSERT_WITH_MESSAGE(res, "failed unlockSwapchain")
		}

		res = oxr->endFrame();
        ASSERT_WITH_MESSAGE(res, "failed endFrame")
	}

    ASSERT_WITH_MESSAGE(active, "callback must have been called")
	
	res = oxr->endSession();
    ASSERT_WITH_MESSAGE(res, "failed endSession")
	
	std::map<std::string, int> map = pr->getMap();

    ASSERT_WITH_MESSAGE(map["initSwapchains"] == 1			, "wrong number of 'initSwapchains' calls")
    ASSERT_WITH_MESSAGE(map["initPlatformResources"] == 1	, "wrong number of 'initPlatformResources' calls")
    ASSERT_WITH_MESSAGE(map["getSwapchain"] == 400			, "wrong number of 'getSwapchain' calls")
    ASSERT_WITH_MESSAGE(map["getRenderExtensionName"] == 2	, "wrong number of 'getRenderExtensionName' calls")
    ASSERT_WITH_MESSAGE(map["getGraphicsBinding"] == 1		, "wrong number of 'getGraphicsBinding' calls")
    ASSERT_WITH_MESSAGE(map["beginEyeFrame"] == 200			, "wrong number of 'beginEyeFrame' calls")
    ASSERT_WITH_MESSAGE(map["endEyeFrame"] == 200			, "wrong number of 'endEyeFrame' calls")	


	res = oxr->free();
	ASSERT_WITH_MESSAGE(res, "failed free");

	delete oxr;
}


void initGL(int argc, char *argv[]) {
	// Init GLUT and set some context flags:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(4, 4);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_DEBUG);

	// Create window:
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(APP_WINDOWSIZEX, APP_WINDOWSIZEY);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	windowId = glutCreateWindow("OpenXR tests");

	// Init all available OpenGL extensions:
	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if (GLEW_OK != error)
	{
		std::cout << "[ERROR] " << glewGetErrorString(error) << std::endl;
		exit(-1);
	}
	else
		if (GLEW_VERSION_4_4)
			std::cout << "Driver supports OpenGL 4.4\n" << std::endl;
		else
		{
			std::cout << "[ERROR] OpenGL 4.4 not supported\n" << std::endl;
			exit(-1);
		}
}

int main(int argc, char *argv[])
{
	initGL(argc, argv);

	testOpenXR();

	// Done:
	std::cout << std::endl;
	std::cout << "+------------------------+" << std::endl;
	std::cout << "|--- ALL TESTS PASSED ---|" << std::endl;
	std::cout << "+------------------------+" << std::endl;
	return 0;
}
