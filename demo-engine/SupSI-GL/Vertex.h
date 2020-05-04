#pragma once

/**
* Supsi-GE, Vertex management class
* This class manages the single points that comprise a mesh's polygons.
* Other than the 3D coordinates, it has a normal vector for correct light calculations (and resulting color).
* It also maps a Texture to the points so that the details appear correctly
* on the represented object during render time, when OpenGL interpolates their color
* to "fill up" the polygon.
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/
class LIB_API Vertex
{
private:
	/**
	@var value 
	The vertex' coordinates in 3D space
	*/
	glm::vec3* value;
	
	/**
	@var norm 
	The vertex normal vector.
	Normal vectors are algebric elements that are perfectly perpendicular
	to their associated element, like a tower on a plain field.
	*/
	glm::vec3* norm;
	
	/**
	@var texCord 
	Coordinate belonging to a 2D picture (texture).
	Used to associate where the picture should appear on the mesh by mapping it to
	the mesh's polygon's verteces.
	If done incorrectly, the texture may appear distorted, misplaced or completely missing.
	@see Texture.h
	*/
	glm::vec2* texCord;

public:
	/**
	Constructor with defaults values
	@see Object.h
	*/
	Vertex();

	/**
	Constructor with all values
	@param norm Pointer to the new normal vector
	@param value Pointer to the new coordinates
	@param texCord Pointer to the coordinates
	@see Object.h
	*/
	Vertex(glm::vec3 *norm, glm::vec3 *value, glm::vec2 *texCord);
	
	/**
	Destructor
	@see Object.h
	*/
	~Vertex();

	/**
	Sets the new normal vector of the vertex, 
	used when applying linear applications before rerender the scene
	@param norm Pointer to the new normal vector
	*/
	void setNorm(glm::vec3 *norm);

	/**
	Sets the new coordinates of the vertex
	@param value Pointer to the new coordinates
	*/
	void setValue(glm::vec3 *value);
	
	/**
	Maps a point in the texture image file to the vertex.
	@param texCord Pointer to the coordinates
	*/
	void setTexCord(glm::vec2 *texCord);

	/**
	Returns the normal vector of the vertex
	*/
	glm::vec3* getNorm();
	
	/**
	Returns the coordinates of the vertex
	*/
	glm::vec3* getValue();

	/**
	Returns the mapped texture coordinate of the vertex
	*/
	glm::vec2* getTexCord();
};

