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
#include "Engine.h"

/// Global variables
Engine* engine = &Engine::getInstance();
Node* n;
int sizeX;
int sizeY;


/**
 * Display callback  this is the main rendering routine
 */
void displayCallback()
{
    engine->clear();

	//engine->renderScene(n);
	engine->renderOpenXR(n);

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
}


int main(int argc, char *argv[])
{
	engine->init(argc, argv, "Transformer");

	n = engine->load("../resources/scena0.OVO");
	//la scena è immensa
	glm::mat4 scale = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 0.005f });
	glm::mat4 pos = glm::translate(scale, glm::vec3{ 0.f, -150.f, -400.f });
	n->setPosMatrix(pos);
	/*/
	Camera* cam = new Camera();
	cam->setAspect(16. / 9.);
	cam->setFarPlane(1000);
	cam->setNearPlane(1);
	cam->setFov(45.f);

	cam->setPosMatrix(glm::translate(glm::mat4{ 1.f }, glm::vec3{0.f, 100.f, 200.f}));
	engine->setActiveCamera(cam);
	n->appendChild(cam);
	*/
	engine->initOpenXR();

    //callbacks
    engine->display(displayCallback);
    engine->reshape(reshapeCallback);

    //render
    engine->startEventLoop();
}
