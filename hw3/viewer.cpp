/*
 * program3 - Geometry Viewer
 * @author  - Aditya Kousik
 * @email   - adit267.kousik@gmail.com
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

#include <GL/gl.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#include "vgl.h"
#include "LoadShaders.h"


enum metaList { _META_OBJ_CMD, _META_MTL_CMD, _META_CONTROL,
				_META_LIGHT_TYPES, _META_LIGHT_ATTR, _META_VIEW_ATTR,
				_META_LIGHT_SHADER_PROPS, _NUM_META_LIST};

enum objCmd {_INVALID = -1, _V, _VT, _VN, _MTLLIB, _USEMTL, _NEWMTL,
		  _F, _S, _G, _O, _NUM_CMDS};
enum mtlCmd {_MTL_INVALID = -1, _MTL_NEWMTL, _KA, _KD, _KS, _NS, _NUM_MTL_CMDS};
enum control {_CTL_INVALID = -1, _OBJ, _RX, _RY, _RZ, _T, _CTL_S, _LIGHT, _VIEW, _NUM_CTL_CMDS};
enum bufferIndex { _POS, _NORM_POS, _BUF_KA, _BUF_KD, _NUM_INDICES};

/* Light attribute enums */
enum LightTypes {_LTYPE_INV = -1, _LOCAL, _SPOT, _DIRECTIONAL, _NUM_LTYPES};
enum LightAttr { _LA_INV = -1, _LA_AMBIENT, _LA_COLOR, _LA_POSITION,
				 _LA_CONST_ATT, _LA_LINEAR_ATT, _LA_QUAD_ATT,
				 _LA_CONE_DIR, _LA_SPOT_COS_CUT_OFF, _LA_SPOT_EXP, _NUM_LATTR};
enum ViewAttr { _VA_INV = -1, _VA_CAMERA, _VA_FOCAL, _VA_VIEWUP, _NUM_VATTR};
enum LightShaderProperties {_LS_INV = -1, _LS_IS_ENABLED, _LS_IS_LOCAL, _LS_IS_SPOT,
							_LS_AMBIENT, _LS_COLOR, _LS_POS, _LS_HALF_VECTOR,
							_LS_CONE_DIR, _LS_SPOT_COS_CUT_OFF, _LS_SPOT_EXP,
							_LS_CONST_ATT, _LS_LINEAR_ATT, _LS_QUAD_ATT, _NUM_LS_PROPS};


string objKeywords[] = { "v", "vt", "vn", "mtllib", "usemtl", "newmtl",
					  "f", "s", "g", "o"};
string mtlKeywords[] = { "newmtl", "Ka", "Kd", "Ks", "Ns"};
string controls[] = {"obj", "rx", "ry", "rz", "t", "s", "light", "view"};

/* Light attribute strings */
string lightTypes[] = {"local", "spot", "directional"};
string lightAttr[] =  {	"ambient", "color", "position",
						"constAtt", "linearAtt", "quadAtt",
						"coneDirection", "spotCosCutoff", "spotExponent"};
string viewAttr[] = {"camera", "focal", "viewup"};
string lightUniformVariable = "Lights";

string lightShaderProp[] = {"isEnabled", "isLocal", "isSpot",
							"ambient", "color", "position",
							"halfVector", "coneDirection", "spotCosCutoff",
							"spotExponent", "constantAttenuation",
							"linearAttenuation", "quadraticAttenuation"};

std::vector<std::vector<string > > metaCommands;
/* Object information*/


