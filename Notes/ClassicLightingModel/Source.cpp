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

	GLuint dogVertexArray;
	glGenVertexArrays(1, &dogVertexArray);
	glBindVertexArray(dogVertexArray);

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

	GLuint dogBuffer; // position, normal, texture
	glGenBuffers(1, &dogBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, dogBuffer);
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
	GLuint dogTexture;
	glGenTextures(1, &dogTexture);

	////common way of setting up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dogTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, (channels == 4) ? GL_RGBA8 : GL_RGB8, imageWidth, imageHeight);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, (channels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imageBytes);

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
	
	//the transpose inverse of the model view matrix
	auto normalMatrixIndex = glGetUniformLocation(program, "normalMatrix");
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));  // view
	glm::mat4 vp = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.f) // projection
		* view;

	glUniformMatrix4fv(vpIndex, 1, GL_FALSE, &vp[0][0]);

	auto ambientLocation = glGetUniformLocation(program, "ambient");
	auto diffuseLocation = glGetUniformLocation(program, "diffuse");
	auto specularLocation = glGetUniformLocation(program, "specular");
	auto specularStrengthLocation = glGetUniformLocation(program, "specularStrength");
	auto lightDirectionLocation = glGetUniformLocation(program, "lightDirection");
	auto eyeDirectionLocation = glGetUniformLocation(program, "eyeDirection");
	
	auto lightingSubroutineLocation = glGetSubroutineUniformLocation(program, GL_FRAGMENT_SHADER, "lighting");
	auto ambientSubroutineIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "ambientLighting");
	auto diffuseSubroutineIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "diffuseLighting");
	auto directionalSubroutineIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "directionalLighting");
	
	GLuint selectedSubroutine [] = { directionalSubroutineIndex };

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, selectedSubroutine);

	/* ambient light is not light from any particular direction, but rather is a constant light that exists throughout
	the scene.*/
	glm::vec3 ambient(0.7, 0.7, 0.7);
	glUniform3fv(ambientLocation, 1, &ambient[0]);

	/*Diffuse light is scattered by the surface in all directions equally. It doesn't matter what the direciton of
	the eye is, but rather what the direction of the light is. Diffuse light computation depends on the surface normal, 
	the direction of the light, and the color of the surface*/
	//glm::vec3 diffuse(0.5, 0.6, 0.9);
	glm::vec3 diffuse(1);
	glUniform3fv(diffuseLocation, 1, &diffuse[0]);
	glm::vec4 lightDirection = glm::vec4(glm::normalize(glm::vec3(0.5, 0.3, -1.0)),1.0);
	glUniform4fv(lightDirectionLocation, 1, &lightDirection[0]);

	/*Specular light is light that is reflected directly by the surface, this highlighting is related to how much the 
	surface acts like a mirror.*/
	glm::vec4 specular(.8f, .8f, .8f, 0.5f);
	glUniform4fv(specularLocation, 1, &specular[0]);
	glm::vec4 eyeDirection = glm::vec4(glm::normalize(glm::vec3(-0.1, -0.1, -1)), 1.0);
	glUniform4fv(eyeDirectionLocation, 1, &eyeDirection[0]);
	glUniform1f(specularStrengthLocation, 10.0f);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 1);
	glm::mat4 model;
	glm::mat3 normalMatrix;
	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model = glm::translate(glm::vec3{ 0.0f,-8.0f,-250.0f }) 
			* glm::rotate((float)sin(glfwGetTime()), glm::vec3(1.0f, 0.0f, 1.0f))
			* glm::rotate((float)cos(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		
		normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * model)));
		glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(normalMatrixIndex, 1, GL_FALSE, &normalMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &dogTexture);
	glDeleteBuffers(1, &dogBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &dogVertexArray);
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
	glfwWindowHint(GLFW_SAMPLES, 4);
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

