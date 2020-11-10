#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include "stl.h"

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

/* PARTICULES */
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 velocity;
	float size;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0, 1);
	std::uniform_real_distribution<float> distributionWorld(-1, 1);

	std::vector<Particule> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particule{
				{
				distributionWorld(generator),
				distributionWorld(generator),
				distributionWorld(generator)
				},
				{
				distribution01(generator),
				distribution01(generator),
				distribution01(generator)
				},
				{ 0., 0., 0. },
				{ distribution01(generator) * 50 }
				});
	}

	return p;
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

	//const size_t nParticules = 100;
	//auto particules = MakeParticules(nParticules);

	// Load logo
	std::vector<Triangle> triangles = ReadStl("buddha.stl");
	std::vector<glm::vec3> vertices;
	for (int i = 0; i < triangles.size(); i++) {
		vertices.push_back(triangles[i].p0);
		vertices.push_back(triangles[i].p1);
		vertices.push_back(triangles[i].p2);
	}

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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	// Bindings
	const auto indexPosition = glGetAttribLocation(program, "position");
	glVertexAttribPointer(indexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
	glEnableVertexAttribArray(indexPosition);
	
	/*
	const auto indexColor = glGetAttribLocation(program, "color");
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<GLvoid*>(offsetof(Particule, color)));
	glEnableVertexAttribArray(indexColor);
	
	const auto indexSize = glGetAttribLocation(program, "size");
	glVertexAttribPointer(indexSize, 1, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<GLvoid*>(offsetof(Particule, size)));
	glEnableVertexAttribArray(indexSize);
	*/

	glPointSize(20.f);

	glEnable(GL_PROGRAM_POINT_SIZE);

	while (!glfwWindowShouldClose(window)) {
		int width, height;
		/*double xpos, ypos;
		float u_time = glfwGetTime();


		glfwGetCursorPos(window, &xpos, &ypos);
		xpos = xpos / width;
		ypos = ypos / height;

		glUniform1f(glGetUniformLocation(program, "u_time"), u_time);
		glUniform2d(glGetUniformLocation(program, "mousePos"), xpos, ypos);
		*/

		/*for (int i = 0; i < particules.size(); i++) {
			particules[i].velocity += glm::vec3(0., -0.001, 0.) * u_time;
			particules[i].position += particules[i].velocity * u_time;
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, nParticules * sizeof(Particule), particules.data());*/

		glClear(GL_COLOR_BUFFER_BIT);

		//glDrawArrays(GL_POINTS, 0, nParticules);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
