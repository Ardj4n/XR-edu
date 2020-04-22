#pragma once

#ifdef _WINDOWS
#ifdef SUPSIGL_EXPORTS
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif
#else
#define LIB_API
#endif // _WINDOWS


/// INCLUDE
/// system dependecies (external / system library)
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

/// USING
using namespace std;

/// library dependecies (internal, 3rd party)
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/matrix_inverse.hpp>

/// object dependecies (internal, 1st party)
#include "Object.h"
#include "Vertex.h"
#include "Face.h"
#include "Node.h"
#include "Camera.h"
#include "Light.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "OvoReader.h"
#include "List.h"
#include "shader.h"
#include "Program.h"
#include "Fbo.h"



/// CLASSES
/**
* Supsi-GE, Main graphics engine class
* This class was conceived as a wrapper to OpenGL v1.2,
* therefore any graphics applications needs to include only this file to work
* Written as a singleton, there exists one and only instance of this class during
* the entire lifespan of the application.
* Deployed as a shared object / dynamic library to optimize memory usage and reduce application dependecies
*
* @authors D.Nasi, J.Petralli, D.Calabria, A.Peternier
*
* NOTES: 1) cursor keys, function keys, shift, control, alternate (graphics), super/meta/windows/command, numeric pad keys
*/
class LIB_API Engine
{
private:
	/**
	Constructor
	Since Engine is a singleton, this method has been disabled outside the "Engine" scope
	*/
	Engine();

	/**
	@var active
	Pointer to the main camera object
	*/
	Camera *active;

	/**
	@var maxRenderLights
	The maximum number of light managed by the engine.
	Defaults to 8, with 8 being the maximum absolute limit due to the engine is just a
	OpenGL wrapper. Since OpenGL rarely operates more than 8 concurrent light sources,
	it's necessary to provide a priority system to have more than that.
	For additional details on the priority system
	@see "Light.h"
	*/
	int maxRenderLights = 8; //number of rendered lights, the max value is 8 (due to OpenGL constraints), if a grater value is passed as argument 8 will be used instead


	// Shaders:
	Shader *vs = nullptr;
	Shader *fs = nullptr;
	Shader *program = nullptr;
	Program *pr = nullptr;

	void initShaders();
public:

	/**
	@static Either creates or returns the Engine instance's memory address
	*/
	static Engine& getInstance();

	/**
	Constructor
	Since Engine is a singleton, this method has been disabled outside the "Engine" scope
	*/
	Engine(Engine const&) = delete;

	/**
	Assignment operator "=" (equals)
	Since Engine is a singleton, this operator has been disabled outside the "Engine" scope
	*/
	void operator=(Engine const&) = delete;

	/**
	Initialize the engine and creates the internal OpenGL context
	@param argc Count of CLI arguments
	@param argv Vector of CLI arguments
	@param title The window's title
	*/
	void init(int argc, char *argv[], string title);

	/**
	Reads and loads a graphic scene from a OVO file. Returns such scene graph.
	@param scene The path to the OVO file
	*/
	Node* load(string scene);

	/**
	Cleans the memory buffers. Particularly the OpenGL's depth and color buffer
	*/
	void clear();

	/**
	Switches the rendering buffers (Frame and back).
	Used to untie the application's frame rate from the screen's refresh rate.
	(e.g. A 60 Hz screen does not forces a 60 FPS application)
	*/
	void swap();

	/**
	Creates the list to be rendered by renderScene()
	@param node The current Node to be added to the scene's graph
	*/
	List* createList(Node* node);

	/**
	Renders the scene from the indicated node and returns the corresponding tree.
	The rendering process is:
	- disable all active lights (will be reactivated at selection time, see Light.h)
	- Sets the active projection matrix
	- renders with the active camera
	- If necessary, renders 2D text object (e.g for GUI / user help
	@param node The current Node to be added to the scene's graph
	*/
	List* renderScene(Node* node);

	/**
	Renders the scene from the root of the scene.
	Calls "renderScene(Node* node)" on each of the nodes.
	*/
	void renderScene(List* list);

	/**
	Sets a new active camera
	@param camera Pinter to the newly selected camera
	*/
	void setActiveCamera(Camera* camera);

	/**
	Returns the active camera
	*/
	Camera* getActiveCamera();

	/**
	Returns the maximum number of lights
	*/
	int getMaxRenderLights();

	/**
	Sets the maximum number of lights to "n",
	provided that "n" is lower than 8. Else it's set to 8.
	@param n The new maximum number of light. Accepts integer values in the [1;8] range
	*/
	void setMaxRenderLights(int n);


	/**
	Toggles the "polygons filling routine" of OpenGL (blocks color interpolation),
	resulting in either the fully rendered scene, with visible materials and textures, 
	or only the colored polygons' edges.
	*/
	void wireframeSwitch();


	/**
	Sets the window background color.
	This method receives the color as its single components Red, Green and Blue
	@param r The RED color component
	@param g The GREEN color component
	@param b The BLUE color component
	*/
	void clearColor(float r, float g, float b);

	/**
	Returns the window's width
	*/
	int getWindowWidth();

	/**
	Returns the window's height
	*/
	int getWindowHeight();

	/**
	Sets the window's aspect ratio
	@param x The window's pixel horizontal offset
	@param y The window's pixel vertical offset
	@param width The window's pixel horizontal lenght
	@param height The window's pixel vertical lenght
	*/
	void setViewport(int x, int y, int width, int height);

	/**
	Starts the FreeGLUT rendering loop.
	*/
	void startEventLoop();

	/**
	Used to rerender the scene without risking any memory errors due to recursion.
	*/
	void redisplay();

	/**
	Used to start/stop printing FPS.
	*/
	void showFps();

	/**
	Callback functions
	The following methods are used to bind callbacks to events (sets the event's handler)
	/!\ All parameters are function pointers. Therefore
	/!\ ALL PASSED FUNCTION MUST RESPECT THE INDICATED FIRM TO WORK
	/!\ e.g: void(*reshapeFunc)(int, int) --> <any, prefer void> myReshapeFunc(int param1, int param2)
	*/

	/**
	Binds the "reshapeFunc" function to the [Window reshape] event
	*/
	void reshape(void(*reshapeFunc)(int, int));

	/**
	Binds the "displayFunc" function to the [Content display] event
	*/
	void display(void(*displayFunc)());
	/**
	Binds the "keyboardFunc" function to the [Keyboard input > alfanumeric keys] event
	*/
	void keyboard(void(*keyboardFunc)(unsigned char, int, int));

	/**
	Binds the "specialFunc" function to the [Keyboard input > other(1) keys] event
	*/
	void specialKeyboard(void(*specialFunc)(int, int, int));

	/**
	Binds the "mouseFunc" function to the [Mouse input > mouse movement] event
	*/
	void mouseMoved(void(*mouseFunc)(int, int));

	/**
	Binds the "wheelFunc" function to the [Mouse input > scroll wheel movement] event
	*/
	void mouseWheel(void(*wheelFunc)(int, int, int, int));

	bool initOpenXR();
	void renderOpenXR(Node* n);

	Program* getProgram();
	Shader* getShader();
};