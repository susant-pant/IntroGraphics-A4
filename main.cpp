// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include "glm/glm.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define PI 3.14159265358976

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

GLFWwindow* window = 0;

vector<vec3> triangleVecs;
vector<vec3> sphereVecs;
vector<vec3> planeVecs;
vector<vec3> lightVecs;

vector<vec2> points;
vector<vec2> uvs;

struct VAO{
	enum {LINES=0, COUNT};		//Enumeration assigns each name a value going up
};

struct VBO{
	enum {POINTS=0, COLOR, COUNT};	//POINTS=0, COLOR=1, COUNT=2
};

struct SHADER{
	enum {LINE=0, COUNT};		//LINE=0, COUNT=1
};

GLuint vbo [VBO::COUNT];		//Array which stores OpenGL's vertex buffer object handles
GLuint vao [VAO::COUNT];		//Array which stores Vertex Array Object handles
GLuint shader [SHADER::COUNT];		//Array which stores shader program handles

//Gets handles from OpenGL
void generateIDs()
{
	glGenVertexArrays(VAO::COUNT, vao);
	glGenBuffers(VBO::COUNT, vbo);
}

//Clean up IDs when you're done using them
void deleteIDs()
{
	for(int i=0; i<SHADER::COUNT; i++)
	{
		glDeleteProgram(shader[i]);
	}
	
	glDeleteVertexArrays(VAO::COUNT, vao);
	glDeleteBuffers(VBO::COUNT, vbo);	
}

//Describe the setup of the Vertex Array Object
bool initVAO()
{
	glBindVertexArray(vao[VAO::LINES]);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo[VBO::POINTS] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		2,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec2),	//Stride
		(void*)0			//Offset
		);
	
	glEnableVertexAttribArray(1);		//Tell opengl you're using layout attribute 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0
		);	

	return !CheckGLErrors("initVAO");		//Check for errors in initialize
}

//Loads buffers with data
bool loadBuffer(const vector<vec2>& points, const vector<vec2>& colors)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::POINTS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec2)*points.size(),	//Size of data in array (in bytes)
		&points[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec2)*uvs.size(),
		&uvs[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors("loadBuffer");	
}

//Compile and link shaders, storing the program ID in shader array
bool initShader()
{	
	string vertexSource = LoadSource("vertex.glsl");		//Put vertex file text into string
	string fragmentSource = LoadSource("fragment.glsl");		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	shader[SHADER::LINE] = LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array

	return !CheckGLErrors("initShader");
}

//For reference:
//	https://open.gl/textures
GLuint createTexture(const char* filename)
{
	int components;
	GLuint texID;
	int tWidth, tHeight;

	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename, &tWidth, &tHeight, &components, 0);
	
	if(data != NULL)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		if(components==3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if(components==4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Clean up
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		return texID;
	} 
	
	return 0;	//Error
}

void generateSquare(float width)
{
	vec2 p00 = vec2(-width*0.5f, width*0.5f);
	vec2 uv00 = vec2(0.f, 0.f);

	vec2 p01 = vec2(width*0.5f, width*0.5f);
	vec2 uv01 = vec2(1.f, 0.f);

	vec2 p10 = vec2(-width*0.5f, -width*0.5f);
	vec2 uv10 = vec2(0.f, 1.f);
	
	vec2 p11 = vec2(width*0.5f, -width*0.5f);
	vec2 uv11 = vec2(1.f, 1.f);

	//Triangle 1
	points.push_back(p00); points.push_back(p10); points.push_back(p01);
	uvs.push_back(uv00); uvs.push_back(uv10); uvs.push_back(uv01);

	//Triangle 2
	points.push_back(p11); points.push_back(p01); points.push_back(p10);
	uvs.push_back(uv11);uvs.push_back(uv01); uvs.push_back(uv10);
}

float xPos = 0.f;
float yPos = 0.f;
float zPos = 0.f;
float lookUp = 0.f;
float lookRight = 0.f;
bool focus = false;
bool scene3 = false;

bool loadUniforms()
{
	glUseProgram(shader[SHADER::LINE]);
	GLint uniformLocation;

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "xPos");
	glUniform1f(uniformLocation, xPos);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "yPos");
	glUniform1f(uniformLocation, yPos);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "zPos");
	glUniform1f(uniformLocation, zPos);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "xRot");
	glUniform1f(uniformLocation, lookUp);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "yRot");
	glUniform1f(uniformLocation, lookRight);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "brokenFocus");
	glUniform1f(uniformLocation, focus);

	uniformLocation = glGetUniformLocation(shader[SHADER::LINE], "scene3");
	glUniform1f(uniformLocation, scene3);

	return !CheckGLErrors("loadUniforms");
}

//Draws buffers to screen
void render()
{
	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)

	//Don't need to call these on every draw, so long as they don't change
	glUseProgram(shader[SHADER::LINE]);		//Use LINE program
	glBindVertexArray(vao[VAO::LINES]);		//Use the LINES vertex array

	loadUniforms();

	glDrawArrays(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			0,					//Starting index
			points.size()		//How many vertices
			);

	CheckGLErrors("render");
}

