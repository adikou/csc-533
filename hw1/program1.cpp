// ch01.cpp : Defines the entry point for the console application.
//
//////////////////////////////////////////////////////////
//  triangles.cpp  from the OpenGL Red Book   Chapter 1
//////////////////////////////////////////////////////////

#include "GL/glew.h"
#include "GL/freeglut.h"

#include <GL/gl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"

//enum VAO_IDs { Triangles, NumVAOs };
const GLuint Triangles = 0, colorTriangle = 1, circle = 2, NumVAOs = 3;
//enum Buffer_IDs { ArrayBuffer, NumBuffers };
const GLuint ArrayBuffer = 0, colorBuffer = 1, NumBuffers = 2;
//enum Attrib_IDs { vPosition = 0 };
const GLuint vPosition = 0, cPosition = 1;

const GLuint TWO_TRIANGLES = 0, COLOR_TRIANGLE = 1, CIRCLE = 2;
const GLfloat PI = 3.14159f;

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint trialBuf[NumBuffers];
GLuint circleBuf[NumBuffers];

const GLuint NumVertices = 6;
const GLuint NumTriangles = 2, NumColorTriangle = 1, NumCircle = 1;
GLuint toggle[3] = { 1, 1, 0 };

GLfloat r = 0.0f, g = 0.0f, b = 1.0f;
GLfloat globalTrianglesPositionArray[NumVertices][2];
GLfloat globalTrianglesColorArray[NumVertices][3];

// Single color triangle
GLfloat globalColorTrianglePositionArray[NumColorTriangle * 3][2];
GLfloat globalColorTriangleColorArray[NumColorTriangle * 3][3];

// Circle
GLfloat **globalCirclePositionArray;
GLfloat **globalCircleColorArray;
GLuint numSteps = 10;
GLfloat radius = 0.5;

void init();

/////////////////////////////////////////////////////
//  Keyboard callback
/////////////////////////////////////////////////////
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'c': case 'C':
		fprintf(stdout, "\nEnter the RGB color components :: ");
		cin >> r >> g >> b;
		while ((r < 0 || r > 1) || (g < 0 || g > 1) || (b < 0 || b > 1)
			|| (isalpha((int)r)) || (isalpha((int)g)) || isalpha((int)b))
		{
			fprintf(stdout,"\nErroneous values for RGB. Please enter in range [0,1] :: ");
			cin >> r >> g >> b;
		}

		for (int i = 0; i < NumTriangles * 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				switch (j)
				{
				case 0: globalTrianglesColorArray[i][j] = r; break;
				case 1: globalTrianglesColorArray[i][j] = g; break;
				case 2: globalTrianglesColorArray[i][j] = b; break;
				}
			}
		}
		init();
		glutPostRedisplay();
		break;
	case 'w': case 'W':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.5);
		init();
		glutPostRedisplay();
		break;
	case 's': case 'S':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		init();
		glutPostRedisplay();
		break;
	case 'q': case 'Q': case 27:
		exit(EXIT_SUCCESS);
		break;
	case 'x': case 'X' :
		toggle[TWO_TRIANGLES] = !toggle[TWO_TRIANGLES];
		init();
		glutPostRedisplay();
		break;
	case 'y': case 'Y':
		toggle[COLOR_TRIANGLE] = !toggle[COLOR_TRIANGLE];
		init();
		glutPostRedisplay();
		break;
	case 'z': case 'Z':
		toggle[CIRCLE] = !toggle[CIRCLE];
		init();
		glutPostRedisplay();
		break;
	case 'g': case 'G' :
		fprintf(stdout,"\nInput the radius and number of steps :: ");
		cin >> radius >> numSteps;

		while(radius <=0 || radius >1 || isalpha((int)radius))
		{
		    fprintf(stdout, "\nEnter radius in the range [0,1] only :: ");
		    fprintf(stdout, "\nRe-enter radius and number of steps :: ");
		    cin >> radius >> numSteps;
		}
		toggle[CIRCLE] = 1;
		init();
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

/////////////////////////////////////////////////////
//  drawCircle
/////////////////////////////////////////////////////
GLfloat** allocateArray(GLuint numSteps, GLuint columns)
{
	GLfloat **temp, *vals;
	vals = (GLfloat*)malloc(numSteps * columns * sizeof(GLfloat));
	temp = (GLfloat**)malloc(numSteps * sizeof(GLfloat*));

	for (GLuint i = 0; i < numSteps; ++i)
		temp[i] = &(vals[columns * i]);

	return temp;
}

/////////////////////////////////////////////////////
//  drawCircle
/////////////////////////////////////////////////////

