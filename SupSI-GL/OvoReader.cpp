#include "Engine.h"
#include <stack> 
//needed to create hierarchy
stack<Node*> stackNode;
stack<int> stackNr;
Node* root;
void createHierarchy(Node* n, int children)
{
	if (stackNode.size() == 0) 
	{
		stackNode.push(n);
		stackNr.push(children);
		root = n;
		return;
	}
	Node* parent = stackNode.top();
	n->setParent(parent);
	int topNr = stackNr.top();
	topNr--;
	stackNr.pop();
	stackNr.push(topNr);
	if (topNr == 0) 
	{
		stackNode.pop();
		stackNr.pop();
	}
	if (children != -1 && children != 0)
	{
		stackNode.push(n);
		stackNr.push(children);
	}
}


/**
 * Read a OVO file and returns the node root of node containing OVO file's data
 * @param  name the filename of the OVO file
 * @return a list of node containing the scene elements
 */
Node* OvoReader::readOVOfile(const char * name)
{

	stackNode = {};
	stackNr = {};
	cout << stackNode.size() << endl;
	vector<Material*> materials;
	FILE *dat = fopen(name, "rb");
	ofstream f ("../resources/propertyFile.txt");
	// Configure stream:
	cout.precision(2);  // 2 decimals are enough
	cout << fixed;      // Avoid scientific notation
	// Parse chunks:
	unsigned int chunkId, chunkSize;
	while (true)
	{
		fread(&chunkId, sizeof(unsigned int), 1, dat);
		if (feof(dat))
			break;
		fread(&chunkSize, sizeof(unsigned int), 1, dat);
		f << "[chunk id: " << chunkId << ", chunk size: " << chunkSize << ", chunk type: ";
		// Load whole chunk into memory:
		char *data = new char[chunkSize];
		if (fread(data, sizeof(char), chunkSize, dat) != chunkSize)
		{
			f << "ERROR: unable to read from file '" << name << "'" << endl;
			fclose(dat);
			delete[] data;
		}
		// Parse chunk information according to its type:
		unsigned int position = 0;
		switch ((OvObject::Type) chunkId)
		{
			///////////////////////////////
		case OvObject::Type::OBJECT: //
		{
			f << "version]" << endl;
			// OVO revision number:
			unsigned int versionId;
			memcpy(&versionId, data + position, sizeof(unsigned int));
			f << "   Version . . . :  " << versionId << endl;
			position += sizeof(unsigned int);
		}
		break;
		/////////////////////////////
		case OvObject::Type::NODE: //
		{
			f << "node]" << endl;
			// Node name:
			char nodeName[FILENAME_MAX];
			strcpy(nodeName, data + position);
			f << "   Name  . . . . :  " << nodeName << endl;
			position += (unsigned int)strlen(nodeName) + 1;
			// Node matrix:
			glm::mat4 matrix;
			memcpy(&matrix, data + position, sizeof(glm::mat4));
			MAT2STR(f, matrix);
			position += sizeof(glm::mat4);
			// Nr. of children nodes:
			unsigned int children;
			memcpy(&children, data + position, sizeof(unsigned int));
			f << "   Nr. children  :  " << children << endl;
			position += sizeof(unsigned int);
			// Optional target node, [none] if not used:
			char targetName[FILENAME_MAX];
			strcpy(targetName, data + position);
			f << "   Target node . :  " << targetName << endl;
			position += (unsigned int)strlen(targetName) + 1;
			Node* node = new Node();
			node->setName(nodeName);
			node->setPosMatrix(matrix);
			createHierarchy(node,children);
		}
		break;
		/////////////////////////////////
		case OvObject::Type::MATERIAL: //
		{
			f << "material]" << endl;
			// Material name:
			char materialName[FILENAME_MAX];
			strcpy(materialName, data + position);
			f << "   Name  . . . . :  " << materialName << endl;
			position += (unsigned int)strlen(materialName) + 1;
			// Material term colors, starting with emissive:
			glm::vec3 emission, albedo;
			memcpy(&emission, data + position, sizeof(glm::vec3));
			f << "   Emission  . . :  " << emission.r << ", " << emission.g << ", " << emission.b << endl;
			position += sizeof(glm::vec3);
			// Albedo:
			memcpy(&albedo, data + position, sizeof(glm::vec3));
			f << "   Albedo  . . . :  " << albedo.r << ", " << albedo.g << ", " << albedo.b << endl;
			position += sizeof(glm::vec3);
			// Roughness factor:
			float roughness;
			memcpy(&roughness, data + position, sizeof(float));
			f << "   Roughness . . :  " << roughness << endl;
			position += sizeof(float);
			// Metalness factor:
			float metalness;
			memcpy(&metalness, data + position, sizeof(float));
			f << "   Metalness . . :  " << metalness << endl;
			position += sizeof(float);
			// Transparency factor:
			float alpha;
			memcpy(&alpha, data + position, sizeof(float));
			f << "   Transparency  :  " << alpha << endl;
			position += sizeof(float);
			// Albedo texture filename, or [none] if not used:
			char textureName[FILENAME_MAX];
			strcpy(textureName, data + position);
			f << "   Albedo tex. . :  " << textureName << endl;
			position += (unsigned int)strlen(textureName) + 1;
			// Normal map filename, or [none] if not used:
			char normalMapName[FILENAME_MAX];
			strcpy(normalMapName, data + position);
			f << "   Normalmap tex.:  " << normalMapName << endl;
			position += (unsigned int)strlen(normalMapName) + 1;
			// Height map filename, or [none] if not used:
			char heightMapName[FILENAME_MAX];
			strcpy(heightMapName, data + position);
			f << "   Heightmap tex.:  " << heightMapName << endl;
			position += (unsigned int)strlen(heightMapName) + 1;
			// Roughness map filename, or [none] if not used:
			char roughnessMapName[FILENAME_MAX];
			strcpy(roughnessMapName, data + position);
			f << "   Roughness tex.:  " << roughnessMapName << endl;
			position += (unsigned int)strlen(roughnessMapName) + 1;
			// Metalness map filename, or [none] if not used:
			char metalnessMapName[FILENAME_MAX];
			strcpy(metalnessMapName, data + position);
			f << "   Metalness tex.:  " << metalnessMapName << endl;
			position += (unsigned int)strlen(metalnessMapName) + 1;
			Material *material = new Material();
			material->setEmission(glm::vec4(emission.r, emission.g, emission.b, 1.0f));
			material->setShininess((1-sqrt(roughness))*128);
			material->setName(materialName);
			string sName{textureName};
			if (sName.compare("[none]") == 0)
			{
				material->setTexture(nullptr);
			}
			else
			{
				Texture* texture = new Texture(textureName);
				material->setTexture(texture);
			}
			glm::vec4 albedo4 = glm::vec4(albedo, alpha);

			material->setAmbient(albedo4*0.2f);
			material->setSpecular(albedo4*0.4f);
			material->setDiffuse(albedo4*0.6f);
			materials.push_back(material);
		}
		break;
		// Both standard and skinned meshes are handled through this case:
		////////////////////////////////
		case OvObject::Type::MESH:    //
		case OvObject::Type::SKINNED:
		{
			bool isSkinned = false;
			if ((OvObject::Type) chunkId == OvObject::Type::SKINNED)
			{
				isSkinned = true;
				f << "skinned mesh]" << endl;
			}
			else
				f << "mesh]" << endl;
			// Mesh name:
			char meshName[FILENAME_MAX];
			strcpy(meshName, data + position);
			position += (unsigned int)strlen(meshName) + 1;
			f << "   Name  . . . . :  " << meshName << endl;
			// Mesh matrix:
			glm::mat4 matrix;
			memcpy(&matrix, data + position, sizeof(glm::mat4));
			MAT2STR(f, matrix);
			position += sizeof(glm::mat4);
			// Mesh nr. of children nodes:
			unsigned int children;
			memcpy(&children, data + position, sizeof(unsigned int));
			f << "   Nr. children  :  " << children << endl;
			position += sizeof(unsigned int);
			// Optional target node, or [none] if not used:
			char targetName[FILENAME_MAX];
			strcpy(targetName, data + position);
			f << "   Target node . :  " << targetName << endl;
			position += (unsigned int)strlen(targetName) + 1;
			// Mesh subtype (see OvMesh SUBTYPE enum):
			unsigned char subtype;
			memcpy(&subtype, data + position, sizeof(unsigned char));
			char subtypeName[FILENAME_MAX];
			switch ((OvMesh::Subtype) subtype)
			{
			case OvMesh::Subtype::DEFAULT:
				strcpy(subtypeName, "standard");
				break;
			case OvMesh::Subtype::NORMALMAPPED:
				strcpy(subtypeName, "normal-mapped");
				break;
			case OvMesh::Subtype::TESSELLATED:
				strcpy(subtypeName, "tessellated");
				break;
			default:
				strcpy(subtypeName, "UNDEFINED");
			}
			f << "   Subtype . . . :  " << (int)subtype << " (" << subtypeName << ")" << endl;
			position += sizeof(unsigned char);
			// Nr. of vertices:
			unsigned int vertices, faces;
			memcpy(&vertices, data + position, sizeof(unsigned int));
			f << "   Nr. vertices  :  " << vertices << endl;
			position += sizeof(unsigned int);
			// ...and faces:
			memcpy(&faces, data + position, sizeof(unsigned int));
			f << "   Nr. faces . . :  " << faces << endl;
			position += sizeof(unsigned int);
			// Material name, or [none] if not used:
			char materialName[FILENAME_MAX];
			strcpy(materialName, data + position);
			f << "   Material  . . :  " << materialName << endl;
			position += (unsigned int)strlen(materialName) + 1;
			// Mesh bounding sphere radius:
			float radius;
			memcpy(&radius, data + position, sizeof(float));
			f << "   Radius  . . . :  " << radius << endl;
			position += sizeof(float);
			// Mesh bounding box minimum corner:
			glm::vec3 bBoxMin;
			memcpy(&bBoxMin, data + position, sizeof(glm::vec3));
			f << "   BBox minimum  :  " << bBoxMin.x << ", " << bBoxMin.y << ", " << bBoxMin.z << endl;
			position += sizeof(glm::vec3);

			// Mesh bounding box maximum corner:
			glm::vec3 bBoxMax;
			memcpy(&bBoxMax, data + position, sizeof(glm::vec3));
			f << "   BBox maximum  :  " << bBoxMax.x << ", " << bBoxMax.y << ", " << bBoxMax.z << endl;
			position += sizeof(glm::vec3);

			// Optional physics properties:
			unsigned char hasPhysics;
			memcpy(&hasPhysics, data + position, sizeof(unsigned char));
			f << "   Physics . . . :  " << (int)hasPhysics << endl;
			position += sizeof(unsigned char);
			if (hasPhysics)
			{
				/**
				 * Mesh physics properties.
				 */

				struct PhysProps
				{
					// Pay attention to 16 byte alignement (use padding):
					unsigned char type;
					unsigned char contCollisionDetection;
					unsigned char collideWithRBodies;
					unsigned char hullType;
					// Vector data:
					glm::vec3 massCenter;
					// Mesh properties:
					float mass;
					float staticFriction;
					float dynamicFriction;
					float bounciness;
					float linearDamping;
					float angularDamping;
					void *physObj;
				};

				PhysProps mp;
				memcpy(&mp, data + position, sizeof(PhysProps));
				position += sizeof(PhysProps);
				f << "      Type . . . :  " << (int)mp.type << endl;
				f << "      Hull type  :  " << (int)mp.hullType << endl;
				f << "      Cont. coll.:  " << (int)mp.contCollisionDetection << endl;
				f << "      Col. bodies:  " << (int)mp.collideWithRBodies << endl;
				f << "      Center . . :  " << mp.massCenter.x << ", " << mp.massCenter.y << ", " << mp.massCenter.z << endl;
				f << "      Mass . . . :  " << mp.mass << endl;
				f << "      Static . . :  " << mp.staticFriction << endl;
				f << "      Dynamic  . :  " << mp.dynamicFriction << endl;
				f << "      Bounciness :  " << mp.bounciness << endl;
				f << "      Linear . . :  " << mp.linearDamping << endl;
				f << "      Angular  . :  " << mp.angularDamping << endl;
			}
			// Extra information for skinned meshes:
			if (isSkinned)
			{
				// Initial mesh pose matrix:
				glm::mat4 poseMatrix;
				memcpy(&poseMatrix, data + position, sizeof(glm::mat4));
				MAT2STR(f, poseMatrix);
				position += sizeof(glm::vec4);

				// Bone list:
				unsigned int nrOfBones;
				memcpy(&nrOfBones, data + position, sizeof(unsigned int));
				f << "   Nr. bones . . :  " << nrOfBones << endl;
				position += sizeof(unsigned int);

				for (unsigned int c = 0; c < nrOfBones; c++)
				{
					// Bone name:
					char boneName[FILENAME_MAX];
					strcpy(boneName, data + position);
					f << "      Bone name  :  " << boneName << " (" << c << ")" << endl;
					position += (unsigned int)strlen(boneName) + 1;

					// Initial bone pose matrix (already inverted):
					glm::mat4 boneMatrix;
					memcpy(&boneMatrix, data + position, sizeof(glm::mat4));
					MAT2STR(f, boneMatrix);
					position += sizeof(glm::mat4);
				}

				// Per vertex bone weights and indexes:
				for (unsigned int c = 0; c < vertices; c++)
				{
					f << "   Bone data . . :  v" << c << endl;

					// Bone indexes:
					unsigned int boneIndex[4];
					memcpy(boneIndex, data + position, sizeof(unsigned int) * 4);
					f << "      index  . . :  " << boneIndex[0] << ", " << boneIndex[1] << ", " << boneIndex[2] << ", " << boneIndex[3] << endl;
					position += sizeof(unsigned int) * 4;

					// Bone weights:
					unsigned short boneWeightData[4];
					memcpy(boneWeightData, data + position, sizeof(unsigned short) * 4);
					glm::vec4 boneWeight;
					boneWeight.x = glm::unpackHalf1x16(boneWeightData[0]);
					boneWeight.y = glm::unpackHalf1x16(boneWeightData[1]);
					boneWeight.z = glm::unpackHalf1x16(boneWeightData[2]);
					boneWeight.w = glm::unpackHalf1x16(boneWeightData[3]);
					f << "      weight . . :  " << boneWeight.x << ", " << boneWeight.y << ", " << boneWeight.z << ", " << boneWeight.w << endl;
					position += sizeof(unsigned short) * 4;
				}
			}

			float* coordinatesArray = new float[vertices * 3];
			float* textureCoordinatesArray = new float[vertices * 2];
			float* normalsArray = new float[vertices * 3];

			// Interleaved and compressed vertex/normal/UV/tangent data:
			for (unsigned int c = 0; c < vertices; c++)
			{
				// Vertex coords:
				glm::vec3 vertex;
				memcpy(&vertex, data + position, sizeof(glm::vec3));
				position += sizeof(glm::vec3);
				coordinatesArray[c * 3] = vertex.x;
				coordinatesArray[c * 3 + 1] = vertex.y;
				coordinatesArray[c * 3 + 2] = vertex.z;

				// Vertex normal:
				unsigned int normalData;
				memcpy(&normalData, data + position, sizeof(unsigned int));
				position += sizeof(unsigned int);
				glm::vec4 normal = glm::unpackSnorm3x10_1x2(normalData);
				normalsArray[c * 3] = normal.x;
				normalsArray[c * 3 + 1] = normal.y;
				normalsArray[c * 3 + 2] = normal.z;

				// Texture coordinates:
				unsigned short textureData[2];
				memcpy(textureData, data + position, sizeof(unsigned short) * 2);
				position += sizeof(unsigned short) * 2;

				glm::vec2 uv;
				uv.x = glm::unpackHalf1x16(textureData[0]);
				uv.y = glm::unpackHalf1x16(textureData[1]);
				textureCoordinatesArray[c * 2] = uv.x;
				textureCoordinatesArray[c * 2 + 1] = uv.y;

				// Tangent vector:
				unsigned int tangentData;
				memcpy(&tangentData, data + position, sizeof(unsigned int));
				position += sizeof(unsigned int);
			}

			unsigned int face[3];
			unsigned int* facesArray = new unsigned int[faces * 3];
			for (unsigned int c = 0; c < faces; c++)
			{
				// Face indexes:
				//f << "   Face data . . :  f" << c << " (" << face[0] << ", " << face[1] << ", " << face[2] << ")" << endl;
				memcpy(face, data + position, sizeof(unsigned int) * 3);
				position += sizeof(unsigned int) * 3;
				facesArray[c * 3] = face[0];
				facesArray[c * 3 + 1] = face[1];
				facesArray[c * 3 + 2] = face[2];
			}
			Material *material = new Material();
			for (vector<Material*>::iterator it = materials.begin(); it != materials.end(); ++it)
			{
				if ((*it)->getName().compare(materialName) == 0) {
					material = *it;
					break;
				}
			}
			Mesh *mesh = new Mesh();
			mesh->setName(meshName);
			mesh->setPosMatrix(matrix);
			mesh->setMaterial(material);
			mesh->fillData(coordinatesArray, textureCoordinatesArray, normalsArray, vertices, facesArray, faces);
			createHierarchy(mesh, children);
		}
		break;
		//////////////////////////////
		case OvObject::Type::LIGHT: //
		{
			f << "light]" << endl;
			// Light name:
			char lightName[FILENAME_MAX];
			strcpy(lightName, data + position);
			f << "   Name  . . . . :  " << lightName << endl;
			position += (unsigned int)strlen(lightName) + 1;

			// Light matrix:
			glm::mat4 matrix;
			memcpy(&matrix, data + position, sizeof(glm::mat4));
			MAT2STR(f, matrix);
			position += sizeof(glm::mat4);

			// Nr. of children nodes:
			unsigned int children;
			memcpy(&children, data + position, sizeof(unsigned int));
			f << "   Nr. children  :  " << children << endl;
			position += sizeof(unsigned int);

			// Optional target node name, or [none] if not used:
			char targetName[FILENAME_MAX];
			strcpy(targetName, data + position);
			f << "   Target node . :  " << targetName << endl;
			position += (unsigned int)strlen(targetName) + 1;

			// Light subtype (see OvLight SUBTYPE enum):
			unsigned char subtype;
			memcpy(&subtype, data + position, sizeof(unsigned char));
			char subtypeName[FILENAME_MAX];
			f << "   Subtype . . . :  " << (int)subtype << " (" << subtypeName << ")" << endl;
			position += sizeof(unsigned char);

			// Light color:
			glm::vec3 color;
			memcpy(&color, data + position, sizeof(glm::vec3));
			f << "   Color . . . . :  " << color.r << ", " << color.g << ", " << color.b << endl;
			position += sizeof(glm::vec3);

			// Influence radius:
			float radius;
			memcpy(&radius, data + position, sizeof(float));
			f << "   Radius  . . . :  " << radius << endl;
			position += sizeof(float);

			// Direction:
			glm::vec3 direction;
			memcpy(&direction, data + position, sizeof(glm::vec3));
			f << "   Direction . . :  " << direction.r << ", " << direction.g << ", " << direction.b << endl;
			position += sizeof(glm::vec3);

			// Cutoff:
			float cutoff;
			memcpy(&cutoff, data + position, sizeof(float));
			f << "   Cutoff  . . . :  " << cutoff << endl;
			position += sizeof(float);

			// Exponent:
			float spotExponent;
			memcpy(&spotExponent, data + position, sizeof(float));
			f << "   Spot exponent :  " << spotExponent << endl;
			position += sizeof(float);

			// Cast shadow flag:
			unsigned char castShadows;
			memcpy(&castShadows, data + position, sizeof(unsigned char));
			f << "   Cast shadows  :  " << (int)castShadows << endl;
			position += sizeof(unsigned char);

			// Volumetric lighting flag:
			unsigned char isVolumetric;
			memcpy(&isVolumetric, data + position, sizeof(unsigned char));
			f << "   Volumetric  . :  " << (int)isVolumetric << endl;
			position += sizeof(unsigned char);
			Light *light = new Light();
			light->setName(lightName);
			switch ((OvLight::Subtype) subtype)
			{
			case OvLight::Subtype::DIRECTIONAL:
				strcpy(subtypeName, "directional");
				light->setW(0);
				break;
			case OvLight::Subtype::OMNI:
				strcpy(subtypeName, "omni");
				break;
			case OvLight::Subtype::SPOT:
				strcpy(subtypeName, "spot");
				break;
			default:
				strcpy(subtypeName, "UNDEFINED");
				break;
			}
			light->setPosMatrix(matrix);
			light->setColor(glm::vec4(color.r, color.g, color.b, 1.0f));
			light->setDirection(glm::vec4(direction.r, direction.g, direction.b, 1.0f));
			light->setCutoff(cutoff);
			createHierarchy(light, children);
		}
		break;

		/////////////////////////////
		case OvObject::Type::BONE: //
		{
			f << "bone]" << endl;

			// Bone name:
			char boneName[FILENAME_MAX];
			strcpy(boneName, data + position);
			f << "   Name  . . . . :  " << boneName << endl;
			position += (unsigned int)strlen(boneName) + 1;

			// Bone matrix:
			glm::mat4 matrix;
			memcpy(&matrix, data + position, sizeof(glm::mat4));
			MAT2STR(f, matrix);
			position += sizeof(glm::mat4);

			// Nr. of children nodes:
			unsigned int children;
			memcpy(&children, data + position, sizeof(unsigned int));
			f << "   Nr. children  :  " << children << endl;
			position += sizeof(unsigned int);

			// Optional target node, or [none] if not used:
			char targetName[FILENAME_MAX];
			strcpy(targetName, data + position);
			f << "   Target node . :  " << targetName << endl;
			position += (unsigned int)strlen(targetName) + 1;

			// Mesh bounding box minimum corner:
			glm::vec3 bBoxMin;
			memcpy(&bBoxMin, data + position, sizeof(glm::vec3));
			f << "   BBox minimum  :  " << bBoxMin.x << ", " << bBoxMin.y << ", " << bBoxMin.z << endl;
			position += sizeof(glm::vec3);

			// Mesh bounding box maximum corner:
			glm::vec3 bBoxMax;
			memcpy(&bBoxMax, data + position, sizeof(glm::vec3));
			f << "   BBox maximum  :  " << bBoxMax.x << ", " << bBoxMax.y << ", " << bBoxMax.z << endl;
			position += sizeof(glm::vec3);
		}
		break;
		///////////
		default: //
			f << "UNKNOWN]" << endl;
			f << "ERROR: corrupted or bad data in file " << name << endl;
			fclose(dat);
			delete[] data;
		}
		// Release chunk memory:
		delete[] data;
	}
	// Done:
	fclose(dat);
	cout << "\nFile parsed" << endl;

	return root;
}


