/**
 * @file		main.cpp
 * @brief	OpenXR kickstart with OGL 4.4
 *
 * @author	Achille Peternier (C) SUPSI [achille.peternier@supsi.ch]
 */



 //////////////
 // #INCLUDE //
 //////////////

	// GLM:   
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// GLEW:
#include <GL/glew.h>

// FreeGLUT:
#include <GL/freeglut.h>   

// C/C++:
#include <iostream>
#include <string>
#include <stdio.h>

// Shader class:
#include "shader.h"

// Fbo class:
#include "fbo.h"

// OpenXR:
#include "oxr.h"



/////////////
// #DEFINE //
/////////////

#ifdef _WINDOWS
    #define STD_CALL    __stdcall
#else
    #define STD_CALL
#endif

  // Window size:
#define APP_WINDOWSIZEX   800
#define APP_WINDOWSIZEY   600 

// Define for verbose logging:
//#define APP_VERBOSE



/////////////
// GLOBALS //
/////////////

  // Context information:
int windowId;

// Matrices:   
glm::mat4 ortho;

// Vertex buffers:   
unsigned int globalVao = 0;
unsigned int boxVertexVbo = 0;
unsigned int boxTexCoordVbo = 0;

glm::vec3 color{ 0.f };

// Textures:
unsigned int eyeTexId[OvXR::EYE_LAST] = { 0, 0 };

// FBO:   
Fbo *eyeFbo[OvXR::EYE_LAST] = { nullptr, nullptr };

// Passthrough shader:
Shader *passthroughVs = nullptr;
Shader *passthroughFs = nullptr;
Shader *passthroughShader = nullptr;
int ptProjLoc = -1;
int ptMvLoc = -1;

// OpenXR interface:
OvXR *oxr = nullptr;



/////////////
// SHADERS //
/////////////

// Passthrough shader with texture mapping:
const char *passthroughVS = R"(
   #version 440 core

   // Uniforms:
   uniform mat4 projection;
   uniform mat4 modelview;   

   // Attributes:
   layout(location = 0) in vec2 in_Position;   
   layout(location = 2) in vec2 in_TexCoord;

   // Varying:   
   out vec2 texCoord;

   void main(void)
   {      
      gl_Position = projection * modelview * vec4(in_Position, 0.0f, 1.0f);    
      texCoord = in_TexCoord;
   }
)";

const char *passthroughFS = R"(
   #version 440 core
   
   // Varying:
   in vec2 texCoord;

   // Output color:
   out vec4 fragOutput;   

   // Texture mapping:
   layout(binding = 0) uniform sampler2D texSampler;

   void main(void)   
   {        
      fragOutput = texture(texSampler, texCoord);            
   }
)";



///////////////
// CALLBACKS //
///////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	  
/**
 * Debug message callback for OpenGL. See https://www.opengl.org/wiki/Debug_Output
 */
void STD_CALL DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	std::cout << "OpenGL says: \"" << std::string(message) << "\"" << std::endl;
}

