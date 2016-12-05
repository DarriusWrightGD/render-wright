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
#include <glm\gtc\random.hpp>
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


GLfloat vertices[6][3]{
	{ -1.0f, 1.0f ,-1.0f}, //1
	{ -1.0f, -1.0f,-1.0f }, //2
	{ 1.0f, 1.0f,-1.0f }, //3
	{ -1.0f, -1.0f,-1.0f }, //2
	{ 1.0f, -1.0f,-1.0f },//4
	{ 1.0f, 1.0f,-1.0f }, // 3
};

GLfloat uvs[6][2]{
	{ 0,1 },
	{ 0,0 },
	{ 1,1 },
	{ 0,0 },
	{ 1,0 },
	{ 1,1 },

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


	GLuint quadVertexArray;
	glGenVertexArrays(1, &quadVertexArray);
	glBindVertexArray(quadVertexArray);
	GLuint quadBuffer;
	glGenBuffers(1, &quadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof vertices, sizeof uvs, uvs);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	int imageWidth, imageHeight, channels;
	GLubyte * imageBytes = SOIL_load_image("../../Models/Dog/dogColor.png", &imageWidth, &imageHeight, &channels, SOIL_LOAD_AUTO);

	GLuint imageSize = sizeof(GLubyte) * imageWidth * imageHeight * channels;
	GLuint dogTexture;
	glGenTextures(1, &dogTexture);

	////common way of setting up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dogTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, (channels == 4) ? GL_RGBA8 : GL_RGB8, imageWidth, imageHeight);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight,
		(channels == 4) ? GL_RGBA : GL_RGB,
		GL_UNSIGNED_BYTE, 
		imageBytes);

	delete[] imageBytes;

	GLuint gbuffer;
	GLuint textureBuffers[4];

	glGenFramebuffers(1, &gbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);

	glGenTextures(4, textureBuffers);
	glBindTexture(GL_TEXTURE_2D, textureBuffers[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glBindTexture(GL_TEXTURE_2D, textureBuffers[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textureBuffers[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glBindTexture(GL_TEXTURE_2D, textureBuffers[3]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureBuffers[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textureBuffers[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, textureBuffers[2], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureBuffers[3], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint vertexShader = createShader("vertexShader.vert", GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader("fragmentShader.frag", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	checkProgram(program);

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glUseProgram(program);

	GLuint deferredVertexShader = createShader("deferredVertexShader.vert", GL_VERTEX_SHADER);
	GLuint deferredFragShader = createShader("deferredFragShader.frag", GL_FRAGMENT_SHADER);

	GLuint deferredProgram = glCreateProgram();
	glAttachShader(deferredProgram, deferredVertexShader);
	glAttachShader(deferredProgram, deferredFragShader);
	glLinkProgram(deferredProgram);

	checkProgram(deferredProgram);

	glDetachShader(deferredProgram, deferredVertexShader);
	glDetachShader(deferredProgram, deferredFragShader);
	glUseProgram(deferredProgram);

	auto viewLocation = glGetUniformLocation(deferredProgram, "view");
	auto projectionLocation = glGetUniformLocation(deferredProgram, "projection");
	auto modelIndex = glGetUniformLocation(deferredProgram, "model");
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // view
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width)/height, 0.1f, 1000.f);  // projection

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	glEnable(GL_DEPTH_TEST);
	glm::vec3 modelPosition = glm::vec3{ 0.0f,-8.0f,-150.0f };
	glm::mat4 model = glm::translate(glm::vec3{ 0.0f,-8.0f,-150.0f })
		* glm::rotate((float)sin(glfwGetTime()), glm::vec3(1.0f, 0.0f, 1.0f))  // model
		* glm::rotate((float)cos(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f)); // model

	glUseProgram(program);

	const int lightCount = 3;
	auto lightCountLocation = glGetUniformLocation(program, "lightCount");
	glUniform1i(lightCountLocation, lightCount);

	auto lightPositionsLocation = glGetUniformLocation(program, "lightPositions");
	auto lightColorsLocation = glGetUniformLocation(program, "lightColors");

	glm::vec3 lightPositions[lightCount];
	glm::vec3 lightColors[lightCount];

	for (size_t i = 0; i < lightCount; i++)
	{
		lightPositions[i] = glm::vec3(modelPosition + glm::sphericalRand(10.0f));
	}

	lightColors[0] = glm::vec3(1, 0, 0);
	lightColors[1] = glm::vec3(0, 1, 0);
	lightColors[2] = glm::vec3(0, 0, 1);

	glUniform3fv(lightPositionsLocation, 3, &lightPositions[0].x);
	glUniform3fv(lightColorsLocation,3, &lightColors[0].x);


	glEnable(GL_DEPTH_TEST);
	//glClearColor(1, 1, 0, 1);

	while (!glfwWindowShouldClose(window))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
		GLenum drawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, drawBuffers);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(deferredProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dogTexture);
		glBindVertexArray(dogVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, dogBuffer);

		model = glm::translate(glm::vec3{ 0.0f,-8.0f,-250.0f })
			* glm::rotate((float)sin(glfwGetTime()), glm::vec3(1.0f, 0.0f, 1.0f))  // model
			* glm::rotate((float)cos(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f)); // model
		glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
		
		glUseProgram(program);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureBuffers[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureBuffers[2]);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
		glBindVertexArray(quadVertexArray);

		glDrawArrays(GL_TRIANGLES, 0, 6);

#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &dogTexture);
	glDeleteTextures(4, textureBuffers);
	glDeleteFramebuffers(1, &gbuffer);
	glDeleteBuffers(1, &dogBuffer);
	glDeleteBuffers(1, &quadBuffer);
	//glDeleteBuffers(1, &unpackBuffer);
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
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	window = glfwCreateWindow(580, 720, "Hello Deferred Shading Loading", NULL, NULL);
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

