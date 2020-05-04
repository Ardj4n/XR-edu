#include "Engine.h"
#include "GL/freeglut.h"


LIB_API List::List() : Object()
{
}


LIB_API List::~List()
{
}

//find the correct position in the list for a given priority, highest priority first, if n lights have the same priority, oldest first
int LIB_API List::findLightPos(int priority)
{
	if (lightsCount == 0)
		return 0;
	for (int i = 0; i < lightsCount;i++) {
		Light* light = dynamic_cast<Light*>( list.at(i).node);
		if (priority > light->getPriority())
			return i;
	}
	return lightsCount;
}

void LIB_API List::addNode(Node* node, glm::mat4 finalMat)
{
	NodeMat x = {node, finalMat};
	if (node->getType().compare("light")==0) 
	{
		Light* light = dynamic_cast<Light*>(node);
		list.insert(list.begin()+ findLightPos(light->getPriority()), x);
		lightsCount+=1;
	}
	else {
		list.push_back(x);
	}

}

void LIB_API List::clear()
{
	list.clear();
}

void LIB_API List::render()
{
	renderWithCamera(glm::mat4(1));
}

void LIB_API List::renderWithCamera(glm::mat4 invCamera)
{
	Engine &e = Engine::getInstance();
	Program* prog = e.getProgram();

	int maxLights = e.getMaxRenderLights();
	int count = 0;
	for (NodeMat i : list) {
		//doesn't render low priority lights that exceed the maxRenderLights value defined by the engine
		if (!(count >= maxLights && count < lightsCount)) {
			//if the node is a light sets the lightNumber param (GL_LIGHT0, 1, ...)
			if (count < lightsCount) {
				Light* light = dynamic_cast<Light*>(i.node);
				light->setLightNumber(GL_LIGHT0 + count);
			}
			else
			{
				//Light::loadToShader();
			}

			prog->setMatrix(Location::PROJECTION_MATRIX, e.getActiveCamera()->getProjMatrix());
			prog->setMatrix(Location::MODLVIEW_MATRIX, invCamera * i.finalMat);
			prog->setMatrix(Location::NORMAL_MATRIX, glm::mat3{ glm::inverseTranspose(invCamera * i.finalMat) });

			i.node->render();
		}
		count++;
	}
}


void LIB_API List::renderXR(glm::mat4 proj, glm::mat4 head)
{
	Engine &e = Engine::getInstance();
	Program* prog = e.getProgram();

	int maxLights = e.getMaxRenderLights();
	int count = 0;
	for (NodeMat i : list) {
		//doesn't render low priority lights that exceed the maxRenderLights value defined by the engine
		if (!(count >= maxLights && count < lightsCount)) {
			//if the node is a light sets the lightNumber param (GL_LIGHT0, 1, ...)
			if (count < lightsCount) {
				Light* light = dynamic_cast<Light*>(i.node);
				light->setLightNumber(GL_LIGHT0 + count);
			}
			else
			{
				//Light::loadToShader();
			}

			prog->setMatrix(Location::PROJECTION_MATRIX, proj);
			prog->setMatrix(Location::MODLVIEW_MATRIX, head * i.finalMat);
			prog->setMatrix(Location::NORMAL_MATRIX, glm::mat3{ glm::inverseTranspose(head * i.finalMat) });

			i.node->render();
		}
		count++;
	}
}

vector<Node*> LIB_API List::getNodes()
{
	vector<Node*> nodes;
	for (NodeMat n : list)
	{
		nodes.push_back(n.node);
	}
	return nodes;
}

int LIB_API List::getLightsCount()
{
	return lightsCount;
}

string LIB_API List::getType()
{
	return "list";
}