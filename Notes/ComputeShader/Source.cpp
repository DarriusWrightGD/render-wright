#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <glm\glm.hpp>
#include <SOIL.h>
#include <glm\gtc\random.hpp>

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

void checkProgram(GLuint program)
{
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

}

GLfloat vertices[6][2]{
	{ -1.0f, -1.0f },
	{ 1.0f, -1.0f },
	{ -1.0f, 1.0f },
	{ 1.0f, -1.0f },
	{ 1.0f, 1.0f },
	{ -1.0f, 1.0f},
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
	GLuint quadVao;
	glGenVertexArrays(1, &quadVao);
	glBindVertexArray(quadVao);

	GLuint quadBuffer;
	glGenBuffers(1, &quadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	GLuint computeShader = createShader("computeShader.comp", GL_COMPUTE_SHADER);

	// the compute shader can only be by itself in a program
	// but can use all of the forms of storage that you are accustom to from other shaders/
	GLuint rayTraceProgram = glCreateProgram();
	glAttachShader(rayTraceProgram, computeShader);
	glLinkProgram(rayTraceProgram);
	checkProgram(rayTraceProgram);
	glDetachShader(rayTraceProgram, computeShader);
	glUseProgram(rayTraceProgram);

	GLuint vertexShader = createShader("vertexShader.vert", GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader("fragmentShader.frag", GL_FRAGMENT_SHADER);

	GLuint quadProgram = glCreateProgram();
	glAttachShader(quadProgram, vertexShader);
	glAttachShader(quadProgram, fragmentShader);
	glLinkProgram(quadProgram);
	checkProgram(quadProgram);
	glDetachShader(quadProgram, vertexShader);
	glDetachShader(quadProgram, fragmentShader);
	glUseProgram(quadProgram);


	
	GLint workGroupSizeX, workGroupSizeY, workGroupSizeZ;
	GLint workGroupInvocations;
	GLint workGroupSize[3];


	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizeZ);

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInvocations);
	glGetProgramiv(rayTraceProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);


	std::cout << "Work Group Size : ( " << workGroupSize[0] << " , " << workGroupSize[1] << " , " << workGroupSize[2] << " ) " << std::endl;
	std::cout << "Max Work Group Size : ( " << workGroupSizeX << " , " << workGroupSizeY << " , " << workGroupSizeZ << " ) " << std::endl;
	std::cout << "Max Work Invocations : ( " << workGroupInvocations << " ) " << std::endl;

	glUseProgram(rayTraceProgram);

	GLuint outputTexture;
	GLuint imageWidth = 1024, imageHeight = 768;
	glGenTextures(1, &outputTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outputTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	GLuint sphereBuffer;
	GLuint sphereTexture;
	const GLuint sphereCount = 10;

	glm::vec4 spheres[sphereCount];
	for (int i = 0; i < sphereCount; i++)
	{
		spheres[i] = glm::vec4(glm::sphericalRand(10.0f) + glm::vec3(0,0,-20), 10.0f);
	}

	glGenBuffers(1, &sphereBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, sphereBuffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof glm::vec4 * sphereCount, spheres, GL_STATIC_DRAW);

	glGenTextures(1, &sphereTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, sphereTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, sphereBuffer);
	glBindImageTexture(1, sphereTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	//GLubyte * pixels = new GLubyte[imageWidth * imageHeight * 3];
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	//SOIL_save_image("computeImage.bmp", SOIL_SAVE_TYPE_BMP, imageWidth, imageHeight, 3, pixels);

	//delete[] pixels;



	while (!glfwWindowShouldClose(window))
	{
		/*Compute shaders execute in work groups, and a call to the below function will cause a sinlge global work group to be sent
		to opengl the work group will then be divided into a number of local work groups.
		A work group is a 3D block of work items where ech work item is processed by an invocation of a compute shader running your code.
		//glDispatchCompute(32, 32, 1);
		*/
		glUseProgram(rayTraceProgram);
		glDispatchCompute(imageWidth / workGroupSize[0], imageHeight / workGroupSize[1], 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glUseProgram(quadProgram);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteShader(computeShader);
	glDeleteProgram(rayTraceProgram);
	glDeleteProgram(quadProgram);
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

