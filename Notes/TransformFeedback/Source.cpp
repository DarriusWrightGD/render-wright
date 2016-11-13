#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <glm\mat4x4.hpp>
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

void checkProgramStatus(GLuint program) {
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
	{ -0.9, -0.9 },
	{ 0.85, -0.9 },
	{ -0.9, 0.85 },
	{ 0.9, -0.85 },
	{ 0.9, 0.9 },
	{ -0.85, 0.9},
};

int main() {
	init();

	// Compile shaders
	GLuint vertexShader = createShader("vertexShader.vert", GL_VERTEX_SHADER);

	// Create program and specify transform feedback variables
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);

	const GLchar* feedbackVaryings[] = { "position" };
	glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(program);
	glUseProgram(program);

	GLuint getError = glGetError();

	// Create VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create input VBO and vertex format
	GLfloat data[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);

	// Create transform feedback buffer
	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data) * 3, nullptr, GL_STATIC_READ);
	getError = glGetError();
	// Create query object to collect info
	GLuint query;
	glGenQueries(1, &query);
	getError = glGetError();
	// Perform feedback transform
	glEnable(GL_RASTERIZER_DISCARD);
	getError = glGetError();
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
	getError = glGetError();
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	getError = glGetError();
	glDrawArrays(GL_POINTS, 0, 5);
	glEndTransformFeedback();
	getError = glGetError();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	getError = glGetError();
	glDisable(GL_RASTERIZER_DISCARD);
	getError = glGetError();
	glFlush();

	// Fetch and print results
	GLuint primitives;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

	GLfloat feedback[15];
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
	getError = glGetError();
	printf("%u primitives written!\n\n", primitives);

	for (int i = 0; i < 15; i++) {
		printf("%f\n", feedback[i]);
	}

	glDeleteQueries(1, &query);
	getError = glGetError();
	glDeleteProgram(program);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &tbo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
	getError = glGetError();
	system("pause");

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
	window = glfwCreateWindow(640, 480, "Hello Transform Feedback", nullptr, nullptr);
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