string readFile(string filename){
	ifstream myFile;
	
	myFile.open(filename);
	
	myFile.seekg(0, myFile.end);
	int length = (myFile.tellg());
	myFile.seekg(0,myFile.beg);

	char *buffer = new char[length];
	myFile.read(buffer,length);
	myFile.close();

	string retString(buffer);
	return retString;
}

//Taken from: http://ysonggit.github.io/coding/2014/12/16/split-a-string-using-c.html
void split(const string &s, const char* delim, vector<string> & v){
    char * dup = strdup(s.c_str());
    char * token = strtok(dup, delim);
    while(token != NULL){
        v.push_back(string(token));
        token = strtok(NULL, delim);
    }
    free(dup);
}

void parse(string line, int id){
	vector<string> coords;
	split(line, " ", coords);
	GLfloat x = stof(coords[0]);
	GLfloat y = stof(coords[1]);
	GLfloat z = stof(coords[2]);

	switch (id) {
		case 0 :
			triangleVecs.push_back(vec3(x,y,z));
			break;
		case 1 :
			sphereVecs.push_back(vec3(x,y,z));
			break;
		case 2 :
			planeVecs.push_back(vec3(x,y,z));
			break;
		case 3 :
			lightVecs.push_back(vec3(x,y,z));
			break;
	}
}

void parseObjects(string textData){
	vector<string> lines;
	split(textData,  "\n", lines);
	for (unsigned i = 0; i < lines.size(); i++){
		char id = lines[i][0];
		if (id == 't'){
			parse(lines[i+1], 0);
			parse(lines[i+2], 0);
			parse(lines[i+3], 0);
			parse(lines[i+4], 0);
			parse(lines[i+5], 0);
			i+=5;
		}
		else if (id == 's'){
			parse(lines[i+1], 1);
			parse(lines[i+2], 1);
			parse(lines[i+3], 1);
			parse(lines[i+4], 1);
			i+=4;
		}
		else if (id == 'p'){
			parse(lines[i+1], 2);
			parse(lines[i+2], 2);
			parse(lines[i+3], 2);
			parse(lines[i+4], 2);
			i+=4;
		}
		else if (id == 'l'){
			parse(lines[i+1], 3);
			parse(lines[i+2], 3);
			i+=2;
		}
		else{
			//do nothing
		}
	}
}

bool loadUniformBuffer(){
	int count = 0;
	GLint uniformLocation;
	vec3 temp;
	string uniformName;
	for (unsigned i = 0; i < 200; i++){
		glUseProgram(shader[SHADER::LINE]);
		string nameBeg = "triangles[";
		string strCount = to_string(count);

		if (i < triangleVecs.size())
			temp = triangleVecs[i];
		else
			temp = vec3(0,0,0);

		string nameEnd = "].p0"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform1 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform1);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < triangleVecs.size())
			temp = triangleVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].p1"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform2 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform2);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < triangleVecs.size())
			temp = triangleVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].p2"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform3 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform3);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < triangleVecs.size())
			temp = triangleVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].color"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform4 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform4);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < triangleVecs.size()){
			temp = triangleVecs[i];
		}
		else {
			temp = vec3(0,0,0);
		}
		nameEnd = "].p"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform5 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform5);
		glUniform1f(uniformLocation, temp.x);

		nameEnd = "].specCol"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform6 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform6);
		glUniform1f(uniformLocation, temp.y);

		nameEnd = "].ref"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform7 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform7);
		glUniform1f(uniformLocation, temp.z);

		count++;
	}
	count = 0;
	for (unsigned i = 0; i < 40; i++){
		glUseProgram(shader[SHADER::LINE]);
		string nameBeg = "spheres[";
		string strCount = to_string(count);

		if (i < sphereVecs.size())
			temp = sphereVecs[i];
		else
			temp = vec3(0,0,0);

		string nameEnd = "].center"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform1 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform1);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < sphereVecs.size())
			temp = sphereVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].radius"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform2 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform2);
		glUniform1f(uniformLocation, temp.x);
		i++;
		if (i < sphereVecs.size())
			temp = sphereVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].color"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform3 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform3);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < sphereVecs.size()){
			temp = sphereVecs[i];
		}
		else{
			temp = vec3(0,0,0);
		}

		nameEnd = "].p"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform4 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform4);
		glUniform1f(uniformLocation, temp.x);

		nameEnd = "].specCol"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform5 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform5);
		glUniform1f(uniformLocation, temp.y);

		nameEnd = "].ref"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform6 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform6);
		glUniform1f(uniformLocation, temp.z);

		count++;
	}
	count = 0;
	for (unsigned i = 0; i < 8; i++){
		glUseProgram(shader[SHADER::LINE]);
		string nameBeg = "planes[";
		string strCount = to_string(count);

		if (i < planeVecs.size())
			temp = planeVecs[i];
		else
			temp = vec3(0,0,0);

		string nameEnd = "].normal"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform1 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform1);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < planeVecs.size())
			temp = planeVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].point"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform2 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform2);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < planeVecs.size())
			temp = planeVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].color"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform3 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform3);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < planeVecs.size()){
			temp = planeVecs[i];
		}
		else{
			temp = vec3(0,0,0);
		}
		
		nameEnd = "].p"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform4 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform4);
		glUniform1f(uniformLocation, temp.x);

		nameEnd = "].specCol"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform5 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform5);
		glUniform1f(uniformLocation, temp.y);

		nameEnd = "].ref"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform6 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform6);
		glUniform1f(uniformLocation, temp.z);

		count++;
	}
	count = 0;
	for (unsigned i = 0; i < 4; i++){
		glUseProgram(shader[SHADER::LINE]);
		string nameBeg = "lights[";
		string strCount = to_string(count);

		if (i < lightVecs.size())
			temp = lightVecs[i];
		else
			temp = vec3(0,0,0);

		string nameEnd = "].pos"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform1 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform1);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);
		i++;
		if (i < lightVecs.size())
			temp = lightVecs[i];
		else
			temp = vec3(0,0,0);

		nameEnd = "].color"; uniformName = nameBeg + strCount + nameEnd;
		const char * uniform2 = uniformName.c_str();
		uniformLocation = glGetUniformLocation(shader[SHADER::LINE], uniform2);
		glUniform3f(uniformLocation, temp.x, temp.y, temp.z);

		count++;
	}
	planeVecs.clear();
    sphereVecs.clear();
    lightVecs.clear();
    triangleVecs.clear();
	return !CheckGLErrors("loadUniformBuffer");
}

