#ifdef _WINDOWS
#include <Windows.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * DLL entry point. Avoid to rely on it for easier code portability (Linux doesn't use this method).
 * @param instDLL handle
 * @param reason reason
 * @param _reserved reserved
 * @return true on success, false on failure
 */
int APIENTRY DllMain(HANDLE instDLL, DWORD reason, LPVOID _reserved)
{
	// Check use:
	switch (reason)
	{
		///////////////////////////
	case DLL_PROCESS_ATTACH: //
		break;


		///////////////////////////
	case DLL_PROCESS_DETACH: //
		break;
	}

	// Done:
	return true;
}
#endif
#include "Engine.h"
#include <GL/glew.h>
#include <GL/wglew.h>
#include "GL/freeglut.h"
#include <FreeImage.h>

#include "oxr.h"

int windowId;
glm::mat4 textCamera;
int frames = 0;
List list{};
bool fpsFlag = true;
GLuint globalVao;
OvXR xr{"Transformer"};

////////////////////////////////////////////////////////////// SERIE FBO

// Window size:
#define APP_WINDOWSIZEX   1024
#define APP_WINDOWSIZEY   512
#define APP_FBOSIZEX      APP_WINDOWSIZEX / 2
#define APP_FBOSIZEY      APP_WINDOWSIZEY

// Enums:
enum Eye
{
	EYE_LEFT = 0,
	EYE_RIGHT = 1,

	// Terminator:
	EYE_LAST,
};

// Matrices:   
glm::mat4 fboPerspective;
glm::mat4 ortho;

// Textures:
unsigned int texId = 0;
unsigned int fboTexId[EYE_LAST] = { 0, 0 };

// FBO:      
Fbo *fbo[EYE_LAST] = { nullptr, nullptr };

// Passthrough shader:
Shader *passthroughVs = nullptr;
Shader *passthroughFs = nullptr;
Program *passthroughShader = nullptr;
int ptProjLoc = -1;
int ptMvLoc = -1;
int ptColorLoc = -1;


unsigned int boxVertexVbo = 0;
unsigned int boxTexCoordVbo = 0;

////////////////////////////////////////////////////////////// XXXXXXXXX
static void screenshot_ppm(const char *filename, unsigned int width,
	unsigned int height, GLubyte **pixels) {
	size_t i, j, cur;
	const size_t format_nchannels = 3;
	FILE *f = fopen(filename, "w");
	fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
	*pixels = (GLubyte*)realloc(*pixels, format_nchannels * sizeof(GLubyte) * width * height);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			cur = format_nchannels * ((height - i - 1) * width + j);
			fprintf(f, "%3d %3d %3d ", (*pixels)[cur], (*pixels)[cur + 1], (*pixels)[cur + 2]);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

/**
 * This callback is invoked once each 3 seconds in order to calculate fps
 * @param value passepartout value
 */
void timerCallback(int value)
{
	int fps = frames;
	frames = 0;
	if(fpsFlag)
		std::cout << "fps: " << fps << std::endl;

	// Register the next update:
	glutTimerFunc(1000, timerCallback, 0);
}

void closeCallback()
{
	std::cout << "Requestin Exit" << std::endl;

	xr.endSession();
	xr.free();
	
	// Free OpenGL stuff:
	glDeleteBuffers(1, &boxVertexVbo);
	glDeleteBuffers(1, &boxTexCoordVbo);
	glDeleteVertexArrays(1, &globalVao);
	for (int c = 0; c < 2; c++)
	{
		delete fbo[c];
		glDeleteTextures(1, &fboTexId[c]);
	}
	delete passthroughShader;
	delete passthroughFs;
	delete passthroughVs;
}



Engine::Engine()
{
}


Engine LIB_API &Engine::getInstance()
{
	static Engine instance; // Guaranteed to be destroyed.
						  // Instantiated on first use.
	return instance;
}

#ifdef _DEBUG
// Very simple debug callback: 
void __stdcall DebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	GLvoid* userParam)
{
	printf("OpenGL says: %s\n", message);
}
#endif // _DEBUG

