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

GLfloat vertices[6][2]{
	{ -0.9, -0.9 },
	{ 0.85, -0.9 },
	{ -0.9, 0.85 },
	{ 0.9, -0.85 },
	{ 0.9, 0.9 },
	{ -0.85, 0.9},
};

GLfloat colors[6][3]{
	{ 0.1, 0.1, 0.2 },
	{ 0.85, 0.9, 0.9 },
	{ 0.1, 0.85, 0.3 },
	{ 0.2, 0.15, 0.2 },
	{ 0.5, 0.6, 0.8 },
	{ 0.5, 0.2, 0.1 },
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

	// Using subbuffer to assign the buffer values after 
	//GLuint triangleBuffer;
	//glGenBuffers(1, &triangleBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), nullptr, GL_STATIC_DRAW);

	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

	//Using copySubbuffer to assign the buffer data
	GLuint triangleBuffer;

	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, triangleBuffer);

	GLuint vertexBuffers[2]; //1 : position, 2: color
	glGenBuffers(2, vertexBuffers);
	glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffers[0]);
	glBufferData(GL_COPY_READ_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffers[1]);
	glBufferData(GL_COPY_READ_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(vertices) + sizeof(colors), nullptr, GL_STATIC_DRAW);
	
	GLint positionSize;
	glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffers[0]);
	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &positionSize);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertices));
	
	GLint colorSize;
	glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffers[1]);
	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &colorSize);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, positionSize, colorSize);

	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);

	//Pulling vertex data from the triangle buffer...
	GLfloat* verticesData = new GLfloat[2];
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2, verticesData);
	std::cout << "First Position In Buffer x: " << verticesData[0] << ", y:" << verticesData[1] << std::endl;
	delete[] verticesData;

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


	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(vertices)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
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
	window = glfwCreateWindow(640, 480, "Hello GLFW", NULL, NULL);
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

