#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

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

#include <glm\gtc\random.hpp>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <vector>
#include <SOIL.h>

int main() {
	init();

	/*
	Any OpenGL program consists of the following steps...
	1. Specify the data for constructing shapes from OpenGLs primitives
	2. Excute shaders
	3. Convert the input data into fragments
	4. Perform additional per-fragment operations.
	*/
	GLuint particleVertexArray;
	glGenVertexArrays(1, &particleVertexArray);
	glBindVertexArray(particleVertexArray);

	GLuint particleBuffer;
	glGenBuffers(1, &particleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
	std::vector<glm::vec3> points(1000);
	for (size_t i = 0; i < points.size(); i++)
	{
		//points[i] = glm::vec3(0.0,0.0,-2.0);
		points[i] = glm::ballRand(200.0f) - glm::vec3(100,100,400.0f);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), points.data(), GL_STATIC_DRAW);

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

	glm::mat4 projection = glm::perspective(glm::radians(70.0f), static_cast<float>(width) / height, 0.1f, 1000.0f);

	GLint projLocation = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	int width, height, channels;
	auto image = SOIL_load_image("../../Models/Images/light.jpg", &width, &height, &channels, SOIL_LOAD_AUTO);

	GLuint pointTexture;
	glGenTextures(1, &pointTexture);
	glBindTexture(GL_TEXTURE_2D, pointTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

	delete[] image;
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glPointSize(20.0f);
	//glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_POINTS, 0, points.size());
		
#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &particleBuffer);
	glDeleteVertexArrays(1, &particleVertexArray);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
	glDeleteTextures(1, &pointTexture);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (GL_DEBUG_TYPE_ERROR == type)
		std::cout << message << std::endl;
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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

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

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	GLuint unusedIds = 0;
	glDebugMessageControl(GL_DONT_CARE,
		GL_DONT_CARE,
		GL_DONT_CARE,
		0,
		&unusedIds,
		true);
}