struct LightProperties {
    bool isEnabled;
    bool isLocal;
    bool isSpot;
    vec3 ambient;
    vec3 color;
    vec3 position;
    vec3 halfVector;
    vec3 coneDirection;
    float spotCosCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

// the set of lights to apply, per invocation of this shader
const int MAXLIGHTS = 4;
LightProperties Lights[MAXLIGHTS];
GLuint curLight = -1, numLights = 0;

GLuint numObj = 0;
GLfloat **objVertexPosData, **objVertexNormalData;
//GLfloat **objVertexKaColData, **objVertexKdColData;

GLuint activeMaterialIndex;
string materialFileName;

class MaterialData
{
	public:	string materialName;
			string materialPath;
			GLfloat Ka[3], Kd[3], Ks[3], Ns;
			std::vector <std::vector <std::vector<GLuint > > > faceIndexList;
};


class ViewerObject
{
	public: string objName;
			string objPath;
			GLuint numVertices;
			GLboolean isNormalThere;
			vector<string > materialFileList;
	std::vector< std::vector < GLfloat > > vertexPositions,
		normalPositions;
	std::vector< MaterialData > materialInfo;
};
std::vector <ViewerObject > viewerObjects;

/* GL data */
GLuint *VAOs, **objBuffers;
const GLuint numBuffers = 4;
const GLuint vPosition = _POS, normPosition = _NORM_POS;
const GLuint cKaPosition = _BUF_KA, cKdPosition = _BUF_KD;
GLuint programID;
void init(GLuint, GLuint, GLuint);
int windowHeight = 512, windowWidth = 512;

glm::vec3 Ka, Kd, Ks;
GLfloat Ns;

GLboolean setKa = GL_FALSE, setKd = GL_FALSE, setKs = GL_FALSE, setNs = GL_FALSE;

/* GLM based methods and variables*/

glm::vec3 maxCoord, camCoord, focPoint;
glm::mat4 Projection, View, Model, MV, MVP;
glm::mat3 Normal = mat3(1.0f);


glm::mat4 trans = mat4(1.0f);
glm::mat4 rot = mat4(1.0f);
glm::mat4 ctlTransform = mat4(1.0f);

glm::vec3 cameraCoord = vec3(3.f, 3.f, 2.f);
glm::vec3 focalCoord = vec3(0.f, 0.f, 0.f);
glm::vec3 viewCoord = vec3(0.f, 0.f, 1.f);
GLfloat xCam = cameraCoord.x, yCam = cameraCoord.y, zCam = cameraCoord.z;
GLfloat xFocal = focalCoord.x, yFocal = focalCoord.y, zFocal = focalCoord.z;
GLfloat xView, yView, zView;
glm::vec3 up = vec3(viewCoord);
glm::vec3 camDir, camR;

GLfloat rx,ry,rz, tx,ty,tz, sx,sy,sz;

GLfloat Near = 0.1f, Far = 400.0f;

void setupMVP();

/* Transformation matrics */


vec3 getMaxCoords()
{
	vec3 max = vec3(0,0,0);
	vec3 min = vec3(0, 0, 0);
	for (GLuint i = 0; i < viewerObjects.size(); ++i)
	{
		for (GLuint j = 0; j < viewerObjects[i].vertexPositions.size(); ++j)
		{

			//Max

			if (fabs(viewerObjects[i].vertexPositions[j][0]) > max.x)
				max.x = fabs(viewerObjects[i].vertexPositions[j][0]);
			if (fabs(viewerObjects[i].vertexPositions[j][1]) > max.y)
				max.y = fabs(viewerObjects[i].vertexPositions[j][1]);
			if (fabs(viewerObjects[i].vertexPositions[j][2]) > max.z)
				max.z = fabs(viewerObjects[i].vertexPositions[j][2]);
		
		}
	}

	maxCoord.x = 1.5 * max.x;
	maxCoord.y = 1.5 * max.y;
	maxCoord.z = 1.5 * max.z;

	return maxCoord;
}

/* setup Global Model View Projection */
void setupMVP()
{
	glm::vec3 eye(xCam, yCam, zCam);
	glm::vec3 center(xFocal, yFocal, zFocal);
	glm::vec3 viewUp;

	camDir = glm::normalize(eye - center);
	
	camR = glm::normalize(glm::cross(up, camDir));
	viewUp = glm::cross(camDir, camR);

	Projection = glm::frustum(-0.1f, .1f, -.1f, .1f, Near, Far);
	View = glm::lookAt(eye, center, viewUp);
	Model = glm::mat4(1.0f);  

	
	MV = View * Model * trans * ctlTransform;
	Normal = glm::mat3(glm::transpose(glm::inverse(MV)));
	MVP = Projection * View * Model * trans * ctlTransform;

}

void addDummyMaterial(int currentObj)
{
	MaterialData dummy;
	dummy.materialName = "default";
	dummy.Ka[0] = dummy.Ka[1] = dummy.Ka[2] = 0.1;
	dummy.Kd[0] = dummy.Kd[1] = dummy.Kd[2] = 0.9;
	dummy.Ks[0] = dummy.Ks[1] = dummy.Ks[2] = 1.0;
	dummy.Ns = 20;

	if(!setKa)
	{
		Ka.x = dummy.Ka[0];
		Ka.y = dummy.Ka[1];
		Ka.z = dummy.Ka[2];
		setKa = GL_TRUE;
	}

	if(!setKd)
	{
		Kd.x = dummy.Kd[0];
		Kd.y = dummy.Kd[1];
		Kd.z = dummy.Kd[2];
		setKd = GL_TRUE;
	}

	if(!setKs)
	{
		Ks.x = dummy.Ks[0];
		Ks.y = dummy.Ks[1];
		Ks.z = dummy.Ks[2];
		setKs = GL_TRUE;
	}

	if(!setNs)
	{
		Ns = dummy.Ns;
		setNs = GL_TRUE;

	}

	viewerObjects[currentObj].materialInfo.push_back(dummy);
}

// Allocate contiguous array of vertex pos, col;
GLfloat** allocateArray(GLuint numVertices, GLuint columns)
{
	GLfloat **temp, *vals;
	vals = (GLfloat*)malloc(numVertices * columns * sizeof(GLfloat));
	temp = (GLfloat**)malloc(numVertices * sizeof(GLfloat*));

	for (GLuint i = 0; i < numVertices; ++i)
		temp[i] = &(vals[columns * i]);

	return temp;
}

void setCommand()
{
	int i;
	std::vector<string > objCmdTemp, mtlCmdTemp, lightTypeTemp;
	std::vector<string > lightAttrTemp, viewAttrTemp;
	std::vector<string > lightPropTemp, ctlTemp;

	// Set _OBJ_CMD
	for(i = _V; i < _NUM_CMDS; ++i)
		objCmdTemp.push_back(objKeywords[i]);
	metaCommands.push_back(objCmdTemp);

	// Set _MTL_CMD
	for(i = _MTL_NEWMTL; i < _NUM_MTL_CMDS; ++i)
		mtlCmdTemp.push_back(mtlKeywords[i]);
	metaCommands.push_back(mtlCmdTemp);

	// Set _CONTROL
	for(i = _OBJ; i < _NUM_CTL_CMDS; ++i)
		ctlTemp.push_back(controls[i]);
	metaCommands.push_back(ctlTemp);

	// Set _LIGHT_TYPES
	for(i = _LOCAL; i < _NUM_LTYPES; ++i)
		lightTypeTemp.push_back(lightTypes[i]);
	metaCommands.push_back(lightTypeTemp);

	// Set _LIGHT_ATTR
	for(i = _LA_AMBIENT; i < _NUM_LATTR; ++i)
		lightAttrTemp.push_back(lightAttr[i]);
	metaCommands.push_back(lightAttrTemp);

	// Set _VIEW_ATTR
	for(i = _VA_CAMERA; i < _NUM_VATTR; ++i)
		viewAttrTemp.push_back(viewAttr[i]);
	metaCommands.push_back(viewAttrTemp);

	// Set _LIGHT_SHADER_PROPS
	for(i = _LS_IS_ENABLED; i < _NUM_LS_PROPS; ++i)
		lightPropTemp.push_back(lightShaderProp[i]);
	metaCommands.push_back(lightPropTemp);
}

GLuint getCommand(string& str, GLuint listType)
{
	for(int i = 0; i < metaCommands[listType].size(); ++i)
		if(metaCommands[listType][i].compare(str) == 0)
			return i;

	return -1;
}

GLuint getObjectIndex(string obj)
{
	for (GLuint i = 0; i < viewerObjects.size(); ++i)
		if (viewerObjects[i].objName.compare(obj) == 0)
			return i;

	return -1;
}

void getNumFaces(GLuint currentObj, GLuint& numVertices, GLuint& columns)
{
	numVertices = 0;
	columns = 3;
	for (int i = 0; i < viewerObjects[currentObj].materialInfo.size(); ++i)
		for (int j = 0; j < viewerObjects[currentObj].materialInfo[i].faceIndexList.size(); ++j)
			numVertices += viewerObjects[currentObj].materialInfo[i].faceIndexList[j].size();
}

GLboolean isMaterialFilePresent(int currentObj, string filename)
{
	for(int i = 0; i < viewerObjects[currentObj].materialFileList.size(); ++i)
		if(viewerObjects[currentObj].materialFileList[i].compare(filename) == 0) 
			return GL_TRUE;

	return GL_FALSE;
}

// Tokenizer
void tokenizeGeneral(const string &str,
	std::vector<string>& tokens,
	const string& delimiters = " \n\r")
{
	GLchar* bufToken = new GLchar[strlen(str.c_str()) + 1];
	GLchar* buf = new GLchar[strlen(str.c_str()) + 1];
	for (GLuint i = 0; i < strlen(str.c_str()); ++i)
	{
		buf[i] = str[i];
	}
	buf[strlen(str.c_str())] = '\0';
	bufToken = strtok(buf, delimiters.c_str());
	while (bufToken)
	{
		tokens.push_back(bufToken);
		bufToken = strtok(NULL, delimiters.c_str());
	}

	delete[] bufToken;
	delete[] buf;

	bufToken = NULL;
	buf = NULL;
}

/* Shameless hack 
 * f a/b/c | a//b | a b c
 */
void tokenizeFaceIndices(std::vector<string >& str,
	std::vector<std::vector< GLuint > >& tokens,
	const string& delimiters = "/")
{
	GLchar *bufStr, *bufToken;
	
	for (GLuint i = 1; i < str.size(); ++i)
	{
		bufStr = new GLchar[strlen(str[i].c_str()) + 1];
		bufToken = new GLchar[strlen(str[i].c_str()) + 1];
		strcpy(bufStr, str[i].c_str());
		std::vector<GLuint > indices;
		bufToken = strtok(bufStr, delimiters.c_str());
		while (bufToken)
		{
			indices.push_back(atoi(bufToken));
			bufToken = strtok(NULL, delimiters.c_str());
		}
		tokens.push_back(indices);
		delete[] bufStr;
		delete[] bufToken;

		bufStr = NULL;
		bufToken = NULL;
	}
}

GLuint getMaterialIndex(int currentObj, string materialName)
{
	for (GLuint i = 0; i < viewerObjects[currentObj].materialInfo.size(); ++i)
		if (viewerObjects[currentObj].materialInfo[i].materialName.compare(materialName) == 0)
			return i;
	return -1;
}

void loadVertexPosColData(GLuint activeObject)
{

	GLuint vertRow = 0, normalRow;
	// Material i
	for (GLuint i = 0; i < viewerObjects[activeObject].materialInfo.size(); ++i)
		// Line j starting with "f ..."
	{
		
		for (GLuint j = 0; j < viewerObjects[activeObject].materialInfo[i].faceIndexList.size(); ++j)
		{
			// k components in "f a1/b1/c1 ... ak/bk/ck"
			for (GLuint k = 0; k < viewerObjects[activeObject].materialInfo[i].faceIndexList[j].size(); ++k)
			{
				// vertRow runs from 0 to numVertices 
				// inherent in the count of these arrays
			
				for (GLuint l = 0; l < 3; ++l)
				{
					if (viewerObjects[activeObject].isNormalThere)
						objVertexNormalData[vertRow][l] = viewerObjects[activeObject].normalPositions[viewerObjects[activeObject].materialInfo[i].faceIndexList[j][k][2] - 1][l];
					objVertexPosData[vertRow][l] = viewerObjects[activeObject].vertexPositions[viewerObjects[activeObject].materialInfo[i].faceIndexList[j][k][0] - 1][l];
					//objVertexKaColData[vertRow][l] = viewerObjects[activeObject].materialInfo[i].Ka[l];
					//objVertexKdColData[vertRow][l] = viewerObjects[activeObject].materialInfo[i].Kd[l];
				}

				vertRow++;
			}

			// For each face j, calculate normal if not available
			if (!viewerObjects[activeObject].isNormalThere)
			{
				normalRow = vertRow - 3;
				vec3 v1 = vec3(objVertexPosData[normalRow][0], 
							   objVertexPosData[normalRow][1], 
							   objVertexPosData[normalRow][2]);
				vec3 v2 = vec3(objVertexPosData[normalRow + 1][0],
							   objVertexPosData[normalRow + 1][1],
							   objVertexPosData[normalRow + 1][2]);

				vec3 v3 = vec3(objVertexPosData[normalRow + 2][0],
							   objVertexPosData[normalRow + 2][1],
							   objVertexPosData[normalRow + 2][2]);

				vec3 normal = normalize(cross((v2 - v1), (v3 - v1)));
				objVertexNormalData[normalRow][0] = normal.x;
				objVertexNormalData[normalRow][1] = normal.y;
				objVertexNormalData[normalRow][2] = normal.z;
				objVertexNormalData[normalRow + 1][0] = normal.x;
				objVertexNormalData[normalRow + 1][1] = normal.y;
				objVertexNormalData[normalRow + 1][2] = normal.z;
				objVertexNormalData[normalRow + 2][0] = normal.x;
				objVertexNormalData[normalRow + 2][1] = normal.y;
				objVertexNormalData[normalRow + 2][2] = normal.z;

			}
		}
	}
}

void scaleUp(int objIndex)
{
	for(GLuint i = 0; i < viewerObjects[objIndex].vertexPositions.size(); ++i)
	{
		
		float x = viewerObjects[objIndex].vertexPositions[i][0];
		float y = viewerObjects[objIndex].vertexPositions[i][1];
		float z = viewerObjects[objIndex].vertexPositions[i][2];
		glm::mat4 scaleM = glm::scale(mat4(1.0f), vec3(sx, sy, sz));
		glm::vec4 tmpVec(x, y, z, 0.0f);
		glm::vec4 tVec = scaleM * tmpVec; 
		viewerObjects[objIndex].vertexPositions[i][0] = tVec.x;
		viewerObjects[objIndex].vertexPositions[i][1] = tVec.y;
		viewerObjects[objIndex].vertexPositions[i][2] = tVec.z;
	}

	if(viewerObjects[objIndex].isNormalThere)
	{
		for(GLuint i = 0; i < viewerObjects[objIndex].vertexPositions.size(); ++i)
		{
		
			float x = viewerObjects[objIndex].normalPositions[i][0];
			float y = viewerObjects[objIndex].normalPositions[i][1];
			float z = viewerObjects[objIndex].normalPositions[i][2];
			glm::mat4 scaleM = glm::scale(mat4(1.0f), vec3(sx, sy, sz));
			glm::vec4 tmpVec(x, y, z, 0.0f);
			glm::vec4 tVec = scaleM * tmpVec; 
			viewerObjects[objIndex].vertexPositions[i][0] = tVec.x;
			viewerObjects[objIndex].vertexPositions[i][1] = tVec.y;
			viewerObjects[objIndex].vertexPositions[i][2] = tVec.z;
		}
	}
}

void translateObj(int objIndex)
{
	for(GLuint i = 0; i < viewerObjects[objIndex].vertexPositions.size(); ++i)
	{
		
		viewerObjects[objIndex].vertexPositions[i][0] += tx;
		viewerObjects[objIndex].vertexPositions[i][1] += ty;
		viewerObjects[objIndex].vertexPositions[i][2] += tz;
		
	}

	/*if(viewerObjects[objIndex].isNormalThere)
	{
		for(GLuint i = 0; i < viewerObjects[objIndex].vertexPositions.size(); ++i)
		{
		
			viewerObjects[objIndex].normalPositions[i][0] += tx;
			viewerObjects[objIndex].normalPositions[i][1] += ty;
			viewerObjects[objIndex].normalPositions[i][2] += tz;
		}
	}*/
}

void rotateObj(int objIndex)
{
	
		

		glm::mat4 rotM = glm::rotate(mat4(1.0f), radians(rx), vec3(1, 0, 0));

		glm::mat4 rot2M = glm::rotate(mat4(1.0f), radians(ry), vec3(0, 1, 0));

		glm::mat4 rot3M = glm::rotate(mat4(1.0f), radians(rz), vec3(0, 0, 1));

		ctlTransform = rot3M * rot2M * rotM;
}


void resetState(int objIndex)
{
	// Allocate and fill vertex pos, col arrays
	// 3 coord for each index
	GLuint numVertices, columns;
	getNumFaces(objIndex, numVertices, columns);
	objVertexPosData = allocateArray(numVertices, columns);
	objVertexNormalData = allocateArray(numVertices, columns);
	//objVertexKaColData = allocateArray(numVertices, columns);
	//objVertexKdColData = allocateArray(numVertices, columns);

	loadVertexPosColData(objIndex);

	viewerObjects[objIndex].numVertices = numVertices;


	// Fill the buffers for buffers[objIndex]
	init(objIndex, numVertices, columns);

	xCam = cameraCoord.x;
	yCam = cameraCoord.y;
	zCam = cameraCoord.z;

	xFocal = focalCoord.x;
	yFocal = focalCoord.y;
	zFocal = focalCoord.z;

	Near = 0.1f; Far = 500.f;
	
	trans = mat4(1.f);
	up = vec3(viewCoord);
	free(objVertexPosData);
	free(objVertexNormalData);
	//free(objVertexKaColData);
	//free(objVertexKdColData);
}

string getPath(string fullPath)
{
	int i;
	for(i = strlen(fullPath.c_str()); i >= 0; i--)
		if(fullPath[i] == '/')
			break;

	return fullPath.substr(0, i+1);
}

void setUniformLights()
{
	for(int i = 0; i < numLights; ++i)
	{
		std::string builder = lightUniformVariable + "[" + to_string(i) + "].";
		for(int j = _LS_IS_ENABLED; j < _NUM_LS_PROPS; ++j)
		{
			string component = builder + lightShaderProp[j];
			switch(j)
			{
			case _LS_IS_ENABLED: 
								glUniform1i(glGetUniformLocation(programID,
											 component.c_str()),
											 Lights[i].isEnabled);
								 break;
			case _LS_IS_LOCAL : glUniform1i(glGetUniformLocation(programID,
					 	 	 	 	 	 	 component.c_str()),
					 	 	 	 	 	 	 Lights[i].isLocal);
								 break;
			case _LS_IS_SPOT  : glUniform1i(glGetUniformLocation(programID,
					 	 	 	 	 	 	 component.c_str()),
					 	 	 	 	 	 	 Lights[i].isSpot);
								break;
			case _LS_AMBIENT : glUniform3f(glGetUniformLocation(programID,
 	 	 	 	 	 	 	 	 	 	 	 component.c_str()),
 	 	 	 	 	 	 	 	 	 	 	 Lights[i].ambient.r,
											 Lights[i].ambient.g,
											 Lights[i].ambient.b);
								break;
			case _LS_COLOR : glUniform3f(glGetUniformLocation(programID,
			 	 	 	 	 	 	 	 component.c_str()),
			 	 	 	 	 	 	 	  Lights[i].color.r,
										  Lights[i].color.g,
										  Lights[i].color.b);
								break;
			case _LS_POS :  glUniform3f(glGetUniformLocation(programID,
			 	 	 	 	 	 	 	 component.c_str()),
			 	 	 	 	 	 	 	 Lights[i].position.r,
										 Lights[i].position.g,
										 Lights[i].position.b);
							break;
			case _LS_CONE_DIR :  glUniform3f(glGetUniformLocation(programID,
						 	 	 	 	 	 	 	 component.c_str()),
						 	 	 	 	 	 	 	 Lights[i].coneDirection.r,
													 Lights[i].coneDirection.g,
													 Lights[i].coneDirection.b);
										break;
			case _LS_SPOT_COS_CUT_OFF  : glUniform1f(glGetUniformLocation(programID,
								 	 	 	 component.c_str()),
								 	 	 	 Lights[i].spotCosCutoff);
										 break;
			case _LS_SPOT_EXP  : glUniform1f(glGetUniformLocation(programID,
								 	 	 	 component.c_str()),
								 	 	 	 Lights[i].spotExponent);
								  break;
			case _LS_CONST_ATT  : glUniform1f(glGetUniformLocation(programID,
								 	 	 	  component.c_str()),
								 	 	 	  Lights[i].constantAttenuation);
								  break;
			case _LS_LINEAR_ATT  : glUniform1f(glGetUniformLocation(programID,
								 	 	 	 	component.c_str()),
								 	 	 	 	Lights[i].linearAttenuation);
								   break;

			case _LS_QUAD_ATT  : glUniform1f(glGetUniformLocation(programID,
											 component.c_str()),
											 Lights[i].quadraticAttenuation);
								 break;

			}
		}

	}
}

void handleLights(std::vector<string >& quanta, GLuint curLight)
{
	Lights[curLight].isEnabled = GL_TRUE;

	GLuint lightType = getCommand(quanta[1], _META_LIGHT_TYPES);

	// Set up ambient, color and position
	GLuint attrType;
	for(int i = 2; i < 14 ; i +=4)
	{
		string tmpAttr = quanta[i];
		attrType = getCommand(tmpAttr, _META_LIGHT_ATTR);
		switch(attrType)
		{
		case _LA_AMBIENT:	Lights[curLight].ambient.r = atof(quanta[i+1].c_str());
							Lights[curLight].ambient.g = atof(quanta[i+2].c_str());
							Lights[curLight].ambient.b = atof(quanta[i+3].c_str());
							break;
		case _LA_COLOR:   	Lights[curLight].color.r = atof(quanta[i+1].c_str());
							Lights[curLight].color.g = atof(quanta[i+2].c_str());
							Lights[curLight].color.b = atof(quanta[i+3].c_str());
							break;
		case _LA_POSITION: 	Lights[curLight].position.r = atof(quanta[i+1].c_str());
							Lights[curLight].position.g = atof(quanta[i+2].c_str());
							Lights[curLight].position.b = atof(quanta[i+3].c_str());
							break;
		}
	}

	switch(lightType)
	{
	case _LOCAL: case _SPOT:
					Lights[curLight].isLocal = GL_TRUE;
					if(lightType == _SPOT)
						Lights[curLight].isSpot = GL_TRUE;
					for(int i = 14; i < 20; i+=2)
					{
						string tmpAttr = quanta[i];
						attrType = getCommand(tmpAttr, _META_LIGHT_ATTR);
						switch(attrType)
						{
						case _LA_CONST_ATT:	Lights[curLight].constantAttenuation = atof(quanta[i+1].c_str());
							 	 	 	 	break;
						case _LA_LINEAR_ATT:	Lights[curLight].linearAttenuation = atof(quanta[i+1].c_str());
												break;
						case _LA_QUAD_ATT:	Lights[curLight].quadraticAttenuation = atof(quanta[i+1].c_str());
											break;
						}
					}
					if(lightType == _SPOT)
					{
						for(int i = 20; i < 28; i+=2)
						{
							string tmpAttr = quanta[i];
							attrType = getCommand(tmpAttr, _META_LIGHT_ATTR);
							switch(attrType)
							{
								case _LA_CONE_DIR:	Lights[curLight].coneDirection.x = atof(quanta[i+1].c_str());
													Lights[curLight].coneDirection.y = atof(quanta[i+2].c_str());
													Lights[curLight].coneDirection.z = atof(quanta[i+3].c_str());
													i += 2;
													break;
								case _LA_SPOT_COS_CUT_OFF:	Lights[curLight].spotCosCutoff = atof(quanta[i+1].c_str());
														break;
								case _LA_SPOT_EXP:	Lights[curLight].spotExponent = atof(quanta[i+1].c_str());
														break;
							}
						}
					}

					break;
	case _DIRECTIONAL: Lights[curLight].isLocal = Lights[curLight].isSpot = GL_FALSE; break;
	}

}

void handleView(std::vector<string >& viewTokens)
{
	for(int i = 1; i < viewTokens.size(); i += 4)
	{
		GLuint cmd = getCommand(viewTokens[i], _META_VIEW_ATTR);
		switch(cmd)
		{
		case _VA_CAMERA : cameraCoord.x = atof(viewTokens[i+1].c_str());
						  cameraCoord.y = atof(viewTokens[i+2].c_str());
						  cameraCoord.z = atof(viewTokens[i+3].c_str());
						  break;
		case _VA_FOCAL : focalCoord.x = atof(viewTokens[i+1].c_str());
		  	  	  	  	 focalCoord.y = atof(viewTokens[i+2].c_str());
						 focalCoord.z = atof(viewTokens[i+3].c_str());
						 break;
		case _VA_VIEWUP : viewCoord.x = atof(viewTokens[i+1].c_str());
		  	  	  	  	  viewCoord.y = atof(viewTokens[i+2].c_str());
		  	  	  	  	  viewCoord.z = atof(viewTokens[i+3].c_str());
		  	  	  	  	  break;
		}
	}

	xCam = cameraCoord.x;
	yCam = cameraCoord.y;
	zCam = cameraCoord.z;

	xFocal = focalCoord.x;
	yFocal = focalCoord.y;
	zFocal = focalCoord.z;

	up = vec3(viewCoord);
}

void parseMaterialFile(int objIndex, string filename)
{
	ifstream materialFile;
	string mtlLine;
	std::vector< string > mtlLines;
	GLuint mtlCmd, count = 0;
	MaterialData temp;
	size_t len = 0;
    ssize_t read;

	string tempS = filename.substr(0, filename.length() - 1);	

	materialFile.open(filename.c_str());	
	if (materialFile.is_open())
	{
		 while (getline(materialFile, mtlLine)) {
	          mtlLines.push_back(mtlLine);
       		}

		materialFile.close();
	}
	else
	{
		cout << "\nInvalid material filename. Re-start viewer with valid "
			"material filename" << endl;
		exit(EXIT_FAILURE);
	}

	for (GLuint i = 0; i < mtlLines.size(); ++i)
	{
		std::vector< string > mtlTokens;
		string mtlName;

		tokenizeGeneral(mtlLines[i], mtlTokens);
		
		if (!mtlTokens.empty())
		{
			mtlCmd = _MTL_INVALID;
			mtlCmd = getCommand(mtlTokens[0], _META_MTL_CMD);

			switch (mtlCmd)
			{
			case _MTL_NEWMTL :
				mtlName = mtlTokens[1];
				count = 0;
				temp.materialName = mtlName;//.substr(0, mtlName.size()-1);
				break;
			case _KA: case _KD: case _KS: case _NS:
				count++;
				for (GLuint j = 1; j < mtlTokens.size(); ++j)
				{
					switch (mtlCmd)
					{
					case _KA: temp.Ka[j - 1] = atof(mtlTokens[j].c_str()); 
							  if(!setKa && j == mtlTokens.size()-1)
							  {
								  Ka.x = temp.Ka[0];
								  Ka.y = temp.Ka[1];
								  Ka.z = temp.Ka[2];
								  setKa = GL_TRUE;
							  }
							  break;
					case _KD: temp.Kd[j - 1] = atof(mtlTokens[j].c_str());
								if(!setKd  && j == mtlTokens.size()-1)
								{
									Kd.x = temp.Kd[0];
									Kd.y = temp.Kd[1];
									Kd.z = temp.Kd[2];
									setKd = GL_TRUE;
								}
							  break;
					case _KS: temp.Ks[j - 1] = atof(mtlTokens[j].c_str());
								if(!setKs  && j == mtlTokens.size()-1)
								{
									Ks.x = temp.Ks[0];
									Ks.y = temp.Ks[1];
									Ks.z = temp.Ks[2];
									setKs = GL_TRUE;
								}
							  break;
					case _NS: temp.Ns = atof(mtlTokens[j].c_str());
							  if(!setNs  && j == mtlTokens.size()-1)
							  {
								  Ns = temp.Ns;
								  setNs = GL_TRUE;
							  }
					}
				}

				if (count == 4)
					viewerObjects[objIndex].materialInfo.push_back(temp);
				
				break;
			}
		}
	}
}

void readObjFile(string objFileName, int objIndex)
{
	ifstream objFile;
	string line, mtlName;
	std::vector< string > lines;
	std::vector< std::vector <string > > tokenList;
	ViewerObject tempObj;
	tempObj.isNormalThere = GL_FALSE;
	GLboolean isMaterialThere = GL_FALSE, flag = GL_FALSE;

	GLuint i = 0, j, k = 0;
	GLuint cmdType;
	objFile.open(objFileName.c_str());

	if (objFile.is_open())
	{
		while (getline(objFile, line))
			lines.push_back(line);

		objFile.close();
	}
	else
	{
		cout << "\nInvalid Obj filename. Re-start viewer with valid "
			"Obj filename" << endl;
		exit(EXIT_FAILURE);
	}

	// Set objName in tempObj
	tempObj.objName = objFileName.substr(0, objFileName.length() - 4);
	tempObj.objName += std::to_string(objIndex);

	tempObj.objPath = getPath(tempObj.objName);
	viewerObjects.push_back(tempObj);
	
	// Load the vertex array/buffer

	for (i = 0; i < lines.size(); ++i)
	{
		std::vector<string > tokens;
		std::vector<std::vector <GLuint > > faceIndexTokens;
		std::vector<GLfloat > vTemp;
		std::vector<GLuint > fTemp;
		
		/* Strictly for face values */
		std::vector<char> buf;
		vector<string> indexTokens;
		vector<GLuint > indices;
		char* tmp;
		tokenizeGeneral(lines[i], tokens);
		if (!tokens.empty())
		{
			cmdType = _INVALID;
			cmdType = getCommand(tokens[0], _META_OBJ_CMD);

			switch (cmdType)
			{	
			case _V: case _VT: case _VN:
				
				for (j = 1; j < tokens.size(); ++j)
					vTemp.push_back(atof(tokens[j].c_str()));
				
				switch (cmdType)
				{
				case _V : viewerObjects[objIndex].vertexPositions.push_back(vTemp); break;
				case _VT:     break;
				case _VN: viewerObjects[objIndex].isNormalThere = GL_TRUE; 
						  viewerObjects[objIndex].normalPositions.push_back(vTemp); break;
				}
				break;
			case _F:

				if (!isMaterialThere)
				{
					addDummyMaterial(objIndex);
					activeMaterialIndex = getMaterialIndex(objIndex, "default");
					isMaterialThere = GL_TRUE;
				}
				GLchar *bufSpaceTokens;
				for (GLuint j = 0; j < lines[i].size(); ++j)
				{
					buf.push_back(lines[i][j]);
					if (lines[i][j] == '/')
						if (lines[i][j + 1] == '/')
							buf.push_back('0');

				}
				tmp = new GLchar[buf.size()+1];
				for (GLuint j = 0; j < buf.size(); ++j)
					tmp[j] = buf[j];
				tmp[buf.size()] = '\0';
				tokenizeGeneral(tmp, indexTokens);
				delete[] tmp;
				tokenizeFaceIndices(indexTokens, faceIndexTokens);
				viewerObjects[objIndex].materialInfo[activeMaterialIndex].faceIndexList.push_back(faceIndexTokens);
				
				break;
			case _MTLLIB: 
				materialFileName = viewerObjects[objIndex].objPath + tokens[1];
				flag = isMaterialFilePresent(objIndex, materialFileName);
				if(!flag)
				{
					viewerObjects[objIndex].materialFileList.push_back(materialFileName); 
					parseMaterialFile(objIndex, materialFileName);
				}
				break;
			case _USEMTL:
				mtlName = tokens[1];//.substr(0, tokens[1].size()-1);
				isMaterialThere = GL_TRUE;
				activeMaterialIndex = getMaterialIndex(objIndex, mtlName);
				break;
			}
		}
	}

	// Normals aren't defined
	//if (!tempObj.isNormalThere)
			
}

void readControlFile(string controlFileName)
{
	fstream controlFile;
	string line, objFileName;
	vector<string > ctlLines;
	GLuint i = 0, j, k = 0;
	GLuint cmdType;
	controlFile.open(controlFileName.c_str());
	int currentObj = -1;

	if (controlFile.is_open())
	{
		while (getline(controlFile, line))
			ctlLines.push_back(line);

		controlFile.close();
	}
	else
	{
		cout << "\nInvalid control filename. Re-start viewer with valid "
			"control filename" << endl;
		exit(EXIT_FAILURE);
	}


	// Count nummber of Objects
	for (i = 0; i < ctlLines.size(); ++i)
	{
		vector<string > countTokens;
		tokenizeGeneral(ctlLines[i], countTokens);
		if (!countTokens.empty())
			if (countTokens[0].compare(controls[_OBJ]) == 0)
				numObj++;
	}

	// Instantiate VAOs and Buffers

	/* GL related config */
	VAOs = new GLuint[numObj];
	objBuffers = new GLuint*[numObj];

	glGenVertexArrays(numObj, VAOs);

	/*****************************/

	for (i = 0; i < ctlLines.size(); ++i)
	{
		vector<string > ctlTokens;
		tokenizeGeneral(ctlLines[i], ctlTokens);
		if (!ctlTokens.empty())
		{
			cmdType = _CTL_INVALID;
			cmdType = getCommand(ctlTokens[0], _META_CONTROL);

			switch (cmdType)
			{
			case _OBJ:	objFileName = ctlTokens[1];
						currentObj++;
						readObjFile(objFileName, currentObj);
						resetState(currentObj);
						break;
			case _RX:  rx = atof(ctlTokens[1].c_str()); 
			  	   	   ry = ctlTokens.size() >= 4? atof(ctlTokens[3].c_str()) : 0; 
		    		   rz = ctlTokens.size() >= 6? atof(ctlTokens[5].c_str()) : 0;
					   rotateObj(currentObj); 
					   resetState(currentObj);
		    		   break;
			case _T:  	 tx = atof(ctlTokens[1].c_str());
					 	 ty = atof(ctlTokens[2].c_str());
					 	 tz = atof(ctlTokens[3].c_str());
						 translateObj(currentObj);
						 resetState(currentObj);
						 break;
			case _CTL_S: sx = atof(ctlTokens[1].c_str());
					 	 sy = atof(ctlTokens[2].c_str());
					 	 sz = atof(ctlTokens[3].c_str());
						 scaleUp(currentObj);
						 
						 resetState(currentObj);
						 break;
			case _LIGHT:
						 curLight++; numLights++;
						 handleLights(ctlTokens, curLight); break;
			case _VIEW:
						 handleView(ctlTokens);
						 break;
			}
		}
	}
}

void resize(int curH, int curW)
{
	GLfloat originalRatio = (float) windowWidth / (float) windowHeight;
    GLfloat newRatio = (float) curW / (float) curH;

    GLfloat scaleW = (float) curW / (float) windowWidth;
    GLfloat scaleH = (float) curH / (float) windowHeight;
    if (newRatio > originalRatio) 
        scaleW = scaleH;
    	else scaleH = scaleW;

    GLfloat Xmargin = (curW - windowWidth * scaleW) / 2;
    GLfloat Ymargin = (curH - windowHeight * scaleH) / 2;

    glViewport(Xmargin, Ymargin, windowWidth * scaleW, windowHeight * scaleH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth / originalRatio, 0, windowHeight / originalRatio, 0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
	
}


/////////////////////////////////////////////////////
//  Keyboard callback
/////////////////////////////////////////////////////
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	
	case 'w': case 'W':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.5);
		/*for (GLuint i = 0; i < viewerObjects.size(); ++i)
			init(i, viewerObjects[i].numVertices, 3);*/
		glutPostRedisplay();
		break;
	case 's': case 'S':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		/*for (GLuint i = 0; i < viewerObjects.size(); ++i)
			init(i, viewerObjects[i].numVertices, 3);*/
		glutPostRedisplay();
		break;
	case 'c': case 'C': zCam = zCam - 0.1f; glutPostRedisplay(); break;
	case 'v': case 'V': zCam = zCam + 0.1f; glutPostRedisplay(); break;
	case 'd': case 'D': zFocal = zFocal - 0.1f; glutPostRedisplay(); break;
	case 'f': case 'F': zFocal = zFocal + 0.1f; glutPostRedisplay(); break;
	case 'r': case 'R': for(int i = 0; i < viewerObjects.size(); ++i) 
							resetState(i); 
						glutPostRedisplay();
						break;
	case 'x': case 'X': 
						rot = glm::rotate(rot, glm::radians(-1.0f), camDir); 
						up = mat3(rot) * up; 
						glutPostRedisplay();
						break;
	case 'z': case 'Z': rot = glm::rotate(rot, glm::radians(1.0f), camDir); 
						up = mat3(rot)* up; glutPostRedisplay();
						break;
	case 'q': case 'Q': case 27:
		exit(EXIT_SUCCESS);
		break;
	
