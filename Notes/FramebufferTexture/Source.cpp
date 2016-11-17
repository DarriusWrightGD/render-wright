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

GLfloat vertices[6][3]{
	{ -0.9f, -0.9f, -1.0f },
	{ 0.85f, -0.9f, - 1.0f },
	{ -0.9f, 0.85f,-1.0f },
	{ 0.9f, -0.85f,-1.0f },
	{ 0.9f, 0.9f,-1.0f },
	{ -0.85f, 0.9f,-1.0f },
};

GLfloat textureCoords[6][2]{
	{ -1,1 },
	{ 1,-1 },
	{ -1,-1 },
	{ 1,-1 },
	{ -1,1 },
	{1,1},
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
	GLuint triangleVertexArray;
	glGenVertexArrays(1, &triangleVertexArray);
	glBindVertexArray(triangleVertexArray);

	GLuint triangleBuffer;
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices  +sizeof textureCoords, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof vertices, sizeof textureCoords, textureCoords);

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


	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof vertices));
	glEnableVertexAttribArray(1);

	GLuint framebufferTexture;
	GLuint framebuffer;
	GLuint depthbuffer;

	glGenTextures(1, &framebufferTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//mutable texture 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glGenRenderbuffers(1, &depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, depthbuffer);
	glEnable(GL_DEPTH_TEST);

	GLuint textureTriLoc = glGetUniformLocation(program, "textureTri");
	while (!glfwWindowShouldClose(window))
	{
		glInvalidateTexImage(framebufferTexture, 0);
		glClearColor(sin(glfwGetTime()), .2, cos(glfwGetTime()), 1);
		glUniform1i(textureTriLoc, GL_FALSE);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glClearColor(1, 1, 0, 1);
		glUniform1i(textureTriLoc, GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// if you know that you are going to redraw the entire screen then it can be beneficial to 
		// clear the framebuffer through invalidation. Otherwise it is not wise to do so as OpenGL will
		// clear the buffer in a better way.
		GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_COMPONENT };
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
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
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebufferTexture);
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

