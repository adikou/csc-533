// ch01.cpp : Defines the entry point for the console application.
//
//////////////////////////////////////////////////////////
//  triangles.cpp  from the OpenGL Red Book   Chapter 1
//////////////////////////////////////////////////////////

#include "Libraries\glew\glew.h"
#include "Libraries\freeglut\freeglut.h"

#include <gl\GL.h>
#include <iostream>

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"

//enum VAO_IDs { Triangles, NumVAOs };
const GLuint Triangles = 0, NumVAOs = 1;
//enum Buffer_IDs { ArrayBuffer, NumBuffers };
const GLuint ArrayBuffer = 0, colorBuffer = 1, NumBuffers = 2;
//enum Attrib_IDs { vPosition = 0 };
const GLuint vPosition = 0, cPosition = 1;

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices = 9;
GLuint toggle[3] = { 1, 1, 0 };

/////////////////////////////////////////////////////
//  int
/////////////////////////////////////////////////////
void init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Triangles]);

	GLfloat vertices[NumVertices][2] = {
		{ -0.90f, -0.9f },	// Triangle 1
		{ 0.85f, -0.9f },
		{ -0.90f, 0.85f },
		{ 0.90f, -0.85f },	// Triangle 2
		{ 0.90f, 0.90f },
		{ -0.85f, 0.90f },
		{ 0.0f, 0.5f},		// Center triangle
		{ -0.5f, -0.5f},
		{ 0.50f, -0.5f}
	};
	
	GLfloat colors[NumVertices][3] = {
		{ 0.0f, 0.0f, 1.0f }, // Triangle 1
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }, // Triangle 2
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f }, // Center triangle
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f }
	};

	glGenBuffers(NumBuffers, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[colorBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	ShaderInfo  shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[colorBuffer]);
	glVertexAttribPointer(cPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(cPosition);
}

////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VAOs[Triangles]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	//glDrawArrays( GL_LINES, 0, NumVertices );

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

	init();
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}