	default:
		break;
	}
}

void arrowKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:	
		trans = glm::rotate(trans, radians(1.0f), vec3(0, 0, 1));
		
		//setupMVP();
		glutPostRedisplay();
						break;
	case GLUT_KEY_RIGHT:
		trans = glm::rotate(trans, radians(-1.0f), vec3(0, 0, 1));
		
		//setupMVP();
		glutPostRedisplay();
						break;
	case GLUT_KEY_UP: xCam -= camDir.x;
					  yCam -= camDir.y;
					  zCam -= camDir.z;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN: xCam += camDir.x;
					    yCam += camDir.y;
						zCam += camDir.z;
		glutPostRedisplay();
		break;
	}
}

void init(GLuint activeObjectIndex, GLuint numVertices, GLuint columns)
{
	ShaderInfo  shaders[] = {
		{ GL_VERTEX_SHADER, "classic.vert" },
		{ GL_FRAGMENT_SHADER, "classic.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);
	programID = program;

	glBindVertexArray(VAOs[activeObjectIndex]);

	objBuffers[activeObjectIndex] = new GLuint[numBuffers];
	glGenBuffers(numBuffers, objBuffers[activeObjectIndex]);
	
	// Position
	glBindBuffer(GL_ARRAY_BUFFER, objBuffers[activeObjectIndex][_POS]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * columns * sizeof(GLfloat), 
				 objVertexPosData[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	// Normal values
	glBindBuffer(GL_ARRAY_BUFFER, objBuffers[activeObjectIndex][_NORM_POS]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * columns * sizeof(GLfloat),
		objVertexNormalData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(normPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(normPosition);

	// Ka values
	/*glBindBuffer(GL_ARRAY_BUFFER, objBuffers[activeObjectIndex][_BUF_KA]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * columns * sizeof(GLfloat),
		objVertexKaColData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(cKaPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(cKaPosition);*/

	// Kd values
	/*glBindBuffer(GL_ARRAY_BUFFER, objBuffers[activeObjectIndex][_BUF_KD]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * columns * sizeof(GLfloat),
		objVertexKdColData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(cKdPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(cKdPosition);*/

	
}

void display()
{
	// Vertices, Normals and Colors are sent to shader.
	// Set up MVP now

	setupMVP();
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVPMatrix"), 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVMatrix"), 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(programID, "NormalMatrix"), 1, GL_FALSE, &Normal[0][0]);

	setUniformLights();

	glUniform3f(glGetUniformLocation(programID, "ambient"), Ka.x, Ka.y, Ka.z);
	glUniform3f(glGetUniformLocation(programID, "diffuse"), Kd.x, Kd.y, Kd.z);
	glUniform3f(glGetUniformLocation(programID, "specular"), Ks.x, Ks.y, Ks.z);
	glUniform1f(glGetUniformLocation(programID, "shininess"), Ns);

	glClearColor(0, 0.15, 0.2, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	for (GLuint i = 0; i < viewerObjects.size(); ++i)
	{
		glBindVertexArray(VAOs[i]);
		glDrawArrays(GL_TRIANGLES, 0, viewerObjects[i].numVertices);
	}
	glFlush();
}

////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowHeight, windowWidth);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);// GLUT_COMPATIBILITY_PROFILE );
	
	glutCreateWindow(argv[0]);

	glewExperimental = GL_TRUE;	// added for glew to work!
	if (glewInit())
	{
		cerr << "Unable to initialize GLEW ... exiting" << endl;
		exit(EXIT_FAILURE);
	}

	// code from OpenGL 4 Shading Language cookbook, second edition
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	cout << "GL Vendor            :" << vendor << endl;
	cout << "GL Renderer          :" << renderer << endl;
	cout << "GL Version (string)  :" << version << endl;
	cout << "GL Version (integer) :" << major << " " << minor << endl;
	cout << "GLSL Version         :" << glslVersion << endl;

	GLuint flag = 0;
	string controlFileName;

	if (argc != 3)
		flag = 10;
	else
	{
		if (strcmp(argv[1], "-c") != 0)
			flag = 10;
		else controlFileName = argv[2];
	}

	if (flag)
	{
		cout << "Usage ::  viewer -c <controlFileName>" << endl;
		exit(EXIT_FAILURE);
	}

	//
	/*GLint nExtensions;
	glGetIntegerv( GL_NUM_EXTENSIONS, &nExtensions );
	for ( int i = 0; i < nExtensions; i++ )
		cout << glGetStringi( GL_EXTENSIONS, i )  << endl;*/

	setCommand();
	readControlFile(controlFileName);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrowKeys);
	glutReshapeFunc(resize);

	glutMainLoop();

	return 0;
}
