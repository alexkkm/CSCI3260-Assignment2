#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/stb_image/stb_image.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;
using glm::vec3;
using glm::mat4;


GLint programID;
// Could define the Vao&Vbo and interaction parameter here
GLuint catVAO;
GLuint catVBO;
GLuint catEBO;
int cat_indice_size;

GLuint floorVAO;
GLuint floorVBO;
GLuint floorEBO;
int floor_indice_size;

GLuint floorTexture;
GLuint catTexture[2];

float delta = 0.2f;
int flag = 0;
int b_num = 4;
int flash_switch = 0;

//object movement
int moveForward = 0;
int moveRotate = 0;
int old=0;
float old_x=0;
float old_z=0;
int old_moveRotate=0;

float PI = glm::pi<float>();

//mouse camera
bool mouseLeftDown = false;
int drag_x_origin;
int drag_y_origin;
int camera_x;
int camera_y;
float zoom = 1.0;

//light rotation
int alpha = 0;
int press_pause=0;

GLuint normalbuffer;

//a series utilities for setting shader parameters 
void setMat4(const std::string& name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}
void setVec4(const std::string& name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string& name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void mouse_callback(int button, int state, int x, int y)
{
	//TODO: Use mouse to do interactive events and animation
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
	{
		mouseLeftDown = true;
		drag_x_origin = x;
		drag_y_origin = y;
	}

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
	{
		mouseLeftDown = false;
	}

	if (button == 3)
	{
		zoom += 0.1;
	}

	if (button == 4)
	{
		if (zoom > 0.2 )
		{
			zoom -= 0.1;
		}
	}
}

void motion_callback(int x, int y)
{
	//TODO: Use mouse to do interactive events and animation
	if (mouseLeftDown == true)
	{
		camera_x += (x - drag_x_origin);
		camera_y += (y - drag_y_origin);
		drag_x_origin = x;
		drag_y_origin = y;
	}
}

void keyboard_callback(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation

	if (key == '1')
	{
		flag = 0;
	}

	if (key == '2')
	{
		flag = 1;
	}

	if (key == 'w')
	{
		b_num++;
	}

	if (key == 's')
	{
		if (b_num >-5)
		{
			b_num--;
		}
	}

	if (key == 'p')
	{
		if (press_pause == 0)
		{
			press_pause = 1;
		}
		else if (press_pause == 1)
		{
			press_pause = 0;
		}
	}
}

void special_callback(int key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation
	if (key == GLUT_KEY_UP)
	{
		moveForward += 1;
	}
	if (key == GLUT_KEY_DOWN)
	{
		moveForward -= 1;
	}

	if (key == GLUT_KEY_LEFT)
	{
		moveRotate += 1;
	}

	if (key == GLUT_KEY_RIGHT)
	{
		moveRotate -= 1;
	}
}

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

GLuint loadTexture(const char* texturePath)
{
	// tell stb_image.h to flip loaded texture's on the y-axis.
	stbi_set_flip_vertically_on_load(true);
	// load the texture data into "data"
	int Width, Height, BPP;
	unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
	// Please pay attention to the format when sending the data to GPU
	GLenum format = 3;
	switch (BPP) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}
	if (!data) {
		std::cout << "Failed to load texture: " << texturePath << std::endl;
		exit(1);
	}

	GLuint textureID = 0;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);

	//"Bind" the newly created texture:
	//to indicate all future texture functions will  modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	
	//Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data);
	std::cout << "Load " << texturePath << " successfully!" << std::endl;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture
	
	Model cat = loadOBJ("./resources/cat/cat.obj");
	cat_indice_size = cat.indices.size();

	catTexture[0] = loadTexture("./resources/cat/cat_01.jpg");
	catTexture[1] = loadTexture("./resources/cat/cat_02.jpg");

	glGenVertexArrays(1, &catVAO);
	glBindVertexArray(catVAO);

	glGenBuffers(1, &catVBO);
	glBindBuffer(GL_ARRAY_BUFFER, catVBO);
	glBufferData(GL_ARRAY_BUFFER, cat.vertices.size()*sizeof(Vertex), &cat.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1,&catEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, catEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cat.indices.size()*sizeof(unsigned int), &cat.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  //vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);  //texture uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);  //vertex normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); //close vao
	
	
	Model floor = loadOBJ("./resources/floor/floor.obj");
	floor_indice_size = floor.indices.size();

	floorTexture = loadTexture("./resources/floor/floor_diff.jpg");

	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);

	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, floor.vertices.size() * sizeof(Vertex), &floor.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &floorEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, floor.indices.size() * sizeof(unsigned int), &floor.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	
	glEnableVertexAttribArray(1);  //texture uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);  //vertex normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
}

