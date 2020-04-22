#include "Engine.h"


int Object::currId = 0;
int Object::getNextId()
{
	return currId++;
}

LIB_API Object::Object()
{
	this->id = getNextId();
}

LIB_API Object::~Object()
{
}

int LIB_API Object::getId()
{
	return id;
}
string LIB_API Object::getName()
{
	return name;
}
void LIB_API Object::setId(int id)
{
	this->id = id;
}
void LIB_API Object::setName(string name)
{
	this->name = name;
}