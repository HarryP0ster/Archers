#include "GLAPI.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
{
	verts.resize(vertices.size());
	inds.resize(indices.size());
	std::copy(vertices.begin(), vertices.end(), verts.begin());
	std::copy(indices.begin(), indices.end(), inds.begin());

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VIO);
	glGenVertexArrays(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(uint32_t), inds.data(), GL_STATIC_DRAW);
}

Mesh::Mesh(const Mesh& other)
{
	verts.resize(other.verts.size());
	inds.resize(other.inds.size());
	std::copy(other.verts.begin(), other.verts.end(), verts.begin());
	std::copy(other.inds.begin(), other.inds.end(), inds.begin());
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VIO);
	glGenVertexArrays(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(uint32_t), inds.data(), GL_STATIC_DRAW);
}

void Mesh::calculate_normals()
{
	for (int i = 0; i < inds.size(); i += 3)
	{
		glm::vec3 A = verts[inds[i]].pos;
		glm::vec3 B = verts[inds[i + 1]].pos;
		glm::vec3 C = verts[inds[i + 2]].pos;

		glm::vec3 contributingNormal = glm::cross(B - A, C - A);
		float area = glm::length(contributingNormal) / 2.f;
		contributingNormal = glm::normalize(contributingNormal) * area;

		verts[inds[i]].normal = verts[inds[i]].normal + contributingNormal;
		verts[inds[i + 1]].normal = verts[inds[i + 1]].normal + contributingNormal;
		verts[inds[i + 2]].normal = verts[inds[i + 2]].normal + contributingNormal;
	}

	for (int i = 0; i < verts.size(); i++)
	{
		verts[i].normal = glm::normalize(verts[i].normal);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

void Mesh::BindBuffers()
{
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);


	GLintptr vertex_position_offset = offsetof(Vertex, pos);
	GLintptr vertex_normal_offset = offsetof(Vertex, normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)vertex_position_offset);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)vertex_normal_offset);
}

void Mesh::ClearBinds()
{
	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

bool OpenGLAPI::GLInit(GLFWwindow** outWindow, int window_width, int window_height, const char* app_name)
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    *outWindow = glfwCreateWindow(window_width, window_height, app_name, NULL, NULL);

    if (!outWindow)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*outWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    glViewport(0, 0, window_width, window_height);

    return true;
}

bool OpenGLAPI::GLCompileShader(const char* shader_path, unsigned int type, unsigned int program)
{
    unsigned int shader;
    shader = glCreateShader(type);

    auto shaderSource = FileManager::ReadFile(shader_path);
    const char* shaderCode = shaderSource.data();
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);

    int  success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    return success;
}

Mesh OpenGLAPI::GenerateSphereMesh(float radius, int rings, int slices)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	float x, y, z, xy;
	float nx, ny, nz, lengthInv = 1.0f / radius;
	float s, t;

	float sectorStep = 2 * glm::pi<double>() / rings;
	float stackStep = glm::pi<double>() / slices;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= slices; ++i)
	{
		stackAngle = glm::pi<double>() / 2 - i * stackStep;
		xy = radius * glm::cos(stackAngle);
		z = radius * glm::sin(stackAngle);

		for (int j = 0; j <= rings; ++j)
		{
			sectorAngle = j * sectorStep;
			x = xy * glm::cos(sectorAngle);
			y = xy * glm::sin(sectorAngle);
			vertices.push_back({ glm::vec3(x, y, z), glm::vec3(x * lengthInv, y * lengthInv, z * lengthInv) });
		}
	}

	for (int i = 0; i < slices; ++i)
	{
		int k1 = i * (rings + 1);
		int k2 = k1 + rings + 1;

		for (int j = 0; j < rings; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}
			if (i != (slices - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	return Mesh(vertices, indices);
}