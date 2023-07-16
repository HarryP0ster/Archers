#pragma once
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#include <glad\glad.h> 
#include <glfw3.h>
#include "FileManager.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
};

struct Mesh
{
	Mesh() = default;
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
	Mesh(const Mesh& other);
	~Mesh() {};

	size_t NumVerts()
	{
		return verts.size();
	}

	size_t NumIndices()
	{
		return inds.size();
	}

	const Vertex* Vertices()
	{
		return verts.data();
	}

	const uint32_t* Indices()
	{
		return inds.data();
	}

	void calculate_normals();
	void BindBuffers();
	void ClearBinds();

private:
	std::vector<Vertex> verts;
	std::vector<uint32_t> inds;
	uint32_t VBO;
	uint32_t VIO;
	uint32_t VAO;
};

class OpenGLAPI
{
public:
	static bool GLInit(GLFWwindow** outWindow, int window_width, int window_height, const char* app_name);
	static bool GLCompileShader(const char* shader_path, unsigned int type, unsigned int program);
	static Mesh GenerateSphereMesh(float radius, int rings, int slices);
};