// --------------------------------------------------------------------------
// GLFW callback functions

bool incZ = false;
bool decZ = false;
bool incX = false;
bool decX = false;
bool jump = false;
float yDisp = 0.f;
bool upRot = false;
bool downRot = false;
bool leftRot = false;
bool rightRot = false;

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_1 && action == GLFW_PRESS){
        string textData = readFile("scene1.txt");
        parseObjects(textData);
        loadUniformBuffer();
        scene3 = false;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS){
        string textData = readFile("scene2.txt");
        parseObjects(textData);
        loadUniformBuffer();
        scene3 = false;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS){
        string textData = readFile("scene3.txt");
        parseObjects(textData);
        loadUniformBuffer();
        scene3 = true;
    }
    if (key == GLFW_KEY_W){
    	if(action == GLFW_PRESS){
    		decZ = true;
    	}
    	else if (action == GLFW_RELEASE){
    		decZ = false;
    	}
    }
    if (key == GLFW_KEY_S){
    	if(action == GLFW_PRESS){
    		incZ = true;
    	}
    	else if (action == GLFW_RELEASE){
    		incZ = false;
    	}
    }
    if (key == GLFW_KEY_A){
    	if(action == GLFW_PRESS){
    		decX = true;
    	}
    	else if (action == GLFW_RELEASE){
    		decX = false;
    	}
    }
    if (key == GLFW_KEY_D){
    	if(action == GLFW_PRESS){
    		incX = true;
    	}
    	else if (action == GLFW_RELEASE){
    		incX = false;
    	}
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
    	jump = true;
    }
    if (key == GLFW_KEY_UP){
    	if(action == GLFW_PRESS){
    		upRot = true;
    	}
    	else if (action == GLFW_RELEASE){
    		upRot = false;
    	}
    }
    if (key == GLFW_KEY_DOWN){
    	if(action == GLFW_PRESS){
    		downRot = true;
    	}
    	else if (action == GLFW_RELEASE){
    		downRot = false;
    	}
    }
    if (key == GLFW_KEY_RIGHT){
    	if(action == GLFW_PRESS){
    		rightRot = true;
    	}
    	else if (action == GLFW_RELEASE){
    		rightRot = false;
    	}
    }
    if (key == GLFW_KEY_LEFT){
    	if(action == GLFW_PRESS){
    		leftRot = true;
    	}
    	else if (action == GLFW_RELEASE){
    		leftRot = false;
    	}
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS){
    	if(!focus){
    		focus = true;
    	}
    	else {
    		focus = false;
    	}
    }
}

//Initialization
void initGL()
{
	generateIDs();
	initShader();
	initVAO();
	generateSquare(2.f);
	loadBuffer(points, uvs);
	string textData = readFile("scene1.txt");
    parseObjects(textData);
    loadUniformBuffer();
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{   
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(512, 512, "Susant Pant A4", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
    	if(incX)
    		xPos += 0.2f;
    	if(decX)
    		xPos -= 0.2f;
    	if(incZ)
    		zPos += 0.2f;
    	if(decZ)
    		zPos -= 0.2f;
    	if(jump){
    		yDisp += 0.1f;
    		yPos = 1.6*sin(yDisp*1.3);
    		if (yPos < 0.f){
    			yPos = 0.f;
    			yDisp = 0.f;
    			jump = false;
    		}
    	}
    	if(rightRot)
    		lookRight -= PI/90.f;
    	if(leftRot)
    		lookRight += PI/90.f;
    	if(upRot)
    		lookUp += PI/90.f;
    	if(downRot)
    		lookUp -= PI/90.f;
        // call function to draw our scene
        render();

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
	}

	// clean up allocated resources before exit
	deleteIDs();
	glfwDestroyWindow(window);
	glfwTerminate();

   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}
