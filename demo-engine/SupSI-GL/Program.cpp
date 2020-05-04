#include "Engine.h"

#include <GL/glew.h>
#include <GL/freeglut.h>


LIB_API Program::Program(Shader * ver_Shader, Shader * frag_Shader)
	: m_vertex{ver_Shader}
	, m_fragment{frag_Shader}
{
}

bool LIB_API Program::build()
{
	if (m_vertex && m_vertex->shaderType() != Shader::TYPE_VERTEX)
	{
		std::cout << "[ERROR] Invalid vertex shader passed" << std::endl;
		return false;
	}
	if (m_fragment && m_fragment->shaderType() != Shader::TYPE_FRAGMENT)
	{
		std::cout << " [ERROR] Invalid fragment shader passed" << std::endl;
		return false;
	}

	// Delete if already used:
	if (m_glId)
	{
		// On reload, make sure it was a program before:
		/*if (this->type != TYPE_PROGRAM)
		{
			std::cout << "[ERROR] Cannot reload a shader as a program" << std::endl;
			return false;
		}*/
		glDeleteProgram(m_glId);
	}

	// Create program:
	m_glId = glCreateProgram();
	if (m_glId == 0)
	{
		std::cout << "[ERROR] Unable to create program" << std::endl;
		return false;
	}

	// Bind vertex shader:
	if (m_vertex)
		glAttachShader(m_glId, m_vertex->getGlId());

	// Bind fragment shader:
	if (m_fragment)
		glAttachShader(m_glId, m_fragment->getGlId());

	// Link program:
	glLinkProgram(m_glId);
	//this->m_type = Shader::TYPE_PROGRAM;

	// Verify program:
	int status;
	char buffer[Shader::MAX_LOGSIZE];
	int length = 0;
	memset(buffer, 0, Shader::MAX_LOGSIZE);

	glGetProgramiv(m_glId, GL_LINK_STATUS, &status);
	glGetProgramInfoLog(m_glId, Shader::MAX_LOGSIZE, &length, buffer);
	if (status == false)
	{
		std::cout << "[ERROR] Program link error: " << buffer << std::endl;
		return false;
	}
	glValidateProgram(m_glId);
	glGetProgramiv(m_glId, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cout << "[ERROR] Unable to validate program" << std::endl;
		return false;
	}

	// Done:
	return true;
}

void LIB_API Program::render()
{
	if (m_glId)
		glUseProgram(m_glId);
	else
	{
		std::cout << "[ERROR] Invalid shader rendered" << std::endl;
	}
}


bool LIB_API Program::bindLocation(Location location, const std::string& var_name)
{
	if (var_name.empty())
	{
		std::cout << "[ERROR] Invalid params" << std::endl;
		return 0;
	}

	// Return location:
	int r = glGetUniformLocation(m_glId, var_name.c_str());
	if (r == -1)
	{
		std::cout << "[ERROR] Param '" << var_name << "' not found" << std::endl;
		return false;
	}

	m_map[location] = r;
	m_strings[location] = var_name;

	return true;
}

bool LIB_API Program::bindLayoutLocation(unsigned int layaout_number, const std::string& name)
{
	glBindAttribLocation(m_glId, layaout_number, name.c_str());
	return false;
}

int LIB_API Program::getLocation(Location location)
{
	return m_map[location];
}

std::string LIB_API Program::getStringLocation(Location location)
{
	return m_strings[location];
}

int LIB_API Program::getParamLocation(const char * name)
{
	if (name == nullptr)
	{
		std::cout << "[ERROR] Invalid params" << std::endl;
		return 0;
	}

	// Return location:
	int r = glGetUniformLocation(m_glId, name);
	if (r == -1)
		std::cout << "[ERROR] Param '" << name << "' not found" << std::endl;
	return r;
}

void LIB_API Program::setMatrix(Location location, const glm::mat4& mat)
{
	glUniformMatrix4fv(m_map[location], 1, GL_FALSE, glm::value_ptr(mat));
}

void LIB_API Program::setMatrix(Location location, const glm::mat3& mat)
{
	glUniformMatrix3fv(m_map[location], 1, GL_FALSE, glm::value_ptr(mat));
}

void LIB_API Program::setVertex(Location location, const glm::vec4 &vect)
{
	glUniform4fv(m_map[location], 1, glm::value_ptr(vect));
}

void LIB_API Program::setVertex(Location location, const glm::vec3 &vect)
{
	glUniform3fv(m_map[location], 1, glm::value_ptr(vect));
}

void LIB_API Program::setVertex(Location location, glm::vec4 * vec, int size)
{
	glUniform4fv(m_map[location], size, (GLfloat *) vec);
}

void LIB_API Program::setVertex(Location location, glm::vec3 * vec, int size)
{
	glUniform3fv(m_map[location], size, (GLfloat *) vec);
}

void LIB_API Program::setFloat(Location location, float value)
{
	glUniform1f(m_map[location], value);
}

void LIB_API Program::setInt(Location location, int value)
{
	glUniform1i(m_map[location], value);
}

std::string LIB_API Program::getType()
{
	return "Program";
}

unsigned int LIB_API Program::getGlId()
{
	return m_glId;
}
