#pragma once

/**
* Supsi-GE, Camera management class
* This class purpose is to create and control objects used to
* visualize the passed scene
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Camera :
	public virtual Node
{
private:
	/**
	@var fov
	Indicates the camera's Field Of View. Defaults to PI/2 radians (90 degrees )
	*/
	float fov = glm::radians(90.0f);

	/**
	@var nearPlane
	Is the "Near Plane" limit of the camera,
	that is how close an object can be before it's no longer rendered. Defaults to 1
	*/
	float nearPlane=1.0f;

	/**
	@var farPlane
	Is the "Far Plane" limit of the view.
	Like the near plane, but for how far the object is. Defaults to 100
	*/
	float farPlane=100.0f;

	/**
	@var aspect
	Is the camera aspect ratio (width/height). Defaults to 1.
	It is immediately set to the window aspect ratio at runtime and refreshed at every redraw callback.
	*/
	float aspect=1.0f;

public:
	/**
	Constructor for the camera
	@see Object.h
	*/
	Camera();

	/**
	Destructor
	@see Object.h
	*/
	~Camera();

	/**
	Sets the FOV
	@param fov the new Field of Vision
	*/
	void setFov(float fov);

	/**
	Returns the current FOV
	*/
	float getFov();

	/**
	Sets the near plane distance (likely to render closer objects)
	@param nearPlane the new near plane value
	*/
	void setNearPlane(float nearPlane);

	/**
	Returns the current nearPlane distance
	*/
	float getNearPlane();

	/**
	Sets the far plane distance (likely to render far awy objects)
	@param farPlane The new far plane value
	*/
	void setFarPlane(float farPlane);

	/**
	Returns the current farPlane distance
	*/
	float getFarPlane();

	/**
	Sets a new aspect ratio (e.g 4:3, 16:9, 4K, ...)
	@param aspect The new aspect ratio value
	*/
	void setAspect(float aspect);

	/**
	Returns the current aspect ratio
	*/
	float getAspect();

	/**
	Calculates and returns the camera's projection matrix.
	*/
	glm::mat4 getProjMatrix();

	/**
	Calculates and returns the camera's inverted positioning matrix,
	used when rendering the scene from the selected camera by moving the world so that the camera
	becomes the center.
	*/
	glm::mat4 getInverse();

	/**
	Returns "camera"
	@see Object.h
	*/
	string getType();
};