void renderScene()
{
	glClearColor(color.x, color.y, color.z, 1.f);
	//glClearColor(1.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This is the main rendering routine automatically invoked by FreeGLUT.
 */
void displayCallback()
{
	// Clear the context back buffer:   
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Store the current viewport size:
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	// Begin frame submission:
	oxr->beginFrame();

	// Render each eye into its OpenXR Texture:
	for (int c = 0; c < OvXR::EYE_LAST; c++)
	{
		OvXR::OvEye curEye = (OvXR::OvEye) c;

		// Acquire swapchain image:
		oxr->lockSwapchain(curEye);

		// Retrive updated projection and modelview matrices:
		glm::mat4 projMat = oxr->getProjMatrix(curEye, 1.0f, 1024.0f);
		glm::mat4 eyeMatrix = oxr->getEyeModelviewMatrix(curEye);

#ifdef APP_VERBOSE   
		std::cout << "Eye " << c << " proj matrix: " << glm::to_string(projMat) << std::endl;
		std::cout << "Eye " << c << " modelview matrix: " << glm::to_string(eyeMatrix) << std::endl;
#endif

		// Put your 3D rendering here...   
		/// ---------------
		renderScene();
		/// ---------------

		// Release swapchain image:
		oxr->unlockSwapchain(curEye);
	}
	
	// Submit frames:
	oxr->endFrame();

#ifdef _DEBUG
	// Render each eye into its FBO:
	for (int c = 0; c < OvXR::EYE_LAST; c++)
	{
		OvXR::OvEye curEye = (OvXR::OvEye) c;

		// Retrive updated projection and modelview matrices:
		glm::mat4 projMat = oxr->getProjMatrix(curEye, 1.0f, 1024.0f);
		glm::mat4 eyeMatrix = oxr->getEyeModelviewMatrix(curEye);
		
		// Put your 3D rendering to FBO here...      
		eyeFbo[c]->render();
		
		/// ---------------
		renderScene();
		/// ---------------
	}
	
	// Done with the FBOs, go back to rendering into the default context buffers:   
	Fbo::disable();
	glViewport(0, 0, prevViewport[2], prevViewport[3]);


	///////////////////////////////////////////
	// 2D rendering (goes to the main context):

	// Set a matrix for the left "eye":    
	glm::mat4  f = glm::mat4(1.0f);

	// Setup the passthrough shader:
	passthroughShader->render();
	passthroughShader->setMatrix(ptProjLoc, ortho);
	passthroughShader->setMatrix(ptMvLoc, f);

	// Bind the FBO buffer as texture and render:
	glBindTexture(GL_TEXTURE_2D, eyeTexId[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Set matrix for the other eye and render another quad:
	f = glm::translate(glm::mat4(1.0f), glm::vec3(APP_WINDOWSIZEX / 2.0f, 0.0f, 0.0f));
	passthroughShader->setMatrix(ptMvLoc, f);
	glBindTexture(GL_TEXTURE_2D, eyeTexId[1]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif // _DEBUG

	// Swap buffers:
	glutSwapBuffers();

	// Force rendering refresh:
	glutPostWindowRedisplay(windowId);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This callback is invoked each time the window gets resized (and once also when created).
 * @param width new window width
 * @param height new window height
 */
void reshapeCallback(int width, int height)
{
	// ... ignore the params, we want a fixed-size window

	// Update matrices:  
	ortho = glm::ortho(0.0f, (float)APP_WINDOWSIZEX, 0.0f, (float)APP_WINDOWSIZEY, -1.0f, 1.0f);

	// (bad) trick to avoid window resizing:
	if (width != APP_WINDOWSIZEX || height != APP_WINDOWSIZEY)
		glutReshapeWindow(APP_WINDOWSIZEX, APP_WINDOWSIZEY);
}

void timerCallback(int value)
{
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	color = glm::vec3{ r, g, b };
	
	glutTimerFunc(1000, timerCallback, 0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Window close callback. Required to shutdown OpenXR before the context is released.
 */
void closeCallback()
{
	// Free OpenXR:   
	oxr->free();
	delete oxr;

	// Free OpenGL stuff:
	glDeleteBuffers(1, &boxVertexVbo);
	glDeleteBuffers(1, &boxTexCoordVbo);
	glDeleteVertexArrays(1, &globalVao);
	for (int c = 0; c < 2; c++)
	{
		delete eyeFbo[c];
		glDeleteTextures(1, &eyeTexId[c]);
	}
	delete passthroughShader;
	delete passthroughFs;
	delete passthroughVs;
}



//////////
// MAIN //
//////////

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
	// Credits:
	std::cout << "OpenGL 4.4 and OpenXR, A. Peternier (C) SUPSI" << std::endl;
	std::cout << std::endl;

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
	windowId = glutCreateWindow("Minimal OpenXR example");

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
	glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	// Init OpenXR:   
	oxr = new OvXR();
	if (oxr->init() == false)
	{
		std::cout << "[ERROR] Unable to init OpenXR" << std::endl;
		delete oxr;
		return -2;
	}
	
	XrQuaternionf quat;
	quat.x = quat.y = quat.z = 0.f;
	quat.w = 1.f;
	XrVector3f pos;
	pos.x = pos.y = pos.z = 0.f;
	XrPosef identityPose;
	identityPose.orientation = quat;
	identityPose.position = pos;

	oxr->setReferenceSpace(identityPose, XR_REFERENCE_SPACE_TYPE_LOCAL);

	oxr->beginSession();

	// Report some info:
	std::cout << "   Manufacturer . . :  " << oxr->getManufacturerName() << std::endl;
	std::cout << "   Runtime Name . . :  " << oxr->getRuntimeName() << std::endl;
	std::cout << "   Resolution   . . :  " << oxr->getHmdIdealHorizRes() << "x" << oxr->getHmdIdealVertRes() << std::endl;
	//std::cout << "   Model number . . :  " << oxr->getModelNumber() << std::endl;

	// Set callback functions:
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);
	glutCloseFunc(closeCallback);
	glutTimerFunc(1000, timerCallback, 0);

	// Init VAO:   
	glGenVertexArrays(1, &globalVao);
	glBindVertexArray(globalVao);

	// Compile shader:
	passthroughVs = new Shader();
	passthroughVs->loadFromMemory(Shader::TYPE_VERTEX, passthroughVS);
	passthroughFs = new Shader();
	passthroughFs->loadFromMemory(Shader::TYPE_FRAGMENT, passthroughFS);
	passthroughShader = new Shader();
	passthroughShader->build(passthroughVs, passthroughFs);
	passthroughShader->render();

	// Bind params and get uniform locations:
	passthroughShader->bind(0, "in_Position");
	passthroughShader->bind(2, "in_TexCoord");
	ptProjLoc = passthroughShader->getParamLocation("projection");
	ptMvLoc = passthroughShader->getParamLocation("modelview");

	// Create a 2D box covering half screen:
	glm::vec2 *boxPlane = new glm::vec2[4];
	boxPlane[0] = glm::vec2(0.0f, 0.0f);
	boxPlane[1] = glm::vec2(APP_WINDOWSIZEX / 2.0f, 0.0f);
	boxPlane[2] = glm::vec2(0.0f, APP_WINDOWSIZEY);
	boxPlane[3] = glm::vec2(APP_WINDOWSIZEX / 2.0f, APP_WINDOWSIZEY);

	// Copy data into VBOs:
	glGenBuffers(1, &boxVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), boxPlane, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	delete[] boxPlane;

	// ...same for tex coords:
	glm::vec2 *texCoord = new glm::vec2[4];
	texCoord[0] = glm::vec2(0.0f, 0.0f);
	texCoord[1] = glm::vec2(1.0f, 0.0f);
	texCoord[2] = glm::vec2(0.0f, 1.0f);
	texCoord[3] = glm::vec2(1.0f, 1.0f);
	glGenBuffers(1, &boxTexCoordVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxTexCoordVbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), texCoord, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	delete[] texCoord;

	// Load FBOs and their textures:
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	int fboSizeX = oxr->getHmdIdealHorizRes();
	int fboSizeY = oxr->getHmdIdealVertRes();
	std::cout << "   Ideal resolution :  " << fboSizeX << "x" << fboSizeY << std::endl;

	for (int c = 0; c < OvXR::EYE_LAST; c++)
	{
		// Fill texture with static color:
		unsigned char *data = new unsigned char[fboSizeX * fboSizeY * 4];
		for (int d = 0; d < fboSizeX * fboSizeY; d++)
		{
			if (c == 0)
			{
				data[d * 4] = 255;
				data[d * 4 + 1] = 0;
				data[d * 4 + 2] = 0;
			}
			else
			{
				data[d * 4] = 0;
				data[d * 4 + 1] = 255;
				data[d * 4 + 2] = 255;
			}
			data[d * 4 + 3] = 255;
		}

		// Init texture:
		glGenTextures(1, &eyeTexId[c]);
		glBindTexture(GL_TEXTURE_2D, eyeTexId[c]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboSizeX, fboSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // GL_RGBA8 IS IMPORTANT!!
		delete[] data;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Init FBO:
		eyeFbo[c] = new Fbo();
		eyeFbo[c]->bindTexture(0, Fbo::BIND_COLORTEXTURE, eyeTexId[c]);
		eyeFbo[c]->bindRenderBuffer(1, Fbo::BIND_DEPTHBUFFER, fboSizeX, fboSizeY);
		if (!eyeFbo[c]->isOk())
			std::cout << "[ERROR] Invalid FBO" << std::endl;
	}

	Fbo::disable();
	glViewport(0, 0, prevViewport[2], prevViewport[3]);

	// Enter the main FreeGLUT processing loop:
	glutMainLoop();

	// Done:
	std::cout << "[application terminated]" << std::endl;
	return 0;
}