void drawCircle(GLfloat radius, GLuint numSteps)
{
	globalCirclePositionArray = allocateArray(numSteps + 2, 2);
	globalCircleColorArray = allocateArray(numSteps + 2, 3);

	GLuint i; GLfloat twoPI = 2 * PI;
	globalCirclePositionArray[0][0] = 0;
	globalCirclePositionArray[0][1] = 0;
	globalCircleColorArray[0][0] = r;
	globalCircleColorArray[0][1] = g;
	globalCircleColorArray[0][2] = b;
	for (i = 1; i <= numSteps + 1; ++i)
	{
		globalCirclePositionArray[i][0] = radius * cos((i-1) * (twoPI / numSteps));
		globalCirclePositionArray[i][1] = radius * sin((i-1) * (twoPI / numSteps));

		globalCircleColorArray[i][0] = r;
		globalCircleColorArray[i][1] = g;
		globalCircleColorArray[i][2] = b;
	}

	glBindVertexArray(VAOs[circle]);

	glGenBuffers(NumBuffers, circleBuf);
	glBindBuffer(GL_ARRAY_BUFFER, circleBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, (numSteps + 2) * 2 * sizeof(GLfloat), 
				 globalCirclePositionArray[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, circleBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, (numSteps + 2) * 3 * sizeof(GLfloat), 
				 globalCircleColorArray[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, circleBuf[0]);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, circleBuf[1]);
	glVertexAttribPointer(cPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(cPosition);
}

/////////////////////////////////////////////////////
//  init
/////////////////////////////////////////////////////
void init()
{
	glGenVertexArrays(NumVAOs, VAOs);
	ShaderInfo  shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);

	if (toggle[TWO_TRIANGLES] == 1)
	{
		glBindVertexArray(VAOs[Triangles]);

		GLfloat vertices[NumVertices][2], colors[NumVertices][3];

		// Vertex positions
		for (int i = 0; i < NumVertices; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				vertices[i][j] = globalTrianglesPositionArray[i][j];
				colors[i][j] = globalTrianglesColorArray[i][j];
			}

			colors[i][2] = globalTrianglesColorArray[i][2];
		}

		glGenBuffers(NumBuffers, Buffers);
		glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, Buffers[colorBuffer]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(vPosition);

		glBindBuffer(GL_ARRAY_BUFFER, Buffers[colorBuffer]);
		glVertexAttribPointer(cPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(cPosition);
	}
	/*  Trials  */

	if (toggle[COLOR_TRIANGLE] == 1)
	{
		glBindVertexArray(VAOs[colorTriangle]);

		GLfloat colorTrianglePos[3][2], colorTriangleCol[3][3];

		// Vertex positions
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				colorTrianglePos[i][j] = globalColorTrianglePositionArray[i][j];
				colorTriangleCol[i][j] = globalColorTriangleColorArray[i][j];
			}

			colorTriangleCol[i][2] = globalColorTriangleColorArray[i][2];
		}

		glGenBuffers(NumBuffers, trialBuf);
		glBindBuffer(GL_ARRAY_BUFFER, trialBuf[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorTrianglePos),
			colorTrianglePos, GL_STATIC_DRAW);

		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(vPosition);

		glBindBuffer(GL_ARRAY_BUFFER, trialBuf[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorTriangleCol),
			colorTriangleCol, GL_STATIC_DRAW);

		glVertexAttribPointer(cPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(cPosition);
	}

	if (toggle[CIRCLE] == 1)
	{
		drawCircle(radius, numSteps);
	}
}

////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (toggle[TWO_TRIANGLES] == 1)
	{
		glBindVertexArray(VAOs[Triangles]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}
	//glDrawArrays( GL_LINES, 0, NumVertices );
	if (toggle[COLOR_TRIANGLE] == 1)
	{
		glBindVertexArray(VAOs[colorTriangle]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	if (toggle[CIRCLE] == 1)
	{
		glBindVertexArray(VAOs[circle]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numSteps+2);
	}

	glFlush();

}

////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(4, 3);
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
	//
	//GLint nExtensions;
	//glGetIntegerv( GL_NUM_EXTENSIONS, &nExtensions );
	//for ( int i = 0; i < nExtensions; i++ )
	//	cout << glGetStringi( GL_EXTENSIONS, i )  << endl;

	// Initial values of the triangle color and position
	GLfloat initTrianglesVertices[NumVertices][2] = {
		{ -0.90f, -0.9f },	// Triangle 1
		{ 0.85f, -0.9f },
		{ -0.90f, 0.85f },
		{ 0.90f, -0.85f },	// Triangle 2
		{ 0.90f, 0.90f },
		{ -0.85f, 0.90f }
	};

	GLfloat initTrianglesColors[NumVertices][3] = {
		{ 0.0f, 0.0f, 1.0f }, // Triangle 1
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }, // Triangle 2
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
	};

	for (int i = 0; i < NumVertices; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			globalTrianglesPositionArray[i][j] = initTrianglesVertices[i][j];
			globalTrianglesColorArray[i][j] = initTrianglesColors[i][j];
		}
		globalTrianglesColorArray[i][2] = initTrianglesColors[i][2];
	}

	GLfloat tmpVertices[3][2] = {
		{ 0.0f, 0.6f },		// Center triangle
		{ -0.6f, -0.6f },
		{ 0.60f, -0.6f }
	};

	GLfloat tmpColors[3][3] = {
		{ 1.0f, 0.0f, 0.0f }, // Center triangle
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f }
	};

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			globalColorTrianglePositionArray[i][j] = tmpVertices[i][j];
			globalColorTriangleColorArray[i][j] = tmpColors[i][j];
		}
		globalColorTriangleColorArray[i][2] = tmpColors[i][2];
	}

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}

