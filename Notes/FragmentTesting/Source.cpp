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

GLfloat vertices[9][3]{
	{ -0.9, -0.9, -0.1 },
	{ 0.85, -0.9,-0.1 },
	{ -0.9, 0.85,-0.1 },
	{ 0.5, -0.85,0 },
	{ 0.9, 0.9 ,0},
	{ -0.85, 0.9,0},
	{ 1.0, -0.85,-0.1 },
	{ 1.0, 1.0 ,-0.1 },
	{ -0.85, 1.0,-0.1 },
};

GLfloat colors[9][4]{
	{ 0.2,0.5,0.6, 1.0 },
	{ 0.2,0.0,0.2, 1.0 },
	{ 0.2,0.1,0.7, 1.0 },
	{ 0.0,0.0,0.0, 1.0 },
	{ 0.0,0.0,0.0, 1.0 },
	{ 0.0,0.0,0.0, 1.0 },
	{ 0.0,1.0,0.0, 1.0 },
	{ 0.0,1.0,0.0, 1.0 },
	{ 0.0,1.0,0.0, 1.0 },
};


int main() {
	init();

	/*
	Any OpenGL program consists of the following steps...
	1. Specify the data for constructing shapes from OpenGLs primitives
	2. Excute shaders
	3. Convert the input data into fragments
	4. Perform additional per-fragment operations.
	*/

	/*
	There are various tests that decide if a fragment will become a pixel or not

	1. Scissor Test
	2. Multisample fragment operations
	3. Stencil Test
	4. Depth test
	5. Blending
	6. Dithering
	7. Logical operations
	*/
	GLuint triangleVertexArray;
	glGenVertexArrays(1, &triangleVertexArray);
	glBindVertexArray(triangleVertexArray);

	GLuint triangleBuffer;
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices + sizeof colors, nullptr, GL_STATIC_DRAW);

	auto triangleData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof vertices + sizeof colors, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
	memcpy(triangleData, vertices, sizeof vertices);
	glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, sizeof vertices);
	memcpy(triangleData + sizeof vertices, colors, sizeof colors);
	glFlushMappedBufferRange(GL_ARRAY_BUFFER, sizeof vertices, sizeof colors);
	glFinish();
	glUnmapBuffer(GL_ARRAY_BUFFER);

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


	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof vertices));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glClearColor(1, 1, 1, 1);


	glEnable(GL_SCISSOR_TEST);
	
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DITHER);
	glEnable(GL_COLOR_LOGIC_OP);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	glClearStencil(0x0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);//default depth function
	glEnable(GL_STENCIL_TEST);

	glLogicOp(GL_XOR);

	while (!glfwWindowShouldClose(window))
	{
		glScissor(50, 50, width - 100, height - 100);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		glStencilFunc(GL_ALWAYS, 1, 0xF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glDrawArrays(GL_TRIANGLES, 3, 3);

		glStencilFunc(GL_EQUAL, 1, 0xF);
		glDrawArrays(GL_TRIANGLES, 6, 3);
		glStencilFunc(GL_NOTEQUAL, 1, 0xF);
		glDrawArrays(GL_TRIANGLES, 0, 3);
#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &triangleBuffer);
	glDeleteVertexArrays(1, &triangleVertexArray);
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
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(640, 480, "Hello Fragment Testing", nullptr, nullptr);
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

