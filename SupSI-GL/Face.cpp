#include "Engine.h"



LIB_API Face::Face()
{
}


LIB_API Face::~Face()
{
}

vector<Vertex*> LIB_API Face::getVertices()
{
	return vertices;
}
void LIB_API Face::addVertex(Vertex* v)
{
	vertices.push_back(v);
}
