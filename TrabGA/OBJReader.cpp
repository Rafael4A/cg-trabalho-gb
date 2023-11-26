#include "OBJReader.h"
#include "MTLReader.h"

Obj3D* OBJReader::readFile(string pathfile) {
	MTLReader mtlreader;
	Obj3D* obj = new Obj3D;
	Mesh* meshObj = new Mesh;
	obj->mesh = meshObj;
	bool isFirstGroup = true;
	float xMin, yMin, zMin, xMax, yMax, zMax;
	bool firstV = true;
	Group* currentGroup = new Group;
	ifstream file(pathfile);

	while (!file.eof()) {
		string line;
		getline(file, line);

		stringstream sline;
		sline << line;

		string temp;
		sline >> temp;

		if (temp == "o") {
			string name;
			sline >> name;
			obj->name = name;
		}

		if (temp == "g") {
			string name;
			sline >> name;

			if (isFirstGroup) {
				meshObj->groups.push_back(currentGroup);
				currentGroup->name = name;
				isFirstGroup = false;
			}
			else {
				Group* newGroup = new Group;
				newGroup->name = name;
				currentGroup = newGroup;
				meshObj->groups.push_back(currentGroup);
			}
		}

		else if (temp == "usemtl") {
			string materialName;
			sline >> materialName;
			currentGroup->materialName = materialName;

			for (Material* e : meshObj->materials) {
				if (e->id == materialName) {
					currentGroup->mat = e;
				}
			}
		}

		else if (temp == "mtllib") {
			string matFile;
			sline >> matFile;
			meshObj->mtllib = matFile;
			string copyPathFile = pathfile;
			string newPath;

			for (int i = pathfile.length(); i > 0; i--) {
				if (pathfile[i] != '/') {
				}
				else {
					newPath = copyPathFile.erase(i + 1);
					break;
				}
			}
			obj->filePath = newPath;
			meshObj->materials = mtlreader.readFile(newPath + matFile);
		}

		else if (temp == "v") {
			float x, y, z;
			sline >> x >> y >> z;

			if (firstV) {
				xMin = x, yMin = y, zMin = z, xMax = x, yMax = y, zMax = z;
				firstV = false;
			}
			if (x < xMin) {
				xMin = x;
			}
			if (y < yMin) {
				yMin = y;
			}
			if (z < zMin) {
				zMin = z;
			}
			if (x > xMax) {
				xMax = x;
			}
			if (y > yMax) {
				yMax = y;
			}
			if (z > zMax) {
				zMax = z;
			}

			glm::vec3* vertex = new glm::vec3(x, y, z);
			meshObj->vertex.push_back(vertex);
		}

		else if (temp == "vt") {
			float x, y;
			sline >> x >> y;
			glm::vec2* texture = new glm::vec2(x, y);
			meshObj->mappings.push_back(texture);
		}

		else if (temp == "vn") {
			float x, y, z;
			sline >> x >> y >> z;
			glm::vec3* normal = new glm::vec3(x, y, z);
			meshObj->normals.push_back(normal);
		}

		else if (temp == "f") {
			Face* face = new Face;
			string token;
			while (sline >> token) {
				size_t pos = token.find("//");

				if (pos != token.npos) {
					int firstElement = stoi(token.substr(0, pos));
					int secondElement = stoi(token.substr(pos + 2));
					face->verts.push_back(firstElement - 1);
					face->norms.push_back(secondElement - 1);
					face->texts.push_back(-1);
				}
				else {
					stringstream stoken;
					stoken << token;
					string aux;
					int faceValues[3];
					int i = 0;
					while (getline(stoken, aux, '/')) {
						faceValues[i] = stoi(aux) - 1;
						i++;
					}
					face->verts.push_back(faceValues[0]);
					face->texts.push_back(faceValues[1]);
					face->norms.push_back(faceValues[2]);
				}
			}
			currentGroup->faces.push_back(face);
		}
	}

	if (isFirstGroup) {
		meshObj->groups.push_back(currentGroup);
		currentGroup->name = "default";
		isFirstGroup = false;
	}

	glm::vec4 minVertex = glm::vec4(xMin, yMin, zMin, 1.0);
	glm::vec4 maxVertex = glm::vec4(xMax, yMax, zMax, 1.0);
	meshObj->min = minVertex;
	meshObj->max = maxVertex;

	return obj;
}
