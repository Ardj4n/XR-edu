/**
 * @file		shader.h
 * @brief	Minimal OpenGL shader class
 *
 * @author	Achille Peternier (C) SUPSI [achille.peternier@supsi.ch]
 */
#pragma once

#include "Engine.h"

#include <iostream>



//////////////////
// CLASS Shader //
//////////////////

/**
 * @brief OGL Shader. Management and rendering of shaders.
 */
class LIB_API Shader : public Object
{
	//////////
public: //
//////////

	// Constants:
	static const unsigned int MAX_LOGSIZE = 4096;  ///< Max output size in char for a shader log

	// Enums:
	enum Type///< Kind of shader
	{
		TYPE_UNDEFINED = 0,
		TYPE_VERTEX,
		TYPE_FRAGMENT,
		TYPE_PROGRAM,
		TYPE_LAST
	};

	// Const/dest:	 
	Shader();
	~Shader();

	// Get/set:
	int getParamLocation(const char *name);
	int shaderType();
	void setMatrix(int param, const glm::mat4 &mat);
	void setFloat(int param, float value);
	void setInt(int param, int value);
	void setVec3(int param, const glm::vec3 &vect);
	void setVec4(int param, const glm::vec4 &vect);
	// Accessing data:
	bool loadFromMemory(int kind, const char *data);
	bool loadFromFile(int kind, const char *filename);
	bool build(Shader *vertexShader, Shader *fragmentShader);

	void bind(int location, const char *attribName);

	unsigned int getGlId();

	// Rendering:				
	void render();

	///////////	 
private:	//
///////////					

	// Object kind:
	int type;

	// OGL id:
	unsigned int glId;

	string getType();
};