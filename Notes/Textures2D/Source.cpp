#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <vector>
#include <SOIL.h>

using std::vector;

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

struct Vertex {
	glm::vec3 position;
};

int main() {
	init();

	/*
	Any OpenGL program consists of the following steps...
	1. Specify the data for constructing shapes from OpenGLs primitives
	2. Execute shaders
	3. Convert the input data into fragments
	4. Perform additional per-fragment operations.
	*/

	Assimp::Importer importer;
	const aiScene * scene = importer.ReadFile("../../Models/Dog/dog.obj",
		aiProcess_Triangulate
	);

	if (!scene) {
		std::cout << "Loading scene failed" << std::endl;
		exit(-1);
	}


	//int vertexCount = 0;

	GLuint bearVertexArray;
	glGenVertexArrays(1, &bearVertexArray);
	glBindVertexArray(bearVertexArray);

	aiMesh* mesh = scene->mMeshes[0];

	GLuint faceCount = mesh->mNumFaces;
	GLuint indexCount = faceCount * 3;
	GLuint vertexCount = mesh->mNumVertices;

	GLuint positionSize = sizeof(float) * 3;
	GLuint normalSize = sizeof(float) * 3;
	GLuint uvSize = sizeof(float) * 2;

	GLuint positionOffset = positionSize * vertexCount;
	GLuint normalOffset = positionOffset + normalSize * vertexCount;
	GLuint uvOffset = normalOffset + uvSize * vertexCount;

	GLuint bearBuffer; // position, normal, texture
	glGenBuffers(1, &bearBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bearBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvOffset, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, positionOffset, mesh->mVertices);
	glBufferSubData(GL_ARRAY_BUFFER, positionOffset, normalSize * vertexCount, mesh->mNormals);

	auto uvData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, normalOffset, uvSize * vertexCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
	for (GLuint i = 0; i < vertexCount; i++)
	{
		auto offset = uvSize * i;
		memcpy(uvData + offset, &mesh->mTextureCoords[0][i], uvSize);
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, offset, uvSize);
	}

	glFinish();
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(positionOffset));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(normalOffset));
	glEnableVertexAttribArray(2);

	GLuint indexSize = sizeof(mesh->mFaces[0].mIndices[0]);
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, nullptr, GL_STATIC_DRAW);
	auto indexData = reinterpret_cast<char*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, indexSize * indexCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
	for (GLuint i = 0; i < faceCount; i++)
	{
		auto offset = indexSize * 3 * i;
		auto copySize = indexSize * 3;
		memcpy(indexData + offset, mesh->mFaces[i].mIndices, copySize);
		glFlushMappedBufferRange(GL_ELEMENT_ARRAY_BUFFER, offset, copySize);
	}
	glFinish();
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	// Creating textures
	/*The nice thing about texture objects in opengl is that they are not limited to only images, they can be 
	just a generic way of storing data for use in shaders. Generally for use with GPGPU*/
	// Checkout proxy texture targets to know if you have the space to create a texture.
	int imageWidth, imageHeight, channels;
	GLubyte * imageBytes = SOIL_load_image("../../Models/Dog/dogColor.png",&imageWidth, &imageHeight, &channels, SOIL_LOAD_AUTO);
	
	GLuint imageSize = sizeof(GLubyte) * imageWidth * imageHeight * channels;
	GLuint bearTexture;
	glGenTextures(1, &bearTexture);

	////common way of setting up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bearTexture);
	/*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/
	glTexStorage2D(GL_TEXTURE_2D, 1, (channels == 4) ? GL_RGBA8 : GL_RGB8, imageWidth, imageHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, 
		(channels == 4) ? GL_RGBA : GL_RGB, // format GL_RED - GL_RGBA as well as integer equivalents which allow you to take in data exactly as presented in the texture.
		// this could be useful with ray tracing to transfer data around while maintaining values.
		GL_UNSIGNED_BYTE, // the type of the texture data? FLOAT, INT, BYTE, DOUBLE etc
		imageBytes);

	//glGetTexImage() //to get texture pixels

	//or from another buffer, the advantage of this method is that since it comes from a buffer the transfer of data can happen in parallel 
	// where as the above method will wait until the transfer is completed. The advantage of the above method is that you can count on the 
	// data being there and you can preform changes on it at will.
	//GLuint unpackBuffer;
	//glGenBuffers(1, &unpackBuffer);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
	//glBufferData(GL_PIXEL_UNPACK_BUFFER, imageSize, imageBytes, GL_STATIC_DRAW);
	//glBindTexture(GL_TEXTURE_2D, bearTexture);
	//glActiveTexture(GL_TEXTURE0);
	//glTexStorage2D(GL_TEXTURE_2D, 1, (channels == 4) ? GL_RGBA8 : GL_RGB8, imageWidth, imageHeight);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, (channels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	delete[] imageBytes;

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

	auto vpIndex = glGetUniformLocation(program, "viewProjection");
	auto modelIndex = glGetUniformLocation(program, "model");
	glm::mat4 vp = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.f) // projection
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));  // view

	glUniformMatrix4fv(vpIndex, 1, GL_FALSE, &vp[0][0]);

	glEnable(GL_DEPTH_TEST);
	//copying from a framebuffers to be used
	glClearColor(0.2, 0.4, 0.7, 1);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 model = glm::translate(glm::vec3{ 0.0f,-8.0f,-150.0f })
		* glm::rotate((float)sin(glfwGetTime()), glm::vec3(1.0f, 0.0f, 1.0f))  // model
		* glm::rotate((float)cos(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f)); // model
	glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
	glFinish();

	GLuint fboTexture;
	glGenTextures(1, &fboTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	 	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);

	glClearColor(1, 1, 1, 1);

	while (!glfwWindowShouldClose(window))
	{
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, bearTexture);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, fboTexture);
		//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 512, 512, 0);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model = glm::translate(glm::vec3{ 0.0f,-8.0f,-250.0f }) 
			* glm::rotate((float)sin(glfwGetTime()), glm::vec3(1.0f, 0.0f, 1.0f))  // model
			* glm::rotate((float)cos(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f)); // model
		glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));



#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &bearTexture);
	glDeleteBuffers(1, &bearBuffer);
	//glDeleteBuffers(1, &unpackBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &bearVertexArray);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
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
	if(GL_DEBUG_TYPE_ERROR == type)
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
	window = glfwCreateWindow(580, 720, "Hello Model Loading", NULL, NULL);
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