void logInfo()
{
	// Log context properties:
	std::cout << "OpenGL properties:" << std::endl;
	std::cout << "   Vendor . . . :  " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "   Driver . . . :  " << glGetString(GL_RENDERER) << std::endl;

	int oglVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &oglVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &oglVersion[1]);
	std::cout << "   Version  . . :  " << glGetString(GL_VERSION) << " [" << oglVersion[0] << "." << oglVersion[1] << "]" << std::endl;

	int oglContextProfile;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &oglContextProfile);
	if (oglContextProfile & GL_CONTEXT_CORE_PROFILE_BIT)
		std::cout << "                :  " << "Core profile" << std::endl;
	if (oglContextProfile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
		std::cout << "                :  " << "Compatibility profile" << std::endl;

	int oglContextFlags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &oglContextFlags);
	if (oglContextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
		std::cout << "                :  " << "Forward compatible" << std::endl;
	if (oglContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
		std::cout << "                :  " << "Debug flag" << std::endl;
	if (oglContextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
		std::cout << "                :  " << "Robust access flag" << std::endl;
	if (oglContextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
		std::cout << "                :  " << "No error flag" << std::endl;

	std::cout << "   GLSL . . . . :  " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << std::endl;
}

const char *vertShader = R"(
   #version 440 core

   uniform mat4 projection;
   uniform mat4 modelview;	
   uniform mat3 normalMatrix;

   layout(location = 0) in vec3 in_Position;
   layout(location = 1) in vec3 in_Normal;
	layout(location = 2) in vec2 in_TexCoord;

   out vec4 fragPosition;
   out vec3 normal;
   out float dist;
	out vec2 texCoord; 

   void main(void)
   {
      fragPosition = modelview * vec4(in_Position, 1.0f);
      gl_Position = projection * fragPosition;      
      normal = normalMatrix * in_Normal;
		dist = abs(gl_Position.z / 100.0f);
		texCoord = in_TexCoord; 
   }
)";


////////////////////////////
const char *fragShader = R"(
   #version 440 core
	
   #define MAX_LIGHTS 16

   in vec4 fragPosition;
   in vec3 normal; 
	in vec2 texCoord;  
   in float dist;   

   out vec4 fragOutput;

	// Texture mapping: 
	layout(binding = 0) uniform sampler2D texSampler;

   // Material properties:
   uniform vec4 matEmission;
   uniform vec4 matAmbient;
   uniform vec4 matDiffuse;
   uniform vec4 matSpecular;
   uniform float matShininess;

   // Light properties:
   uniform int lightNumber;
   uniform vec3 arrLightPosition[MAX_LIGHTS];
   uniform vec4 arrLightAmbient[MAX_LIGHTS];
   uniform vec4 arrLightDiffuse[MAX_LIGHTS];
   uniform vec4 arrLightSpecular[MAX_LIGHTS];

   void main(void)
   {      
		// Texture element: 
		vec4 texel = texture(texSampler, texCoord); 

      // Ambient term:
      vec4 fragColor = matEmission + matAmbient * arrLightAmbient[0];
		
	  vec3 _normal = normalize(normal);
	  for(int i = 0; i < lightNumber; i++) 
	  {
		  // Diffuse term:
		  vec3 lightDirection = normalize(arrLightPosition[i] - fragPosition.xyz);      
		  float nDotL = dot(lightDirection, _normal);   
		  if (nDotL > 0.0f)
		  {
			 fragColor += matDiffuse * nDotL * arrLightDiffuse[i];
      
			 // Specular term:
			 vec3 halfVector = normalize(lightDirection + normalize(-fragPosition.xyz));                     
			 float nDotHV = dot(_normal, halfVector);         
			 fragColor += matSpecular * pow(nDotHV, matShininess) * arrLightSpecular[i];
		  } 
      }
     
      // Final color:
		fragOutput = texel * vec4(fragColor.xyz, 1.0f);
   }
)";

// Passthrough shader with texture mapping:
const char *passthroughVertShader = R"(
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

const char *passthroughFragShader = R"(
   #version 440 core
   
   in vec2 texCoord;
   
   uniform vec4 color;

   out vec4 fragOutput;   

   // Texture mapping:
   layout(binding = 0) uniform sampler2D texSampler;

   void main(void)   
   {  
      // Texture element:
      vec4 texel = texture(texSampler, texCoord);      
      
      // Final color:
      fragOutput = color * texel;       
   }
)";



