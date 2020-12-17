#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include "stl.h"
#include "texture.h"
#include "OBJ_Loader.h"

#define TINYPLY_IMPLEMENTATION
//#include <tinyply.h>

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	// Load mesh
	objl::Loader loaderMonstre;
	loaderMonstre.LoadFile("Monstre.obj");
	size_t nIndices = loaderMonstre.LoadedMeshes[0].Indices.size();
	size_t nVertices = loaderMonstre.LoadedMeshes[0].Vertices.size();

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");

	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);

	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nVertices * sizeof(objl::Vertex), loaderMonstre.LoadedMeshes[0].Vertices.data(), GL_STATIC_DRAW);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(unsigned int), loaderMonstre.LoadedMeshes[0].Indices.data(), GL_STATIC_DRAW);

	// Bindings
	const auto indexPosition = glGetAttribLocation(program, "position");
	glVertexAttribPointer(indexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), reinterpret_cast<GLvoid*>(offsetof(objl::Vertex, Position)));
	glEnableVertexAttribArray(indexPosition);

	const auto indexUv = glGetAttribLocation(program, "textCoord");
	glVertexAttribPointer(indexUv, 2, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), reinterpret_cast<GLvoid*>(offsetof(objl::Vertex, TextureCoordinate)));
	glEnableVertexAttribArray(indexUv);


	const auto indexNormal = glGetAttribLocation(program, "normal_2");
	std::cout << "indexNormal : " << indexNormal << "\n";
	glVertexAttribPointer(indexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), reinterpret_cast<GLvoid*>(offsetof(objl::Vertex, Normal)));
	glEnableVertexAttribArray(indexNormal);
	
	glPointSize(20.f);

	glEnable(GL_PROGRAM_POINT_SIZE);

	// Load Images
	Image img1 = LoadImage("textures/demondays.ppm");

	// Create a texture
	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGB8, img1.width, img1.height);

	glTextureSubImage2D(tex, 0, 0, 0, img1.width, img1.height, GL_RGB, GL_UNSIGNED_BYTE, img1.data.data());
	glBindTextureUnit(1, tex);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		float u_time = glfwGetTime();
		int width, height;

		glm::vec3 lightPosition = glm::vec3(0, 500, 1333);

		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0., 0., width, height);

		glm::mat4 view = glm::lookAt(
			glm::vec3(-200 + 333 * sin(u_time), 800 + 333 * cos(u_time), 1000),
			glm::vec3(-200, 500, 0),
			glm::vec3(0, 1, 0)
		);

		auto proj = glm::perspective(glm::radians(45.f), ((float)width) / height, 1.f, 500000.f);

		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

		glUniform1i(glGetUniformLocation(program, "tex"), 1);
		glUniform1f(glGetUniformLocation(program, "u_time"), u_time);

		glUniform3f(glGetUniformLocation(program, "lightPosition"), lightPosition.x, lightPosition.y, lightPosition.z);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
