#pragma once

#define OV_MAXNUMBEROFCHARS 256

/**
 * Macro for printing an OvMatrix4 to console:
 */
#define MAT2STR(f, m) f << "   Matrix  . . . :  \t" << m[0][0] << "\t" << m[1][0] << "\t" << m[2][0] << "\t" << m[3][0] << std::endl \
                           << "                    \t" << m[0][1] << "\t" << m[1][1] << "\t" << m[2][1] << "\t" << m[3][1] << std::endl \
                           << "                    \t" << m[0][2] << "\t" << m[1][2] << "\t" << m[2][2] << "\t" << m[3][2] << std::endl \
                           << "                    \t" << m[0][3] << "\t" << m[1][3] << "\t" << m[2][3] << "\t" << m[3][3] << std::endl

/**
* Supsi-GE, OVO files binary chunk identifier container
* This class is used to enumerate the variuos binary sections
* in an OVO file.
* to be used in the main OvoReader class
* Some perfomance tuning were added by J.Petralli.
* 
* @authors A.Peternier, J.Petralli, D.Nasi, D.Calabria
*/
class OvObject
{
public:
	/**
	@enum Type
	The subtypes identifications of data chunks
	*/
	enum class Type : int  ///< Type of entities
	{
		// Foundation types:
		OBJECT = 0,
		NODE,
		OBJECT2D,
		OBJECT3D,
		LIST,

		// Derived classes:
		BUFFER,
		SHADER,
		TEXTURE,
		FILTER,
		MATERIAL,
		FBO,
		QUAD,
		BOX,
		SKYBOX,
		FONT,
		CAMERA,
		LIGHT,
		BONE,
		MESH,	   // Keep them...
		SKINNED, // ...consecutive
		INSTANCED,
		PIPELINE,
		EMITTER,

		// Animation type
		ANIM,

		// Physics related:
		PHYSICS,

		// Terminator:
		LAST,
	};
};

// Stripped-down redefinition of OvMesh (just for the subtypes):
/**
* Supsi-GE, OVO files binary chunk meshes container
* This class is used to enumerate the variuos meshes
* in an OVO file.
* Particularly, it enumerate its subtypes.
* To be used in the main OvoReader class
* Some perfomance tuning were added by J.Petralli.
*
* @authors A.Peternier, J.Petralli, D.Nasi, D.Calabria
*/
class OvMesh
{
public:
	/**
	@enum Subtype
	The subtypes identifications of meshes
	*/
	enum class Subtype : int ///< Kind of mesh
	{
		// Foundation types:
		DEFAULT = 0,
		NORMALMAPPED,
		TESSELLATED,

		// Terminator:
		LAST,
	};
};

// Stripped-down redefinition of OvLight (just for the subtypes):
/**
* Supsi-GE, OVO files binary chunk light container
* This class is used to enumerate the variuos lights
* in an OVO file.
* Particularly, it enumerate its subtypes.
* To be used in the main OvoReader class
* Some perfomance tuning were added by J.Petralli.
*
* @authors A.Peternier, J.Petralli, D.Nasi, D.Calabria
*/
class OvLight
{
public:
	/**
	@enum Subtype
	The subtypes identifications of lights
	*/
	enum class Subtype : int ///< Kind of light
	{
		// Foundation types:
		OMNI = 0,
		DIRECTIONAL,
		SPOT,

		// Terminator:
		LAST,
	};
};


/**
* Supsi-GE, OVO files reader class
* This class reads a file in the OverVision Object format,
* a binaray chunked scene container created by
* prof. A.Peternier. Some perfomance tuning were added by J.Petralli.
*
* @authors A.Peternier, J.Petralli, D.Nasi, D.Calabria
*/
class OvoReader
{
public:

	/**
	Opens and reads the "name" OVO file,
	returning the scene graph's root node and its childrens.
	@param name The file's name
	*/
	Node* readOVOfile(const char * name);
private:
};
