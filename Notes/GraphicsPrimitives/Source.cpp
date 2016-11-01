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

GLenum drawMode = GL_TRIANGLES;
GLenum drawPolygonMode = GL_FILL;
GLFWwindow* window;
int width, height;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_1) drawMode = GL_TRIANGLES;
	if (key == GLFW_KEY_2) drawMode = GL_TRIANGLE_FAN;
	if (key == GLFW_KEY_3) drawMode = GL_TRIANGLE_STRIP;
	if (key == GLFW_KEY_4) drawMode = GL_POINTS;
	if (key == GLFW_KEY_5) drawMode = GL_LINES; // the rule for line rasterization is known as the diamond rule, check the opengl spec.
	if (key == GLFW_KEY_6) drawMode = GL_LINE_LOOP;
	if (key == GLFW_KEY_7) drawMode = GL_LINE_STRIP;

	if (key == GLFW_KEY_Q) drawPolygonMode = GL_LINE;
	if (key == GLFW_KEY_W) drawPolygonMode = GL_POINT;
	if (key == GLFW_KEY_E) drawPolygonMode = GL_FILL;


	std::cout << "Draw Mode : " << drawMode << " Polygon Mode : " << drawPolygonMode << std::endl;
}

static void resize_callback(GLFWwindow * window, int w, int h) {
	width = w;
	height = h;
}

void init();



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


/// <summary>
/// The purpose of OpenGL is to render graphics into a framebuffer.
/// The primitive types that OpenGL supports are points, lines, triangles, fans, loops, patches
/// </summary>
/// <returns></returns>
int main() {
	init();

	GLuint triangleVertexArray;
	glGenVertexArrays(1, &triangleVertexArray);
	glBindVertexArray(triangleVertexArray);

	GLuint triangleBuffer;
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), nullptr, 
		GL_STATIC_DRAW // Can be 
					   // STATIC - contents will be written once and used many times  
					   // DYNAMIC - contents will be written more than once and used many times
					   // STREAM - contents will be written once and used a few times
					   // DRAW - source of drawing and image specification commands (used for buffers containing vertex data)
					   // READ - contents are modified by reading from reading from OpenGL (used to read and setup Pixel buffer objects)
					   // COPY - contents are modified by reading from OpenGL and used for drawing and image specifications (Transform feedback buffers and then vertex buffers)
	);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	// A buffer can be cleared by using glClearBufferData, and glClearBufferSubData
	// Data can also be copied between buffers with the glCopyBufferSubData function, you must assign a GL_COPY_READ_BUFFER and GL_COPY_WRITE_BUFFER

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



	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glPointSize(10); // this can be assigned in the shader as well through enabling GL_PROGRAM_POINT_SIZE
	glLineWidth(3); //there is no shader equivalent for lines
	/*By default OpenGL sets the front face of polygons to be GL_CCW (counter clockwise),
	Using glFrontFace we can change that between GL_CCW and GL_CW (clockwise)*/

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	while (!glfwWindowShouldClose(window))
	{
		glPolygonMode(GL_FRONT_AND_BACK, drawPolygonMode);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(drawMode, 0, 6);

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
		glfwTerminate();
		exit(-1);
	}

	glfwSetErrorCallback(error_callback);


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	window = glfwCreateWindow(640, 480, "Hello GLFW", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resize_callback);
	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << " loaded" << std::endl;
}

