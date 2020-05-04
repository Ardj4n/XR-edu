#include "Engine.h"



LIB_API Vertex::Vertex()
{
	this->setTexCord(new glm::vec2(0.0f,0.0f));
}

LIB_API Vertex::Vertex(glm::vec3 *norm, glm::vec3 *value, glm::vec2 *texCord)
{
	this->norm = norm;
	this->value = value;
	this->texCord = texCord;
}


LIB_API Vertex::~Vertex()
{
}

void LIB_API Vertex::setNorm(glm::vec3 *norm)
{
	this->norm = norm;
}

void LIB_API Vertex::setValue(glm::vec3 *value)
{
	this->value = value;
}

void LIB_API Vertex::setTexCord(glm::vec2 *texCord)
{
	this->texCord = texCord;
}

glm::vec3 LIB_API * Vertex::getNorm()
{
	return norm;
}

glm::vec3 LIB_API * Vertex::getValue()
{
	return value;
}

glm::vec2 LIB_API * Vertex::getTexCord()
{
	return texCord;
}
