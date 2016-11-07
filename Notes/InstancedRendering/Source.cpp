#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\random.hpp>
#include <vector>

using std::vector;

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void init();

GLFWwindow* window;
int width, height;

static void resize_callback(GLFWwindow * window, int w, int h) {
	width = w;
	height = h;
}

GLuint createShader(const char * filename, GLuint shaderType) {

	std::ifstream shaderCodeStream(filename);
	if (!shaderCodeStream.is_open()) {
		std::cout << "The file " << filename << " does not exist!" << std::endl;
		exit(-1);
	}

	auto shaderCode = std::string((std::istreambuf_iterator<char>(shaderCodeStream)), std::istreambuf_iterator<char>());
	auto shaderCharArray = shaderCode.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCharArray, nullptr);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		char * log = new char[logSize];
		glGetShaderInfoLog(shader, logSize, nullptr, log);
		std::cout << "Shader Error: " << log << std::endl;
		delete[] log;
		exit(-1);
	}

	return shader;
}

struct Vertex {
	glm::vec3 position;
};

int main() {
	init();

	/*
	Any OpenGL program consists of the following steps...
	1. Specify the data for constructing shapes from OpenGLs primitives
	2. Execute shaders
	3. Convert the input data into fragments
	4. Perform additional per-fragment operations.
	*/

	Assimp::Importer importer;
	const aiScene * scene = importer.ReadFile("../../Models/Bear/bear-obj.obj",
		aiProcess_Triangulate
	);

	if (!scene) {
		std::cout << "Loading scene failed" << std::endl;
		exit(-1);
	}

	aiMesh* mesh = scene->mMeshes[0]; 
	int faceCount = mesh->mNumFaces;
	vector<Vertex> vertices;
	vector<GLuint> indices;
	for (size_t i = 0; i < faceCount; i++)
	{
		const aiFace & face = mesh->mFaces[i];
		for (size_t x = 0; x < 3; x++)
		{
			auto position = mesh->mVertices[face.mIndices[x]];
			vertices.push_back({ { position.x, position.y, position.z } });
		}
	}

	glClearColor(1, 1, 1, 1);

	GLuint bearVertexArray;
	glGenVertexArrays(1, &bearVertexArray);
	glBindVertexArray(bearVertexArray);

	GLuint bearBuffer;
	glGenBuffers(1, &bearBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bearBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	GLuint instanceCount = 1000;
	GLuint colorBuffer;

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof glm::vec3 * instanceCount, nullptr, GL_STATIC_DRAW);

	auto colorData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof glm::vec3 * instanceCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

	for (size_t i = 0; i < instanceCount; i++)
	{
		glm::vec3 randomColor = glm::linearRand(glm::vec3(0.2f,0.4f,0.1f), glm::vec3(1.0f));
		memcpy(colorData + sizeof glm::vec3 * i, &randomColor[0], sizeof glm::vec3);
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, sizeof glm::vec3 * i, sizeof glm::vec3);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glFinish();

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	GLuint mvpBuffer;
	glGenBuffers(1, &mvpBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mvpBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof glm::mat4 * instanceCount, nullptr, GL_STATIC_DRAW);

	auto mvpData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof glm::mat4 * instanceCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
	glm::mat4 viewProjection = glm::perspective(glm::radians(90.0f), static_cast<float>(width)/height, 0.1f, 3000.f) // projection
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	
	for (size_t i = 0; i < instanceCount; i++)
	{
		auto randomPosition = glm::linearRand(glm::vec3(-100.0f, -40.0f, -30.0f), glm::vec3(100.0f, 40.0f, -200.0f));
		auto randomRotation = glm::radians(glm::linearRand(0.0f, 360.0f));
		glm::mat4 mvp = viewProjection * glm::translate(randomPosition) * glm::rotate(randomRotation, glm::vec3(1.0f));
		
		memcpy(mvpData + sizeof glm::mat4 * i, &mvp[0][0], sizeof(glm::mat4));
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, sizeof glm::mat4 * i, sizeof glm::mat4);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glFinish();

	GLuint matLocation = 2;
	for (size_t i = 0; i < 4; i++)
	{

		glVertexAttribPointer(matLocation + i, 4, GL_FLOAT, GL_FALSE, sizeof glm::mat4, reinterpret_cast<void*>(sizeof glm::vec4 * i));
		glEnableVertexAttribArray(matLocation + i);
		glVertexAttribDivisor(matLocation + i, 1);
	}



	GLuint vertexShader = createShader("vertexShader.vert", GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader("fragmentShader.frag", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint programStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &programStatus);
	if (programStatus != GL_TRUE) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char * log = new char[logLength];
		glGetProgramInfoLog(program, logLength, nullptr, log);
		std::cerr << "Program Error: " << log << std::endl;
		delete[] log;
		exit(-1);
	}

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glUseProgram(program);

	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glDrawArraysInstanced(GL_TRIANGLES, 0, vertices.size(), instanceCount);
#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &bearBuffer);
	glDeleteVertexArrays(1, &bearVertexArray);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void init() {
	if (!glfwInit()) {
		//failed
		glfwTerminate();
		exit(-1);
	}

	glfwSetErrorCallback(error_callback);


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	window = glfwCreateWindow(640, 480, "1000 Bears", NULL, NULL);
	if (!window)
	{
		// Window or OpenGL context creation failed
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetWindowSizeCallback(window, resize_callback);

	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << " loaded" << std::endl;
}

