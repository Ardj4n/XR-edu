#pragma once
/**
* Supsi-GE, Lights management class
* This class' purpose is to create and control light sources
* in the scene
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/
class LIB_API Light :
	public virtual Node
{
private:
	/**
	@var color
	The light's color expressed as a RGBA vector
	*/
	glm::vec4 color=glm::vec4(1.0f,1.0f,1.0f,1.0f);

	/**
	@var isOn
	"Boolean" value indicating the light's current status. Either ON (1) or OFF (0), defaults to 1.
	*/
	int isOn = 1;

	/**
	@var priority
	Indicates the current light's priority, meaning if it's in use and to be rendered by the graphics engine.
	Used to overcome the maximum allowed by OpenGL (8 in most cases). Low numbers mean lower priority, Higher numbers mean higher priority.
	Defaults to 0.
	*/
	int priority = 0;

	/**
	@var lightNumber
	The light's identification number for OpenGL.
	Used to pass the variuous GL_LIGHTx (e.g GL_LIGHT0, GL_LIGHT1, ...) and the accompainig methods glLight()
	*/
	int lightNumber;

	/**
	@var attenuation
	Indicates how much the light is attenuated.
	Follows a linear algorithm. Defaults to 0.
	*/
	float attenuation=0.0f; //linear attenuation

	/**
	@var w
	"Boolean" value indicating wheter the light is a solar type (also called directional/infinite)(0) or lamp type (has a specific direction)(1).
	Corrsponds to the 4th coordinates in the pipeline, but is not used for any other but this purpose.
	*/
	float w=1.0f; //0 if directional/infinite, 1 if omnidirectional/point/spot

	//omni/point light settings
	/**
	@var direction
	Indicates where the light is pointing.
	Considered only if w=1
	*/
	glm::vec3 direction;

	/**
	@var cutoff
	Is the light's aperture angle. Ranges from 0 (single point) to 90 (180 degrees aperture).
	A special 180 value indicates a omnidirectional source (360 degrees)
	*/
	float cutoff=180.0f; //must be between 0 e 90, 180 if omni/point

	/*
		static std::vector<glm::vec3> vec_positions;
		static std::vector<glm::vec4> vec_specular;
		static std::vector<glm::vec4> vec_ambient;
		static std::vector<glm::vec4> vec_diffuse;
	*/

public:
	/**
	Constructor
	@see Object.h
	*/
	Light();

	/**
	Destructor
	@see Object.h
	*/
	~Light();

	/**
	Toggles the lights (sets isOn to 0/1)
	*/
	void toggle();

	/**
	Returns the light's color
	*/
	glm::vec4 getColor();

	/**
	Changes the light's color to "color"
	@param color the new RGBA color vector
	*/
	void setColor(glm::vec4 color);

	/**
	Returns w, the 4th dimensional coordinate / light mode selector
	*/
	float getW();

	/**
	Sets w
	@param w the new "mode selector"
	*/
	void setW(float w);

	/**
	Returns the directional vector
	*/
	glm::vec3 getDirection();

	/**
	Sets the directional vector to "direction"
	@param direction the new light direction
	*/
	void setDirection(glm::vec3 direction);

	/**
	Returns the light's aperture
	*/
	float getCutoff();

	/**
	Sets the light's aperture to "cutoff"
	@param cutoff The new aperture value
	*/
	void setCutoff(float cutoff);

	/**
	Returns the attenuation value
	*/
	float getAttenuation();

	/**
	Sets the attenuation value
	@param attenuation The new attenuation value
	*/
	void setAttenuation(float attenuation);

	/**
	Returns the priority value
	*/
	int getPriority();

	/**
	Assigns a new rendering priority number to the light
	@param priority The new light priority
	*/
	void setPriority(int priority);

	/**
	Assigns a new idetification number to the light
	@param lightNumber The new identifier number for the light
	*/
	void setLightNumber(int lightNumber);

	/**
	Returns "light"
	@see Object.h
	*/
	string getType();

	/**
	Renders the light source
	This method changes "rendering mode" depending on w's and cutoff's values. Done to avoid additional subclasses.
	The positioning vector used has all values set to 0, the only exception being w.
	It's done this way as all linear applications used to position the light source were applied before the call to this method.

	For the base definition
	@see Object.h
	*/
	void render();

	//static void loadToShader();
};

