#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <glm\gtc\noise.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <SOIL.h>
#include <vector>

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

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 uv;
};

void createGrid(float width, float depth, uint32_t m, uint32_t n, std::vector<Vertex> & vertices, std::vector<GLuint> & indices) {
	uint32_t vertexCount = m * n;
	uint32_t faceCount = (m - 1) * (n - 1) * 2;

	vertices.resize(vertexCount);
	indices.resize(faceCount * 3);

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	for (GLuint i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dz;
		for (GLuint j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;

			vertices[i*n + j].position = glm::vec3(x, 0.0f, z);
			vertices[i*n + j].normal = glm::vec3(0.0f, 1.0f, 0.0f);

			// Stretch texture over grid.
			vertices[i*n + j].uv.x = j*du;
			vertices[i*n + j].uv.y = i*dv;
		}
	}

	GLuint k = 0;
	for (GLuint i = 0; i < m - 1; ++i)
	{
		for (GLuint j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;
			
			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}
}

int main() {
	init();

	/*Tesselation is the process of breaking a large primitive into smaller ones before rendering it.
	The most common use for the this stage is to add more detail to a low detail model.*/

	GLubyte * heightMap = new GLubyte[width * height];

	GLfloat freqency = 6.0;
	GLfloat scale = 1.5;
	GLfloat xFactor = 1.0f / (width - 1);
	GLfloat yFactor = 1.0f / (height - 1);
	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			glm::vec2 p(x * xFactor * freqency, y * yFactor * freqency);
			float value = glm::perlin(p) / scale;
			float result = (value + 1.0f) / 2.0f;

			heightMap[(x + width * y)] = static_cast<GLubyte>(result * 255.0f);
		}
	}

	SOIL_save_image("heightMap.bmp", SOIL_SAVE_TYPE_BMP, width, height, 1, heightMap);

	GLuint heightTexture;
	glGenTextures(1, &heightTexture);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, heightMap);

	delete[] heightMap;

	GLint maxPatches;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatches);

	std::cout << "Max patches : " << maxPatches << std::endl;

	GLuint gridVertexArray;
	glGenVertexArrays(1, &gridVertexArray);
	glBindVertexArray(gridVertexArray);

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	createGrid(30.0f, 30.0f, 60, 60, vertices, indices);

	GLuint gridBuffer;
	glGenBuffers(1, &gridBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof Vertex * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	GLuint gridIndexBuffer;
	glGenBuffers(1, &gridIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof GLuint * indices.size(), indices.data(), GL_STATIC_DRAW);

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



	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<void*>(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<void*>(sizeof glm::vec3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<void*>(sizeof glm::vec3 * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);

	auto mvpLocation = glGetUniformLocation(program, "mvp");
	glClearColor(1, 1, 0, 1);
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		
		auto mvp = glm::perspective(glm::radians(60.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f)
			//* glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 20, -100), glm::vec3(0, 1, 0))
			* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
			* glm::translate(glm::vec3(0, -10, -50))
			//* glm::rotate((float)glfwGetTime(), glm::vec3(1, 0, 0));
		* glm::rotate(120.0f, glm::vec3(1,0,0));

		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &heightTexture);
	glDeleteBuffers(1, &gridBuffer);
	glDeleteVertexArrays(1, &gridVertexArray);
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
	window = glfwCreateWindow(640, 480, "Hello Terrain Tessellation", nullptr, nullptr);
	if (!window)
	{
		// Window or OpenGL context creation failed
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
	glfwSwapInterval(1);
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetWindowSizeCallback(window, resize_callback);
	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << " loaded" << std::endl;
}

