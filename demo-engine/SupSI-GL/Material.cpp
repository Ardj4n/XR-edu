#include "Engine.h"
#include "GL/freeglut.h"

LIB_API Material::Material() : Object()
{
	emission = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	ambient = glm::vec4(0.8f, 0.8f, 0.8f, 0.8f);
	diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	shininess = powf(2.0f, 7.0f);
	texture = nullptr;
}

LIB_API Material::~Material()
{
}

glm::vec4 LIB_API Material::getEmission()
{
	return emission;
}
void LIB_API Material::setEmission(glm::vec4 emission)
{
	this->emission = emission;
}
glm::vec4 LIB_API Material::getAmbient()
{
	return ambient;
}
void LIB_API Material::setAmbient(glm::vec4 ambient)
{
	this->ambient = ambient;
}
glm::vec4 LIB_API Material::getDiffuse()
{
	return diffuse;
}
void LIB_API Material::setDiffuse(glm::vec4 diffuse)
{
	this->diffuse = diffuse;
}
glm::vec4 LIB_API Material::getSpecular()
{
	return specular;
}
void LIB_API Material::setSpecular(glm::vec4 specular)
{
	this->specular = specular;
}
int LIB_API Material::getShininess()
{
	return shininess;
}
void LIB_API Material::setShininess(int shininess)
{
	this->shininess = shininess;
}
Texture LIB_API * Material::getTexture()
{
	return texture;
}
void LIB_API Material::setTexture(Texture *texture)
{
	this->texture = texture;
}

void LIB_API Material::render()
{
	Program* p = Engine::getInstance().getProgram();

	p->setFloat(Location::MATERIAL_SHININESS, shininess);
	p->setVertex(Location::MATERIAL_DIFFUSE, diffuse);
	p->setVertex(Location::MATERIAL_AMBIENT, ambient);
	p->setVertex(Location::MATERIAL_EMISSIVE, emission);
	p->setVertex(Location::MATERIAL_SPECULAR, specular);

	if (texture == nullptr)
		setTexture(&Texture::getBlankTexture());

	texture->render();
}

string LIB_API Material::getType()
{
	return "material";
}