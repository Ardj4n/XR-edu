/**
* TransformApp, Application's main "class"
* This class passes a scene made up of a room, a transformer, a camera
* and two lights to the Supsi-GE graphics engine to be rendered.
* Then it bind four out of six callbacks to provide interactive control
* (the mouse is not used in this application)
* for the following actions:
* - Change state to the charachter (Umanoid <--> Vehicle)
* - Toggle On/Off the lights
* - Modify the active camera
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

/// Includes
#include "../SupSI-GL/Engine.h"

//needed to play the transformation sound
#ifdef _WIN32
#include <windows.h>
#endif
#include <chrono>

/// Preprocessor Definitions
#define GLUT_KEY_LEFT 0x0064
#define GLUT_KEY_UP 0x0065
#define GLUT_KEY_RIGHT 0x0066
#define GLUT_KEY_DOWN 0x0067

/// Global variables
Engine* engine = &Engine::getInstance();
Node* n;
Node* n1;
Node* n2;
bool animate = false;
Camera* c;
Camera* c1;
Light* dynamicLight;
int cameraSwitch = 0;
Node* startAnim;
Node* currentAnim;
Node* endAnim;
glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f,0.1f,0.0f));
//glm::mat4 translationCamera = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 120.0f, 200.0f));
glm::mat4 translationCamera = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, 20.0f));

glm::mat4 rotationCamera = glm::mat4(1);
int sizeX;
int sizeY;


float animationTime = 6000.0f;
unsigned long long  animationTimeS;
vector<Node*> l;
vector<Node*> l1;
vector<Node*> l2;

/**
When called, this method converts the starting and final scenes's matrices in quaternions
(numbers with one real and three immaginary parts) and interpolates them to generate an
animation.
The timing of such action is calculated according to the duration of a particular sound effect.
*/
void animation()
{
    unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    float intr = (float)(now - animationTimeS) / animationTime;
    if (intr > 1.0f)
    {
		intr = 1.0f;
        animate = false;
    }
    for (int i = 0; i < l.size(); i++)
    {
        glm::quat qM1 = glm::quat(glm::inverse(l1.at(i)->getPosMatrix()) * l1.at(i)->getPosMatrix()); //identy
        glm::quat qM2 = glm::quat(glm::inverse(l1.at(i)->getPosMatrix()) * l2.at(i)->getPosMatrix()); //delta
        glm::quat quat_R = glm::slerp(qM1, qM2, intr);
        glm::mat4 m = l1.at(i)->getPosMatrix() * glm::mat4(quat_R);
        l.at(i)->setPosMatrix(m);
    }
	if (!animate)
	{
		//swap the 2 lists
		vector<Node*> dum = l1;
		l1 = l2;
		l2 = dum;
	}
}

/**
 * Display callback  this is the main rendering routine
 */
void displayCallback()
{
    engine->clear();

    //engine->renderScene(n);
    engine->renderOpenXR(n, c->getPosMatrix());

    engine->swap();
    engine->redisplay();
}

/**
 * Reshape callback this callback is invoked each time the window gets resized (and once also when created).
 * @param width window width
 * @param height window height
 */
void reshapeCallback(int width, int height)
{
    engine->setViewport(0, 0, width, height);
    sizeX = width;
    sizeY = height;
    c->setAspect((float)width / (float)height);
    c1->setAspect((float)width / (float)height);
}


//update the main camera (not the active one) with the current rotations and translations
void updateCamera()
{
    c->setPosMatrix(translationCamera*rotationCamera);
}

/**
 * Keyboard callback  this callback is invoked each time a standard keyboard key is pressed.
 * @param  key the button that was pressed
 * @param mouseX mouse X coordinate
 * @param mouseY mouse Y coordinate
 */
