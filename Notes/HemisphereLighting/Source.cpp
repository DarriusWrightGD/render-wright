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
#include <SOIL.h>

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


	GLuint bearVertexArray;
	glGenVertexArrays(1, &bearVertexArray);
	glBindVertexArray(bearVertexArray);

	GLuint vertexCount = mesh->mNumVertices;
	GLuint positionSize = sizeof glm::vec3 * vertexCount;
	GLuint normalSize = sizeof glm::vec3 * vertexCount;
	GLuint uvSize = sizeof glm::vec2 * vertexCount;
	GLuint bufferSize = positionSize + normalSize + uvSize;

	GLuint bearBuffer;
	glGenBuffers(1, &bearBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bearBuffer);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

	auto bufferData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

	memcpy(bufferData, mesh->mVertices, positionSize);
	glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, positionSize);
	memcpy(bufferData + positionSize, mesh->mNormals, normalSize);
	glFlushMappedBufferRange(GL_ARRAY_BUFFER, positionSize, normalSize);

	auto uvOffset = normalSize + positionSize;
	for (size_t i = 0; i < vertexCount; i++)
	{
		auto copySize = sizeof glm::vec2;
		memcpy(bufferData + uvOffset + copySize * i, &mesh->mTextureCoords[0][i], copySize);
	}
	glFlushMappedBufferRange(GL_ARRAY_BUFFER, uvOffset, uvSize);

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glFinish();

	GLuint indexBuffer;
	GLuint indexCount = mesh->mNumFaces * 3;
	GLuint indexBufferSize = sizeof GLuint * indexCount;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, nullptr, GL_STATIC_DRAW);

	auto indexBufferData = reinterpret_cast<char*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, indexBufferSize, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		auto indices = mesh->mFaces[i].mIndices;
		auto copySize = sizeof GLuint * 3;
		memcpy(indexBufferData + copySize * i, indices, copySize);
	}

	glFlushMappedBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, indexBufferSize);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glFinish();

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
	auto normalMatrixLocation = glGetUniformLocation(program, "normalMatrix");
	auto modelLocation = glGetUniformLocation(program, "model");

	glm::mat4 model = glm::translate(glm::vec3{ 0.0f,-30.0f,-70.0f }) * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // model
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
	glm::mat4 mvp = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.f) // projection
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))  // view
		* model;

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(mvpIndex, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(positionSize));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(uvOffset));
	glEnableVertexAttribArray(2);

	int imageWidth, imageHeight, channels;
	auto image = SOIL_load_image("../../Models/Bear/bear.tga", &imageWidth, &imageHeight, &channels, SOIL_LOAD_AUTO);

	GLuint bearTexture;
	glGenTextures(1, &bearTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bearTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, imageWidth, imageHeight);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, image);

	delete[] image;

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &bearTexture);
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
	window = glfwCreateWindow(640, 480, "Hello Model Loading", NULL, NULL);
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

