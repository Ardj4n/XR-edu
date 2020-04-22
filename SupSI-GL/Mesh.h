#pragma once
/**
* Supsi-GE, Mesh management class
* This class is used to controls the polygon collections to be displayed in the scene.
* A mesh is a representation of an object (real or immaginary) to be rendered by the
* graphics engine.
* It may be made out of a material (Material.h), itself having additional 2D details (Texture.h)
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Mesh : public virtual Node
{
private:
	/**
	@var material
	Pointer to the mesh' material
	*/
	Material* material;

	unsigned int m_vaoID;
	unsigned int m_vboID[2];

	unsigned int m_numVertices;
	unsigned int m_numFaces;
	
public:
	/**
	Constructor
	@see Object.h
	*/
	Mesh();

	/**
	Destructor
	@see Object.h
	*/
	~Mesh();

	/**
	Returns the pointer to the current material
	*/
	Material* getMaterial();

	/**
	Sets the mesh' material to "material"
	@param material The new material
	*/
	void setMaterial(Material *material);

	/**
	@see Object.h
	*/
	void render();

	/**
	Returns "mesh"
	@see Object.h
	*/
	string getType();

	void fillData(float* coordinates, float* textureCoordinates, float* normals, unsigned int nVertices, unsigned int* faces, unsigned int nFaces);
};