void Engine::initShaders()
{
	// Compile vertex shader:
	vs = new Shader();
	vs->loadFromMemory(Shader::TYPE_VERTEX, vertShader);

	// Compile fragment shader:
	fs = new Shader();
	fs->loadFromMemory(Shader::TYPE_FRAGMENT, fragShader);

	pr = new Program{ vs, fs };

	pr->build();
	pr->render();
	pr->bindLayoutLocation(0, "in_Position");
	pr->bindLayoutLocation(1, "in_Normal");
	pr->bindLayoutLocation(2, "in_TexCoord");

	pr->bindLocation(Location::PROJECTION_MATRIX, "projection");
	pr->bindLocation(Location::MODLVIEW_MATRIX, "modelview");
	pr->bindLocation(Location::NORMAL_MATRIX, "normalMatrix");

	pr->bindLocation(Location::MATERIAL_AMBIENT, "matAmbient");
	pr->bindLocation(Location::MATERIAL_EMISSIVE, "matEmission");
	pr->bindLocation(Location::MATERIAL_DIFFUSE, "matDiffuse");
	pr->bindLocation(Location::MATERIAL_SPECULAR, "matSpecular");
	pr->bindLocation(Location::MATERIAL_SHININESS, "matShininess");

	pr->bindLocation(Location::LIGHT_TOTAL, "lightNumber");
	pr->bindLocation(Location::LIGHT_ARRAY_POSITION, "arrLightPosition");
	pr->bindLocation(Location::LIGHT_ARRAY_DIFFUSE, "arrLightDiffuse");
	pr->bindLocation(Location::LIGHT_ARRAY_SPECULAR, "arrLightSpecular");
	pr->bindLocation(Location::LIGHT_ARRAY_AMBIENT, "arrLightAmbient");


	passthroughVs = new Shader();
	passthroughVs->loadFromMemory(Shader::TYPE_VERTEX, passthroughVertShader);

	passthroughFs = new Shader();
	passthroughFs->loadFromMemory(Shader::TYPE_FRAGMENT, passthroughFragShader);

	passthroughShader = new Program{ passthroughVs, passthroughFs };
	passthroughShader->build();
	passthroughShader->render();

	// Bind params:
	passthroughShader->bindLayoutLocation(0, "in_Position");
	passthroughShader->bindLayoutLocation(2, "in_TexCoord");

	passthroughShader->bindLocation(Location::PROJECTION_MATRIX, "projection");
	passthroughShader->bindLocation(Location::MODLVIEW_MATRIX, "modelview");
	passthroughShader->bindLocation(Location::COLOR, "color");
}

Program LIB_API * Engine::getProgram()
{
	return pr;
}

Shader LIB_API  * Engine::getShader()
{
	return program;
}

void loadFboAndItsTexture() {
	// Load FBO and its texture:
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	for (int c = 0; c < EYE_LAST; c++)
	{
		int fboSizeX = APP_WINDOWSIZEX;
		int fboSizeY = APP_WINDOWSIZEY;
		glGenTextures(1, &fboTexId[c]);
		glBindTexture(GL_TEXTURE_2D, fboTexId[c]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboSizeX, fboSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		fbo[c] = new Fbo();
		fbo[c]->bindTexture(0, Fbo::BIND_COLORTEXTURE, fboTexId[c]);
		fbo[c]->bindRenderBuffer(1, Fbo::BIND_DEPTHBUFFER, fboSizeX, fboSizeY);
		if (!fbo[c]->isOk())
			std::cout << "[ERROR] Invalid FBO" << std::endl;
	}
	Fbo::disable();
	glViewport(0, 0, prevViewport[2], prevViewport[3]);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void setUpBox() {
	////////////////////////////
	// Build passthrough shader:
	glGenVertexArrays(1, &globalVao);
	glBindVertexArray(globalVao);

	// Create a 2D box for screen rendering:
	glm::vec2 *boxPlane = new glm::vec2[4];
	boxPlane[0] = glm::vec2(0.0f, 0.0f);
	boxPlane[1] = glm::vec2(APP_FBOSIZEX, 0.0f);
	boxPlane[2] = glm::vec2(0.0f, APP_FBOSIZEY);
	boxPlane[3] = glm::vec2(APP_FBOSIZEX, APP_FBOSIZEY);

	// Copy data into VBOs:
	glGenBuffers(1, &boxVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), boxPlane, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	delete[] boxPlane;

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
}

void LIB_API Engine::init(int argc, char *argv[], string title)
{
	FreeImage_Initialise();

	// FreeGLUT can parse command-line params, in case:
	glutInit(&argc, argv);

	// Init context:
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitContextVersion(4, 4);
	glutInitContextProfile(GLUT_CORE_PROFILE);

#ifdef _DEBUG
	// Enable the debug flag 
	glutInitContextFlags(GLUT_DEBUG);
#endif // _DEBUG

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(APP_WINDOWSIZEX, APP_WINDOWSIZEY);

	// Set some optional flags:
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Create the window with a specific title:
	const char *cstr = title.c_str();
	windowId = glutCreateWindow(cstr);

	// Init Glew (*after* the context creation):
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) 
	{
		// Error loading GLEW
		throw std::runtime_error("Error loading GLEW");
	}
	if (!glewIsSupported("GL_VERSION_4_4")) { 

		// Required OpenGL version not supported 
		throw std::runtime_error("OpenGL 4.4 not supported");
	}

	logInfo();
	initShaders();

	// Global OpenGL settings:
	glEnable(GL_DEPTH_TEST);

	//doesn't print the back face of the triangles
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE); //normalize normals when scaling is performed
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f); //no "scanner" effect
	glShadeModel(GL_SMOOTH); //shades triangles
	timerCallback(0); //fps

	glutCloseFunc(closeCallback);

