#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <glm\gtc\noise.hpp>
#include <SOIL.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

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

int main() {
	init();
	
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


	/*Tesselation is the process of breaking a large primitive into smaller ones before rendering it.
	The most common use for the this stage is to add more detail to a low detail model.*/

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glEnable(GL_CULL_FACE);

	GLint maxPatches;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatches);

	std::cout << "Max patches : " << maxPatches << std::endl;

	GLuint vertexShader = createShader("vertexShader.vert", GL_VERTEX_SHADER);
	GLuint tessControl = createShader("tessellationControl.tess", GL_TESS_CONTROL_SHADER);
	GLuint tessEval = createShader("tessellationEval.tess", GL_TESS_EVALUATION_SHADER);
	GLuint fragmentShader = createShader("fragmentShader.frag", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, tessControl);
	glAttachShader(program, tessEval);
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


	
	glEnableVertexAttribArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);



	auto mvpLocation = glGetUniformLocation(program, "mvp");
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);

		auto mvp = glm::perspective(glm::radians(60.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f)
			//* glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 20, -100), glm::vec3(0, 1, 0))
			* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
			* glm::translate(glm::vec3(0, -10, -70))
			//* glm::rotate((float)glfwGetTime(), glm::vec3(1, 1, 0));
			* glm::rotate(120.0f, glm::vec3(1, 0, 0));
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
		
#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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
	window = glfwCreateWindow(640, 480, "Hello GLFW", nullptr, nullptr);
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

