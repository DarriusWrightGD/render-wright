#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <fstream>
#include <QtGui\qimage.h>

using namespace std;

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
	{ -0.9, -0.9, 0.1 },
	{ 0.85, -0.9, 0.1 },
	{ -0.9, 0.85, 0.1 },
	{ 0.9, -0.85, -0.1 },
	{ 0.9, 0.9, -0.1 },
	{ -0.85, 0.9, -0.1 },
};

void checkFrameBufferError(GLenum status) {
	switch (status)
	{

	case GL_FRAMEBUFFER_UNSUPPORTED:
		cout << "GL_FRAMEBUFFER_UNSUPPORTED" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << endl;
		break;
	case GL_FRAMEBUFFER_COMPLETE:
		cout << "Completed!" << endl;
		break;
	default:
		cout << "Unknown error" << endl;
	}
}

void saveImage(const char * filename, int width, int height) {
	unsigned int channels = 4;
	GLubyte * pixels = new GLubyte[width * height * channels];
	// reading from a multisampled fbo doesn't work use this (http://stackoverflow.com/questions/765434/glreadpixels-from-fbo-fails-with-multisampling)
	// or see below
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	QImage image(pixels, width, height, QImage::Format::Format_RGBA8888);
	image = image.mirrored();
	image.save(filename);
	delete[] pixels;

	cout << "Created " << filename << " Image!" << endl;
}

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	//Framebuffers
	/*
	These are most useful for doing off screen rendering, updating texture maps, and GPGPU
	*/

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // Can be READ or DRAW, DRAW = GL_FRAMEBUFFER
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
	//glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples); 

	GLuint renderbuffers[2];
	glGenRenderbuffers(2, renderbuffers);

	//color
	glBindRenderbuffer(GL_RENDERBUFFER, // must be renderbuffer
		renderbuffers[0]); // render buffer to bind to 
	//before attaching a render buffer to a frame buffer it needs to have it's storage setup
	glRenderbufferStorage(GL_RENDERBUFFER, // must be renderbuffer 
		GL_RGBA, //buffer format, in this case GL_RGBA for color
		width, height); //dimensions 

	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[1]); // depth/stencil
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffers[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffers[1]);
	
	checkFrameBufferError(glCheckFramebufferStatus(GL_FRAMEBUFFER));

	//multisample fbo 
	GLuint samples = 8;
	GLuint msFramebuffer;
	glGenFramebuffers(1, &msFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, msFramebuffer);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples);

	GLuint msRenderbuffers[2];
	glGenRenderbuffers(2, msRenderbuffers);

	glBindRenderbuffer(GL_RENDERBUFFER, msRenderbuffers[0]);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA, width, height);

	glBindRenderbuffer(GL_RENDERBUFFER, msRenderbuffers[1]);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_STENCIL, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msRenderbuffers[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msRenderbuffers[1]);

	checkFrameBufferError(glCheckFramebufferStatus(GL_FRAMEBUFFER));

	////framebuffer offset example
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glViewport(0, 0, width, height);
	//glClearColor(0, .7, 0.2, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer); // read from the system framebuffer
	//glBlitFramebuffer(0, 0, width * 0.8f, height * 0.8f, 0, 0, width * 0.8f, height * 0.8f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	//saveImage("triangleOffset.png", width, height);

	////framebuffer draw example
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	//glViewport(0, 0, width, height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//saveImage("triangle.png", width, height);

	//framebuffer multisample example
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFramebuffer);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, msFramebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	saveImage("triangleMultisample.png", width, height);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1.0);

	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);

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

	glDeleteRenderbuffers(2, renderbuffers);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteRenderbuffers(2, msRenderbuffers);
	glDeleteFramebuffers(1, &msFramebuffer);
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

