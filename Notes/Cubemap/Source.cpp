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
#include <string>
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

struct MeshInfo
{
	GLuint vertexCount;
	GLuint indexCount;
	GLuint vertexOffset;
	GLuint indexOffset;
};

struct MeshBuffer
{
	GLuint vertexBuffer;
	GLuint indexBuffer;
	vector<MeshInfo> meshInfos;
};

MeshBuffer addMeshes(vector<aiMesh*> meshes)
{
	GLuint indexCount = 0;
	GLuint vertexCount = 0;

	vector<MeshInfo> meshInfos;

	for(const auto mesh : meshes)
	{
		MeshInfo info;
		info.vertexOffset = vertexCount;
		info.indexOffset = indexCount;
		indexCount += mesh->mNumFaces * 3;
		vertexCount += mesh->mNumVertices;
		info.indexCount = mesh->mNumFaces * 3;
		info.vertexCount = mesh->mNumVertices;

		meshInfos.push_back(info);
	}

	GLuint positionSize = sizeof(float) * 3;
	GLuint normalSize = sizeof(float) * 3;
	GLuint uvSize = sizeof(float) * 2;

	GLuint positionOffset = positionSize * vertexCount;
	GLuint normalOffset = positionOffset + normalSize * vertexCount;
	GLuint uvOffset = normalOffset + uvSize * vertexCount;
	GLuint bufferSize = uvOffset;

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

	for (GLuint meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
	{
		GLuint meshPositionOffset = meshInfos[meshIndex].vertexOffset * positionSize;
		GLuint meshNormalOffset = positionOffset + meshInfos[meshIndex].vertexOffset * normalSize;
		GLuint meshUvOffset = normalOffset + meshInfos[meshIndex].vertexOffset * uvSize;

		glBufferSubData(GL_ARRAY_BUFFER, meshPositionOffset, positionSize * meshInfos[meshIndex].vertexCount, meshes[meshIndex]->mVertices);
		glBufferSubData(GL_ARRAY_BUFFER, meshNormalOffset, normalSize * meshInfos[meshIndex].vertexCount, meshes[meshIndex]->mNormals);

		auto uvData = reinterpret_cast<char*>(glMapBufferRange(GL_ARRAY_BUFFER, meshUvOffset, uvSize * meshInfos[meshIndex].vertexCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
		for (GLuint i = 0; i < meshInfos[meshIndex].vertexCount; i++)
		{
			auto offset = uvSize * i;
			memcpy(uvData + offset, &meshes[meshIndex]->mTextureCoords[0][i], uvSize);
			glFlushMappedBufferRange(GL_ARRAY_BUFFER, offset, uvSize);
		}

		glFinish();
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(positionOffset));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(normalOffset));
	glEnableVertexAttribArray(2);

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexCount, nullptr, GL_STATIC_DRAW);
	auto indexData = reinterpret_cast<char*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indexCount, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

	GLuint currentIndex = 0;
	GLuint face[3];

	for (GLuint meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
	{
		for (GLuint i = 0; i < meshInfos[meshIndex].indexCount/3; i++)
		{
			auto offset = sizeof(GLuint) * 3 * currentIndex;
			auto copySize = sizeof(GLuint) * 3;
			face[0] = meshes[meshIndex]->mFaces[i].mIndices[0] + meshInfos[meshIndex].indexOffset;
			face[1] = meshes[meshIndex]->mFaces[i].mIndices[1] + meshInfos[meshIndex].indexOffset;
			face[2] = meshes[meshIndex]->mFaces[i].mIndices[2] + meshInfos[meshIndex].indexOffset;

			memcpy(indexData + offset, face, copySize);
			glFlushMappedBufferRange(GL_ELEMENT_ARRAY_BUFFER, offset, copySize);
			currentIndex++;
		}
	}
	glFinish();
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	auto data = reinterpret_cast<GLuint*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indexCount, GL_MAP_READ_BIT));
	/*for (GLuint i = 0; i < indexCount; i++) 
		std::cout << data[i] << std::endl;*/

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	return {buffer,indexBuffer,meshInfos};
}

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
	const aiScene * scene = importer.ReadFile("../../Models/Cube/cube.obj",
		aiProcess_Triangulate
	);

	if (!scene) {
		std::cout << "Loading scene failed" << std::endl;
		exit(-1);
	}


	//int vertexCount = 0;
	Assimp::Importer bearImporter;
	const aiScene * bear = bearImporter.ReadFile("../../Models/Bear/bear-obj.obj", aiProcess_Triangulate);
	GLuint cubeVertexArray;
	glGenVertexArrays(1, &cubeVertexArray);
	glBindVertexArray(cubeVertexArray);


	aiMesh* mesh = scene->mMeshes[0];
	aiMesh* bearMesh = bear->mMeshes[0];
	auto meshBuffer = addMeshes({ mesh,bearMesh });

	int imageWidth, imageHeight, channels;

	std::string cubeMapPath = "../../Models/Cube/Outdoors/";
	std::vector<std::string> cubeMapFiles = { "posx.jpg","negx.jpg" ,"posy.jpg" ,"negy.jpg" ,"posz.jpg" ,"negz.jpg" };



	GLuint cubeTexture;
	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, 2048, 2048);

	GLuint mapIndex = 0;
	for (auto fileName : cubeMapFiles)
	{
		auto newPath = cubeMapPath + fileName;
		int mapWidth, mapHeight, channels;
		GLubyte * imageData = SOIL_load_image(newPath.c_str(), &mapWidth, &mapHeight, &channels, SOIL_LOAD_AUTO);
		GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + mapIndex;
		glTexSubImage2D(target, 0, 0, 0, mapWidth, mapHeight, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		mapIndex++;
	}



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

	auto envLocation = glGetUniformLocation(program, "environmentMap");


	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		glUniform1i(envLocation, GL_FALSE);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model = glm::translate(glm::vec3{ 0.0f,0.0f,-10.0f })
			* glm::scale(glm::vec3{ 100.0f, 100.0f,100.0f })
			* glm::rotate(glm::radians((float)glfwGetTime()* 20.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // model
		glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);

		auto cubeInfo = meshBuffer.meshInfos[0];
		glDrawElements(GL_TRIANGLES, cubeInfo.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(cubeInfo.indexOffset));
		
		glUniform1i(envLocation, GL_TRUE);
		model = glm::translate(glm::vec3{ 0.0f,-0.5f,-5.0f })
			* glm::scale(glm::vec3{ 0.1f, 0.1f,0.1f })
			* glm::rotate(glm::radians((float)glfwGetTime()* 20.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // model
		glUniformMatrix4fv(modelIndex, 1, GL_FALSE, &model[0][0]);

		auto bearInfo = meshBuffer.meshInfos[1];
		//glDrawElements(GL_TRIANGLES, bearInfo.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(bearInfo.indexOffset * sizeof(GLuint)));
		GLuint skip = sizeof(GLuint) * 36;
		glDrawElements(GL_TRIANGLES, bearInfo.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(skip));


#ifndef NDEBUG 
		glFinish();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &cubeTexture);
	glDeleteBuffers(1, &meshBuffer.indexBuffer);
	//glDeleteBuffers(1, &unpackBuffer);
	glDeleteBuffers(1, &meshBuffer.vertexBuffer);
	glDeleteVertexArrays(1, &cubeVertexArray);
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

