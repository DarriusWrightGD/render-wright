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
	GLuint faceCount = mesh->mNumFaces;
	vector<Vertex> vertices;//(mesh->mNumVertices);
	//int vertexCount = 0;
	for (GLuint i = 0; i < faceCount; i++)
	{
		const aiFace & face = mesh->mFaces[i];
		for (GLuint x = 0; x < 3; x++)
		{
			auto position = mesh->mVertices[face.mIndices[x]];
			vertices.push_back({ {position.x, position.y, position.z} });
			//vertexCount++;
		}
	}

	GLuint bearVertexArray;
	glGenVertexArrays(1, &bearVertexArray);
	glBindVertexArray(bearVertexArray);

	GLuint bearBuffer;
	glGenBuffers(1, &bearBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bearBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

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

	auto mvpIndex = glGetUniformLocation(program, "mvp");
	glm::mat4 mvp = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.f) // projection
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))  // view
		*  glm::translate(glm::vec3{ 0.0f,-8.0f,-70.0f }) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 1.0f, 0.0f)); // model

	glUniformMatrix4fv(mvpIndex, 1, GL_FALSE, &mvp[0][0]);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glEnable(GL_POLYGON_OFFSET_FILL);

	auto uniformLocation= glGetUniformLocation(program, "color");

	glEnable(GL_DEPTH_TEST);
	glPolygonOffset(0.75, 1);
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto color = glm::vec4(0, 1, 0, 1);
		glUniform4fv(uniformLocation, 1, &color[0]);
		glPolygonMode(GL_FRONT, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		color = glm::vec4(1, 0, 0, 1);
		glUniform4fv(uniformLocation, 1, &color[0]);
		glPolygonMode(GL_FRONT, GL_LINE);
		glDrawArrays(GL_LINES, 0, vertices.size());



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
	window = glfwCreateWindow(640, 480, "Hello Polygon offset Loading", NULL, NULL);
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
	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << " loaded" << std::endl;
}

