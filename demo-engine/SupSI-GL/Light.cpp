#include "Engine.h"
#include "GL/freeglut.h"

/*
std::vector<glm::vec3> Light::vec_positions{};
std::vector<glm::vec4> Light::vec_specular{};
std::vector<glm::vec4> Light::vec_ambient{};
std::vector<glm::vec4> Light::vec_diffuse{};
*/

LIB_API  Light::Light() : Node()
{
}

LIB_API Light::~Light()
{
}

void LIB_API Light::toggle()
{
	if (isOn == 0)
		isOn = 1;
	else
		isOn = 0;
}

glm::vec4 LIB_API Light::getColor()
{
	return color;
}
void LIB_API Light::setColor(glm::vec4 color)
{
	this->color = color;
}
float LIB_API Light::getW()
{
	return w;
}
void LIB_API Light::setW(float w)
{
	this->w = w;
}
glm::vec3 LIB_API Light::getDirection()
{
	return direction;
}
void LIB_API Light::setDirection(glm::vec3 direction)
{
	this->direction = direction;
}
float LIB_API Light::getCutoff()
{
	return cutoff;
}
void LIB_API Light::setCutoff(float cutoff)
{
	this->cutoff = cutoff;
}
float LIB_API Light::getAttenuation()
{
	return attenuation;
}
void LIB_API Light::setAttenuation(float attenuation)
{
	this->attenuation = attenuation;
}

int LIB_API Light::getPriority()
{
	return priority;
}


void LIB_API Light::setPriority(int priority)
{
	this->priority = priority;
}

string LIB_API Light::getType()
{
	return "light";
}

void LIB_API Light::setLightNumber(int n) 
{
	this->lightNumber = n;
}

void LIB_API Light::render()
{
	static std::vector<glm::vec3> vec_positions{};
	static std::vector<glm::vec4> vec_specular{};
	static std::vector<glm::vec4> vec_ambient{};
	static std::vector<glm::vec4> vec_diffuse{};

	//return;
	if (isOn)
	{
		lightNumber = lightNumber - GL_LIGHT0;

		if (lightNumber == 0)
		{
			vec_positions.clear();
			vec_ambient.clear();
			vec_diffuse.clear();
			vec_specular.clear();
		}

		Program *p = Engine::getInstance().getProgram();

		vec_positions.push_back(glm::vec3(0.f)); //0 becasue transformation have already been applied
		vec_ambient.push_back(color * 0.3f);
		vec_specular.push_back(color * 0.6f);
		vec_diffuse.push_back(color * 0.9f);


		p->setVertex(Location::LIGHT_ARRAY_POSITION, vec_positions.data(), vec_positions.size());
		p->setVertex(Location::LIGHT_ARRAY_AMBIENT, vec_ambient.data(), vec_ambient.size());
		p->setVertex(Location::LIGHT_ARRAY_DIFFUSE, vec_diffuse.data(), vec_diffuse.size());
		p->setVertex(Location::LIGHT_ARRAY_SPECULAR, vec_specular.data(), vec_specular.size());
		p->setInt(Location::LIGHT_TOTAL, vec_positions.size());
	}

}

/*
void Light::loadToShader()
{
	Program *p = Engine::getInstance().getProgram();

	p->setVertex(Location::LIGHT_ARRAY_POSITION, vec_positions.data(), vec_positions.size());
	p->setVertex(Location::LIGHT_ARRAY_AMBIENT, vec_ambient.data(), vec_ambient.size());
	p->setVertex(Location::LIGHT_ARRAY_DIFFUSE, vec_diffuse.data(), vec_diffuse.size());
	p->setVertex(Location::LIGHT_ARRAY_SPECULAR, vec_specular.data(), vec_specular.size());
	p->setInt(Location::LIGHT_TOTAL, vec_positions.size());
}
*/