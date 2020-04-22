#include "Engine.h"
#include "GL/glew.h"
#include "GL/freeglut.h"

LIB_API  Mesh::Mesh() : Node()
{

}

LIB_API  Mesh::~Mesh()
{
	glDeleteBuffers(2, m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Material LIB_API * Mesh::getMaterial()
{
	return this->material;
}

void LIB_API Mesh::setMaterial(Material *material)
{
	this->material = material;
}

void LIB_API Mesh::render()
{
	if (material != nullptr)
	{
		material->render();
	}

	// Bind vertex array
	glBindVertexArray(m_vaoID);
	// Render primitives (trianglese) from array data
	glDrawElements(GL_TRIANGLES, 3 * m_numFaces, GL_UNSIGNED_INT, nullptr);
	// Disable VAO when not needed:
	glBindVertexArray(0);
}

string LIB_API Mesh::getType()
{
	return "mesh";
}

void LIB_API Mesh::fillData(
	float* coordinates, 
	float* textureCoordinates, 
	float* normals, 
	unsigned int nVertices, 
	unsigned int* faces, 
	unsigned int nFaces)
{
	// Save number of vertices and number of faces
	m_numVertices = nVertices;
	m_numFaces = nFaces;

	// Generate a vertex array
	glGenVertexArrays(1, &m_vaoID);
	// Generate two vertex buffer: 
	// - one for vertices (coordinates/normals/texturecoordinates)
	// - one for faces
	glGenBuffers(2, m_vboID);

	// Bind vertex array
	glBindVertexArray(m_vaoID);

	// Working on vertices vertex buffer (bind)
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID[0]); // bind it
	// Copy the vertex data from system to video memory. 
	// The vertex data include coordinates, normals and textureCoordinates,
	// so a dimension of 3 + 3 + 2 = 8 (float) for each vertex... multiplying * N vertices
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * nVertices, nullptr, GL_STATIC_DRAW); // no data for now
	// glBufferSubData updates a subset of a buffer object's data store
	// SPECIFICATION: void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
	// (https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBufferSubData.xml)
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * nVertices, coordinates); // copy coordinates values
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * nVertices, 3 * sizeof(float) * nVertices, normals); // copy normals
	glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * nVertices, 2 * sizeof(float) * nVertices, textureCoordinates); // copy texture coordinates
	
	glVertexAttribPointer((GLuint) 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); //coordinates
	glVertexAttribPointer((GLuint) 1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * nVertices)); //normals
	glVertexAttribPointer((GLuint) 2, 2, GL_FLOAT, GL_FALSE, 0, (void*)((3 + 3) * sizeof(float) * nVertices)); //textureCoordinates

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);																									  
																																				  
	// Working on faces vertex buffer (bind)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboID[1]);
	// Copy the face index data from system to video memory
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int) * nFaces, faces, GL_STATIC_DRAW);

	// Disable VAO when not needed:
	glBindVertexArray(0);

	// free in memory data arrays after have the copy to video memory
	delete[] coordinates;
	delete[] textureCoordinates;
	delete[] normals;
	delete[] faces;
}