#ifdef _DEBUG
	// Register OpenGL debug callback
	glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
	// Enable debug notifications
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif // _DEBUG

	loadFboAndItsTexture();
	setUpBox();

	
	PFNWGLDXREGISTEROBJECTNVPROC interop = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	if (!interop)
	{
	std:cout << "Your Computer Does Not Support NV_DX_interop" << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "Loadded Extension wglDXRegisterObjectNV" << std::endl;
	}
}

Node LIB_API * Engine::load(string scene)
{
	OvoReader ovoReader = {};
	char * sceneChar = new char[scene.length() + 1];
	strcpy(sceneChar, scene.c_str());
	Node* res = ovoReader.readOVOfile(sceneChar);
	return res;
}
void LIB_API Engine::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return;
	//disable all lights, they ll be activated later if needed
	for (int i = 0; i < 8;i++)
		glDisable(GL_LIGHT0 + i);
}
void LIB_API Engine::swap()
{
	glutSwapBuffers();
}



//fills the list recoursively iterating the tree
void fillListRecursive(List* list, Node* node, glm::mat4 parent)
{
	glm::mat4 f = parent * node->getPosMatrix();
	list->addNode(node, f);

	for (Node* child : node->getChildren())
	{
		fillListRecursive(list, child, f);
	}
}

List LIB_API * Engine::createList(Node* node)
{
	List* list = new List();
	if (node->getParent() == nullptr)
		fillListRecursive(list, node, glm::mat4(1));
	//if not rendering a root node, pass the "parent" final matrix so the node will be rendered in the correct absolute position
	else
		fillListRecursive(list, node, node->getParent()->getFinal());
	return list;
}


//fills the rendering list, renders and return the list
List LIB_API * Engine::renderScene(Node* node)
{
	List* list =createList(node);
	renderScene(list);
	return list;
}