void paintGL(void)
{
	/*
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x_press_num * delta + 0.f, 0.f, y_press_num * delta + 0.f));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)(0.9 * rotate_press) + 0.f), glm::vec3(0.f, 1.f, 0.f));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3((float)(delta * scale_num) + 1.f));
	glUniformMatrix4fv(glGetUniformLocation(programID, "ModelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	*/

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 2, 15),
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, 1, 0)		// Head is up (set to 0,-1,0 to look upside-down)
	);
	View = glm::rotate(View, glm::radians((float)(0.3 * camera_y) + 0.f), glm::vec3(1.f, 0.f, 0.f));
	View = glm::rotate(View, glm::radians((float)(0.3 * camera_x) + 0.f), glm::vec3(0.f, 1.f, 0.f));
	View = glm::scale(View,  glm::vec3((float)(zoom))) ;

	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mvp = Projection * View * Model;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	unsigned ModelLoc = glGetUniformLocation(programID, "ModelMatrix");

	//-----------------------------------------------------------//
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");   
	glm::vec4 ambientLight(0.1f, 0.1f, 0.1f, 0.0f);
	glUniform4fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	GLint spotLightPoint = glGetUniformLocation(programID, "spotSourcePosition");
	vec3 spotLightLoc(0.0f, +15.0f, 0.0f);  //spotlight source
	glUniform3fv(spotLightPoint, 1, &spotLightLoc[0]);

	GLint spotLightBeam = glGetUniformLocation(programID, "spotLightDir");
	vec3 spotLightBmDir( cos(alpha * 2*PI/180), -5.0f,  - sin(alpha * 2*PI/180) );  //spotlight direction
	
	glUniform3fv(spotLightBeam, 1, &spotLightBmDir[0]);

	//extract camera position from view
	mat4 viewModel = glm::inverse(View);
	vec3 cameraPos(viewModel[3]);

	GLint eyePositionUniformLocation =  glGetUniformLocation(programID, "eyePositionWorld");
	//vec3 eyePosition(0.0f, 2.0f, 10.0f);
	//glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);
	glUniform3fv(eyePositionUniformLocation, 1, &cameraPos[0]);

	GLint b = glGetUniformLocation(programID, "bright");
	glUniform1f(b, b_num * 0.1);

	GLint flashGlobal = glGetUniformLocation(programID, "flash");
	glUniform1i(flashGlobal, flash_switch);

	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition(-20.0f, -10.0f, 0.0f);  //direction light
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
	

	//------------------------------------------------------------------//
	glBindVertexArray(floorVAO);   //ground
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)); //array buffer offset
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	ModelMatrix = glm::scale(ModelMatrix, glm::vec3((float)0.6f));

	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

	//Textureeeeeeeeeeeeeeeeeeeeee
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glUniform1i(TextureID, 0);

	glDrawElements(GL_TRIANGLES, floor_indice_size, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//--------------------------------------------------------//
	
	glBindVertexArray(catVAO);
	glBindBuffer(GL_ARRAY_BUFFER, catVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)); //array buffer offset
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//Textureeeeeeeeeeeeeeeeeeeeee
	GLuint TextureID1 = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);

	//change texture
	if(flag==0)
	{
		glBindTexture(GL_TEXTURE_2D, catTexture[0]);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, catTexture[1]);
	}

	glUniform1i(TextureID1, 0);
	//ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.f, -1.0f, 0.f));
	
	ModelMatrix = glm::mat4(1.0f);
	if (old == moveForward)
	{
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3((float)0.05f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(old_x *1.5 + 0.f, 0.0f, old_z*1.5 +0.f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.f, -12.0f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(moveRotate * 2 + 0.f), glm::vec3(0.f, 0.f, 1.f));
	}
	else if(old < moveForward)
	{
		float kx = old_x + sin(moveRotate * 2 * PI / 180);   //in terms of radian
		float kz = old_z + cos(moveRotate * 2 * PI / 180);

		if (kx > -70 && kx < 70)  //movement limit
		{
			old_x = kx;
		}
		if (kz > -56 && kz < 56)    //limit
		{
			old_z = kz;
		}

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3((float)0.05f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(old_x*1.5  +0.f, 0.0f, old_z *1.5+ 0.f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.f, -12.0f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(moveRotate * 2 + 0.f), glm::vec3(0.f, 0.f, 1.f));

		old = moveForward;

	}
	else if(old > moveForward)
	{
		float kx = old_x - sin(moveRotate * 2 * PI / 180);
		float kz = old_z - cos(moveRotate * 2 * PI / 180);

		if (kx > -70 && kx < 70)
		{
			old_x = kx;
		}
		if (kz > -56 && kz < 56)
		{
			old_z = kz;
		}

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3((float)0.05f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(old_x*1.5 + 0.f, 0.0f, old_z *1.5 + 0.f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.f, -12.0f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(moveRotate * 2 + 0.f), glm::vec3(0.f, 0.f, 1.f));

		old = moveForward;
		//limit: 105,85
	}
	
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

	glDrawElements(GL_TRIANGLES, cat_indice_size, GL_UNSIGNED_INT, 0 );
	
	glFlush();
	glutPostRedisplay();
}

void timerProc(int id)
{
	if (press_pause == 0)
	{
		alpha = alpha % 360;
		alpha++;
	}
	glutTimerFunc(50,timerProc,1);
}

void timerFlash(int id)
{
	if (flash_switch == 0)
	{
		flash_switch = 1;
	}
	else if(flash_switch == 1)
	{
		flash_switch = 0;
	}
	glutTimerFunc(10, timerFlash, 2);
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1080, 800);

	glutCreateWindow("Assignment 2");

	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */

	initializedGL();
	glutDisplayFunc(paintGL);

	glutMouseFunc(mouse_callback);
	glutMotionFunc(motion_callback);
	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(special_callback);
	glutTimerFunc(50,timerProc,1);
	glutTimerFunc(10,timerFlash, 2);
	glutMainLoop();

	return 0;
}

void mouseWheel_callback(int wheel, int direction, int x, int y)
{
	// Optional.
}