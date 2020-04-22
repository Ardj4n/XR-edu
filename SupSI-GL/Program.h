#pragma once

#include <map>

enum Location : int
{
	MATERIAL_EMISSIVE = 0,
	MATERIAL_SPECULAR,
	MATERIAL_AMBIENT,
	MATERIAL_DIFFUSE,
	MATERIAL_SHININESS,

	LIGHT_POSITION,
	LIGHT_AMBIENT,
	LIGHT_DIFFUSE,
	LIGHT_SPECULAR,

	LIGHT_TOTAL,
	LIGHT_ARRAY_POSITION,
	LIGHT_ARRAY_AMBIENT,
	LIGHT_ARRAY_DIFFUSE,
	LIGHT_ARRAY_SPECULAR,

	PROJECTION_MATRIX,
	MODLVIEW_MATRIX,
	NORMAL_MATRIX,

	COLOR,
};


class LIB_API Program 
	: public virtual Object
{

public:
	Program(Shader* ver_Shader, Shader* frag_Shader);

	bool build();
	void render();

	int getLocation(Location location);
	std::string getStringLocation(Location location);
	int getParamLocation(const char *name);

	void setMatrix(Location location, const glm::mat4& mat);
	void setMatrix(Location location, const glm::mat3& mat);

	void setVertex(Location location, const glm::vec4& vec);
	void setVertex(Location location, glm::vec4* vec, int size = 1);
	void setVertex(Location location, const glm::vec3& vec);
	void setVertex(Location location, glm::vec3* vec, int size = 1);
	
	void setFloat(Location location, float value);
	void setInt(Location location, int value);
	
	bool bindLocation(Location location, const std::string& var_name);
	bool bindLayoutLocation(unsigned int layaout_number, const std::string& name);

	std::string getType();
	unsigned int getGlId();


private:
	Shader* m_vertex;
	Shader* m_fragment;
	std::map<Location, int> m_map;
	std::map<Location, std::string> m_strings;
	unsigned int m_glId;
};