//renders the list
void LIB_API Engine::renderScene(List* list)
{

	pr->render();

	// Store the current viewport size:
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	// Render to each eye: 
	
	for (int c = 0; c < EYE_LAST; c++)
	{
		// Render into this FBO:
		fbo[c]->render();

		// Clear the FBO content:
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 3D rendering:
		list->renderWithCamera(active->getInverse());

		bool capture = false;
		if (capture)
		{
			unsigned char* imageData = (unsigned char *)malloc((int)( APP_WINDOWSIZEX * APP_WINDOWSIZEY * (3)));
			screenshot_ppm("out.ppm", APP_WINDOWSIZEX, APP_WINDOWSIZEY, (GLubyte **)&imageData);
		
		}
		
	}
	
	// Done with the FBO, go back to rendering into the window context buffers:
	Fbo::disable();
	glViewport(0, 0, prevViewport[2], prevViewport[3]);

	////////////////
	// 2D rendering:

	// Set a matrix for the left "eye":    
	glm::mat4 f = glm::mat4(1.0f);
	ortho= glm::ortho(0.0f, (float)APP_WINDOWSIZEX, 0.0f, (float)APP_WINDOWSIZEY, -1.0f, 1.0f);
	// Setup the passthrough shader:
	passthroughShader->render();
	passthroughShader->setMatrix(Location::PROJECTION_MATRIX, ortho);
	passthroughShader->setMatrix(Location::MODLVIEW_MATRIX, f);
	passthroughShader->setVertex(Location::COLOR, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

	glBindVertexArray(globalVao);
	glBindBuffer(GL_ARRAY_BUFFER, boxVertexVbo);
	glVertexAttribPointer((GLuint) 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glDisableVertexAttribArray(1); // We don't need normals for the 2D quad

	glBindBuffer(GL_ARRAY_BUFFER, boxTexCoordVbo);
	glVertexAttribPointer((GLuint) 2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);

	// Bind the FBO buffer as texture and render:
	glBindTexture(GL_TEXTURE_2D, fboTexId[EYE_LEFT]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Do the same for the right "eye": 
	f = glm::translate(glm::mat4(1.0f), glm::vec3(APP_WINDOWSIZEX / 2, 0.0f, 0.0f));
	passthroughShader->setMatrix(Location::MODLVIEW_MATRIX, f);
	passthroughShader->setVertex(Location::COLOR, glm::vec4(0.0f, .0f, 1.0f, 0.0f));
	glBindTexture(GL_TEXTURE_2D, fboTexId[EYE_RIGHT]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	frames++;
}
void LIB_API Engine::setActiveCamera(Camera* camera)
{
	active = camera;
}

Camera LIB_API * Engine::getActiveCamera()
{
	return active;
}

int LIB_API Engine::getMaxRenderLights()
{
	return maxRenderLights;
}

void LIB_API Engine::setMaxRenderLights(int n)
{
	if (n <= 8 && n > 0)
		this->maxRenderLights = n;
	else
		this->maxRenderLights = 8;
}


bool wireframe = false;
void LIB_API Engine::wireframeSwitch()
{
	wireframe = !wireframe;
	if (wireframe) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void LIB_API Engine::clearColor(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}

int LIB_API Engine::getWindowWidth()
{
	return glutGet(GLUT_WINDOW_WIDTH);
}

int LIB_API Engine::getWindowHeight()
{
	return glutGet(GLUT_WINDOW_HEIGHT);
}

void LIB_API Engine::setViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void LIB_API Engine::startEventLoop()
{
	glutMainLoop();
}

void LIB_API Engine::redisplay()
{
	glutPostWindowRedisplay(windowId);
}

void LIB_API Engine::showFps()
{
	fpsFlag = !fpsFlag;
}

//callbacks
void LIB_API Engine::reshape(void(*reshapeFunc)(int, int))
{
	glutReshapeFunc(reshapeFunc);
}

void LIB_API Engine::display(void(*displayFunc)())
{
	glutDisplayFunc(displayFunc);
}

void LIB_API Engine::keyboard(void(*keyboardFunc)(unsigned char, int, int))
{
	glutKeyboardFunc(keyboardFunc);
}

void LIB_API Engine::specialKeyboard(void(*specialFunc)(int, int, int))
{
	glutSpecialFunc(specialFunc);
}
void LIB_API Engine::mouseMoved(void(*mouseFunc)(int, int))
{
	glutPassiveMotionFunc(mouseFunc);
}
void LIB_API Engine::mouseWheel(void(*wheelFunc)(int, int, int, int))
{
	glutMouseWheelFunc(wheelFunc);
}

bool LIB_API Engine::initOpenXR()
{
	xr.init();

	XrQuaternionf quat;
	quat.x = quat.y = quat.z = 0.f;
	quat.w = 1.f;
	XrVector3f pos;
	pos.x = pos.y = pos.z = 0.f;
	XrPosef identityPose;
	identityPose.orientation = quat;
	identityPose.position = pos;

	xr.setReferenceSpace(identityPose, XR_REFERENCE_SPACE_TYPE_LOCAL);

	xr.beginSession();

	std::function<void(XrEventDataBuffer)> cb = [](XrEventDataBuffer runtimeEvent) {
		std::cout << "EVENT: session state changed ";
		XrEventDataSessionStateChanged* event =
			(XrEventDataSessionStateChanged*)&runtimeEvent;
		XrSessionState state = event->state;

		bool isVisible = event->state <= XR_SESSION_STATE_FOCUSED;

		std::cout << "to " << state << " Visible: " << isVisible;
		if (event->state >= XR_SESSION_STATE_STOPPING) {
			std::cout << std::endl << "Session is in state stopping...";
		}
		std::cout << std::endl;
	};

	xr.setCallback(XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED, cb);

	cout << "Runtime name: " << xr.getRuntimeName() << endl;
	cout << "Manufacturer name: " << xr.getManufacturerName() << endl;
	cout << "risoluzione: " << xr.getHmdIdealHorizRes() << "x" << xr.getHmdIdealVertRes() << endl;

	return true;
}

void LIB_API Engine::renderOpenXR(Node* node)
{
	List* list = createList(node);
	pr->render();

	xr.beginFrame();

	for (int i = 0; i < 2; i++)
	{
		OvXR::OvEye e = (OvXR::OvEye) i;
		glm::mat4 headMat = xr.getEyeModelviewMatrix(e);
		glm::mat4 proj = xr.getProjMatrix(e, 0.1f, 1000.f);

		xr.lockSwapchain(e);

		glViewport(0, 0, xr.getHmdIdealHorizRes(), xr.getHmdIdealVertRes());
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		list->renderXR(proj, headMat);

		xr.unlockSwapchain(e);
	}

	xr.endFrame();

	frames++;
}


