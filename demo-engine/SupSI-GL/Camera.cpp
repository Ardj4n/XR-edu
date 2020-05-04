#include "Engine.h"


LIB_API Camera::Camera() : Node()
{
}

LIB_API Camera::~Camera()
{
}

void LIB_API Camera::setFov(float fov)
{
	this->fov = fov;
}

float LIB_API Camera::getFov()
{
	return fov;
}

void LIB_API Camera::setNearPlane(float nearPlane)
{
	this->nearPlane = nearPlane;
}

float LIB_API Camera::getNearPlane()
{
	return nearPlane;
}

void LIB_API Camera::setFarPlane(float farPlane)
{
	this->farPlane = farPlane;
}

float LIB_API Camera::getFarPlane()
{
	return farPlane;
}

void LIB_API Camera::setAspect(float aspect)
{
	this->aspect = aspect;
}

float LIB_API Camera::getAspect()
{
	return aspect;
}

glm::mat4 LIB_API Camera::getProjMatrix()
{ 
	return glm::perspective(fov, aspect, nearPlane, farPlane);
}

//return the inverse matrix of the camera (using camera's final position)
glm::mat4 LIB_API Camera::getInverse()
{
	return glm::inverse(getFinal());
}

string LIB_API Camera::getType()
{
	return "camera";
}
