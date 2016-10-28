#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLFWwindow* window;
int width, height;

void init();

GLfloat vertices[6][2]{
	{ -0.9, -0.9 },
	{ 0.85, -0.9 },
	{ -0.9, 0.85 },
	{ 0.9, -0.85 },
	{ 0.9, 0.9 },
	{ -0.85, 0.9},
};

const char * vertexShaderCode =
"#version 440 core\n"
"layout (location = 0) in vec4 vPosition;\n"
"\n"
"void main(){\n"
"gl_Position = vPosition;\n"
"}\n"
""
;

const char * fragmentShaderCode =
"#version 440 core\n"
"out vec4 fColor;"
"\n"
"void main() {\n"
"fColor = vec4(0,0,1,1);\n"
"}\n"
""
;

int main() {
	init();
	GLuint triangleVertexArray;
	glGenVertexArrays(1, &triangleVertexArray);
	glBindVertexArray(triangleVertexArray);

	GLuint triangleBuffer;
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
	glCompileShader(fragmentShader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glUseProgram(program);


	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(triangleVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glFinish();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();
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
	window = glfwCreateWindow(640, 480, "Hello Glfw", NULL, NULL);
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
	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << " loaded";
}

