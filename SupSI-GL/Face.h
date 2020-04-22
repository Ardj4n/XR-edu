#pragma once
/**
* Supsi-GE, Polygon management class
* This class represent the minimal unit of a mesh:
* a triangular polygon
* Created to control more complex object than the basic
* ones provided by FreeGLUT
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Face
	
{
private:

	/**
	@var vertices
	Pointer list to the face's vertices
	*/
	vector<Vertex*> vertices;
public:
	/**
	Constructor
	@see Object.h
	*/
	Face();

	/**
	Destructor
	@see Object.h
	*/
	~Face();

	/**
	Returns the vertices' list
	*/
	vector<Vertex*> getVertices();

	/**
	Adds a new pointer to the list (adds a new vertex)
	@param v Pointer to the new vertex
	*/
	void addVertex(Vertex* v);
};