void keyboardCallback(unsigned char key, int mouseX, int mouseY)
{
    float tValue = 10.f;

    switch (key)
    {
    //camera switch
    case 'c':
        if (cameraSwitch)
        {
            engine->setActiveCamera(c);
            cameraSwitch = 0;
        }
        else
        {
            engine->setActiveCamera(c1);
            cameraSwitch = 1;
        }
        break;
    //camera up
    case 'q':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(0.0f, tValue, 0.0f, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    //camera down
    case 'e':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(0.0f, -tValue, 0.0f, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    //camera forward
    case 'w':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(0.0f, 0.0f, -tValue, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    // camera backward
    case 's':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(0.0f, 0.0f, tValue, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    // camera left
    case 'a':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(-tValue, 0.0f, 0.0f, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    // camera right
    case 'd':
        translationCamera *= glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec4(tValue, 0.0f, 0.0f, 1.0f) * glm::inverse(rotationCamera)));
        updateCamera();
        break;
    //wireframe
    case 'y':
        engine->wireframeSwitch();
        break;
	case 'l':
		dynamicLight->toggle();
		break;
    //trnsform
    case ' ':
		if (!animate)
		{
			#ifdef _WIN32
				PlaySound("../resources/transform.wav", NULL, SND_ASYNC);
			#endif
			animationTimeS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			animate = true;
			animation();
		}
        break;

    }
}

/**
 * Special callback is invoked each time a special keyboard key is pressed. This callback is used to move
 * the camera (if movable). FreeGlut special key redefinition is necessary (e.g #define GLUT_KEY_LEFT 0x0064)
 * @param  key an integer representing a special key
 * @param mouseX mouse X coordinate
 * @param mouseY mouse Y coordinate
 */
void specialCallback(int key, int mouseX, int mouseY)
{
    float rValue = 5.f;

    switch (key)
    {
    //camera rotate up
    case GLUT_KEY_UP:
        rotationCamera *= glm::rotate(glm::mat4(1), glm::radians(rValue), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    //camera rotate down
    case GLUT_KEY_DOWN:
        rotationCamera *= glm::rotate(glm::mat4(1), glm::radians(-rValue), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    //camera rotate left
    case GLUT_KEY_LEFT:
        //rotationCamera *= glm::rotate(glm::mat4(1), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationCamera = glm::rotate(glm::mat4(1), glm::radians(rValue), glm::vec3(0.0f, 1.0f, 0.0f)) * rotationCamera;
        break;
    //camera rotate right
    case GLUT_KEY_RIGHT:
        rotationCamera = glm::rotate(glm::mat4(1), glm::radians(-rValue), glm::vec3(0.0f, 1.0f, 0.0f)) * rotationCamera;
        break;
    }
    updateCamera();
}


/**
 * Mouse wheel callback that implement a zoom function
 * @param wheel the value of the mouse wheel
 * @param direction the direction in which the wheel is scrolled
 * @param x x coordinate
 * @param y y coordinate
 */
void mouseWheel(int wheel, int direction, int x, int y)
{
    wheel = 0;
    if (direction == -1)
    {

    }
    else if (direction == +1)
    {

    }
}

/**
 * Mouse motion callback
 * @param x x coordinate
 * @param y y coordinate
 */
void mouseMoved(int x, int y)
{
}


int main(int argc, char *argv[])
{
	engine->init(argc, argv, "Transformer");
    engine->initOpenXR();
/*
    XrQuaternionf quat;
    quat.x = quat.y = quat.z = 0.f;
    quat.w = 1.f;
    XrVector3f pos;
    pos.x = pos.y = pos.z = 0.f;
    XrPosef identityPose;
    identityPose.orientation = quat;
    identityPose.position = pos;
*/
	
    //n1 = engine->load("../resources/scena0.OVO");
    //n2 = engine->load("../resources/scena4.OVO");
    n = engine->load("../resources/scena0.OVO");

    l = engine->createList(n)->getNodes();
    //l1 = engine->createList(n1)->getNodes();
    //l2 = engine->createList(n2)->getNodes();


    //Camera
    c = new Camera();
    c->setAspect(1.0);
    c->setFov(glm::radians(45.0f));
    c->setNearPlane(1.0);
    c->setFarPlane(1000.0);
    c->setPosMatrix(translationCamera);
    c1 = new Camera();
    c1->setAspect(1.0);
    c1->setFov(glm::radians(120.0f));
    c1->setNearPlane(1.0);
    c1->setFarPlane(1000.0f);
    c1->setPosMatrix(translationCamera);

	//light
	dynamicLight = new Light();
	dynamicLight->setPosMatrix(glm::mat4(1));
	dynamicLight->setParent(c);
	c->setParent(n);

    //callbacks
    engine->display(displayCallback);
    engine->reshape(reshapeCallback);
    engine->keyboard(keyboardCallback);
    engine->specialKeyboard(specialCallback);
    engine->mouseWheel(mouseWheel);
    engine->mouseMoved(mouseMoved);

    //render
    engine->setActiveCamera(c);
    engine->startEventLoop();

    delete engine;
    delete c;
    delete n;
}
