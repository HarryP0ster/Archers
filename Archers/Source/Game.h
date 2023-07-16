#pragma once
#include "EntityComponents.h"
#include "FileManager.h"

class ArchersGame
{
public:
	ArchersGame()
	{
;
	}

	~ArchersGame()
	{
		ent_registry.clear();
		glfwTerminate();
	}

	bool Prepare()
	{
		bool res = OpenGLAPI::GLInit(&window, 1280, 720, "Archers");
		shaderProgram = glCreateProgram();
		res = res & OpenGLAPI::GLCompileShader("Shaders\\default.vert", GL_VERTEX_SHADER, shaderProgram);
		res = res & OpenGLAPI::GLCompileShader("Shaders\\default.frag", GL_FRAGMENT_SHADER, shaderProgram);

		if (res)
		{
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
			glLinkProgram(shaderProgram);
			camera = ent_registry.create();
			ent_registry.emplace<Position>(camera, camera_sp);
			LoadAssets();
			SetupField(10, 10, 10);
		}

		return res;
	}

	void SetupEvents()
	{
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int new_width, int new_height) 
		{
			glViewport(0, 0, new_width, new_height);
		});
		glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods)
		{
			ArchersGame* context = static_cast<ArchersGame*>(glfwGetWindowUserPointer(win));

			if (action == GLFW_PRESS || action == GLFW_REPEAT)
			{
				switch (key)
				{
				case GLFW_KEY_LEFT:
					context->UpdateCamera(-1);
					break;
				case GLFW_KEY_RIGHT:
					context->UpdateCamera(1);
					break;
				default:
					break;
				}
			}
		});
	}

	void GameCycle()
	{
		int vw, vh, frames = 0;;
		glfwGetFramebufferSize(window, &vw, &vh);
		float aspect = vw / (float)vh;
		//glm::mat4 projection = glm::perspective(45.f, aspect, 0.01f, 1000.f);
		glm::mat4 projection = glm::ortho(-65.f * aspect, 65.f * aspect, -65.f, 65.f, 0.01f, 200.f);

		while (!glfwWindowShouldClose(window))
		{
			double frame_start = glfwGetTime();
			double frame_end;
			glm::mat4 view = glm::lookAt(ent_registry.get<Position>(camera).coord, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

			if (archers_count < 40 && frames % 2 == 0)
			{
				archers_count += 2;
				SpawnArchers();
			}

			glClearColor(0.73, 0.84, 0.95, 1.0);
			glClearDepth(1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(shaderProgram);
			UpdateSimulation();

			//mProj
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));
			//mView
			glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(view));
			DrawFrame();

			glfwSwapBuffers(window);
			frames++;

			do
			{
				glfwPollEvents();
				frame_end = glfwGetTime();
			} while (frame_end - frame_start < 0.05);
		}
	}

	void UpdateCamera(float delta)
	{
		camera_angle += delta;
		glm::quat q = glm::quat_cast(glm::mat3(1.f));
		q = q * glm::angleAxis(glm::radians(camera_angle), glm::vec3(0, 1, 0));
		q = q * glm::angleAxis(glm::radians(0.f), glm::vec3(1, 0, 0));
		q = q * glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1));
		ent_registry.get<Position>(camera) = q * camera_sp;
	}

private:
	void UpdateSimulation()
	{
		for (auto object : ent_registry.view<Trajectory>())
		{
			Trajectory& project_traj = ent_registry.get<Trajectory>(object);
			project_traj.UpdatePosition(ent_registry.get<Position>(object), ent_registry.get<Orientation>(object));

			if (ent_registry.get<Position>(object).coord.y > 0.f)
			{
				for (auto archR : ent_registry.view<Archer>())
				{
					if (glm::distance(ent_registry.get<Position>(object).coord, ent_registry.get<Position>(archR).coord) < 1.7f)
					{
						ent_registry.get<Health>(archR).Hit(20);
						ent_registry.destroy(object);

						if (ent_registry.get<Health>(archR).IsGreaterThanZero() == false)
						{
							ent_registry.destroy(archR);
						}
						break;
					}
				}
			}
			else
			{
				ent_registry.destroy(object);
			}
		}
		ent_registry.compact();

		auto archer_entt = ent_registry.view<Archer>();
		std::vector<int> targets;
		std::vector<float> distances;
		distances.resize(archer_entt.size());
		targets.resize(distances.size());

		for (int i = 0; i < archer_entt.size(); i++)
		{
			for (int j = i+1; j < archer_entt.size(); j++)
			{
				float distance = glm::length(ent_registry.get<Position>(archer_entt[i]).coord - ent_registry.get<Position>(archer_entt[j]).coord);

				if (ent_registry.get<Archer>(archer_entt[i]).IsRed() != ent_registry.get<Archer>(archer_entt[j]).IsRed())
				{
					if (distances[i] == 0 || distances[i] > distance)
					{
						distances[i] = distance;
						targets[i] = j;
					}

					if (distances[j] == 0 || distances[j] > distance)
					{
						distances[j] = distance;
						targets[j] = i;
					}
				}
				else
				{
					if (distance <= 3.4f)
					{
						distances[i] = -1;
						targets[i] = j;
						distances[j] = -1;
						targets[j] = i;
					}
				}
			}

			if (distances[i] != 0.f)
			{
				glm::vec3 dir = glm::normalize(glm::vec3(ent_registry.get<Position>(archer_entt[targets[i]]).coord - ent_registry.get<Position>(archer_entt[i]).coord));

				if (distances[i] == -1 && glm::dot(ent_registry.get<Velocity>(archer_entt[i]).vel, dir) >= 0)
				{
					ent_registry.get<Velocity>(archer_entt[i]) = glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::vec3(-0.1f - static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 0.f, -0.1f - static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * dir);
				}
				else if (distances[i] > 40.f)
				{
					ent_registry.get<Velocity>(archer_entt[i]) = glm::vec3(1.f, 0.f, 1.f) * dir;
				}
				else if (distances[i] > 0 && distances[i] <= 40.f && ent_registry.get<Archer>(archer_entt[i]).CanShoot())
				{
					ent_registry.get<Velocity>(archer_entt[i]) = glm::vec3(0.f, 0.f, 0.f);
					ShootProjectile(archer_entt[i], ent_registry.get<Position>(archer_entt[targets[i]]).coord);
				}
				else if (distances[i] > 0)
				{
					ent_registry.get<Velocity>(archer_entt[i]) = glm::vec3(0.f, 0.f, 0.f);
				}
			}
		}
	}

	void DrawFrame()
	{
		for (auto object : ent_registry.view<MeshComponent>())
		{
			if (ent_registry.try_get<Velocity>(object) != nullptr)
			{
				glm::vec3 new_pos = ent_registry.get<Position>(object).coord + ent_registry.get<Velocity>(object).vel;
				if (new_pos.x >= 48.f || new_pos.x <= -48.f)
				{
					ent_registry.get<Velocity>(object).vel.x *= -1.f;
					new_pos = ent_registry.get<Position>(object).coord + ent_registry.get<Velocity>(object).vel;
				}
				if (new_pos.z >= 48.f || new_pos.z <= -48.f)
				{
					ent_registry.get<Velocity>(object).vel.z *= -1.f;
					new_pos = ent_registry.get<Position>(object).coord + ent_registry.get<Velocity>(object).vel;
				}

				ent_registry.get<Position>(object).coord = new_pos;
			}

			MeshComponent& object_mesh = ent_registry.get<MeshComponent>(object);
			glm::mat4 model = glm::translate(glm::mat4(1.f), ent_registry.get<Position>(object).coord) * glm::mat4_cast(ent_registry.get<Orientation>(object).ori);
			//mWorld
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
			//scale
			glUniform3fv(3, 1, glm::value_ptr(object_mesh.scale()));
			//color
			glUniform3fv(4, 1, glm::value_ptr(object_mesh.color()));

			object_mesh.get()->BindBuffers();
			glDrawElements(GL_TRIANGLES, object_mesh.get()->NumIndices(), GL_UNSIGNED_INT, NULL);
			object_mesh.get()->ClearBinds();
		}
	}

	void LoadAssets()
	{
		std::vector<Vertex> tile_vertices = {
			{{5.f, 0.f, 5.f}, {1.f, 1.f, 1.f}},
			{{5.f, 0.f, -5.f}, {1.f, 1.f, 1.f}},
			{{-5.f, 0.f, 5.f}, {1.f, 1.f, 1.f}},
			{{-5.f, 0.f, -5.f}, {1.f, 1.f, 1.f}},
			{{5.f, -2.f, 5.f}, {1.f, 1.f, 1.f}},
			{{5.f, -2.f, -5.f}, {1.f, 1.f, 1.f}},
			{{-5.f, -2.f, 5.f}, {1.f, 1.f, 1.f}},
			{{-5.f, -2.f, -5.f}, {1.f, 1.f, 1.f}}
		};

		std::vector<Vertex> arrow_vertices = {
			{{0.1f, 0.1f, 1.5f}, {1.f, 1.f, 1.f}},
			{{0.1f, 0.1f, -1.5f}, {1.f, 1.f, 1.f}},
			{{-0.1f, 0.1f, 1.5f}, {1.f, 1.f, 1.f}},
			{{-0.1f, 0.1f, -1.5f}, {1.f, 1.f, 1.f}},
			{{0.1f, -0.1f, 1.5f}, {1.f, 1.f, 1.f}},
			{{0.1f, -0.1f, -1.5f}, {1.f, 1.f, 1.f}},
			{{-0.1f, -0.1f, 1.5f}, {1.f, 1.f, 1.f}},
			{{-0.1f, -0.1f, -1.5f}, {1.f, 1.f, 1.f}}
		};

		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 1, 3,
			6, 5, 4, 7, 5, 6,
			0, 2, 6, 6, 4, 0,
			1, 0, 5, 4, 5, 0,
			5, 7, 1, 3, 1, 7,
			7, 6, 3, 2, 3, 6
		};

		arrow = Mesh(arrow_vertices, indices);
		tile = Mesh(tile_vertices, indices);
		archer = OpenGLAPI::GenerateSphereMesh(1.7f, 32, 32);
		archer.calculate_normals();
		tile.calculate_normals();
	}

	void SetupField(int tilesH, int tilesV, int tileSize)
	{
		int startPoint = -(tilesH * tilesV / 2.f) + tileSize/2.f;
		int posZ = startPoint, posX = startPoint;

		for (int i = 0; i < tilesV; i++)
		{
			for (int j = 0; j < tilesH; j++)
			{
				entt::entity entity = ent_registry.create();
				ent_registry.emplace<Position>(entity, glm::vec3(posX, 0, posZ));
				ent_registry.emplace<Orientation>(entity, glm::angleAxis(0.f, glm::vec3(0, 1, 0)));
				ent_registry.emplace<MeshComponent>(entity, &tile, glm::vec3(0.9f), glm::vec3(0.f, 1.f, 0.f));
				posZ += tileSize;
			}
			posZ = startPoint;
			posX += tileSize;
		}
	}

	void SpawnArchers()
	{
		entt::entity entity = ent_registry.create();
		ent_registry.emplace<Position>(entity, glm::vec3(-40, 2.5, -40));
		ent_registry.emplace<Orientation>(entity, glm::angleAxis(0.f, glm::vec3(0, 1, 0)));
		ent_registry.emplace<MeshComponent>(entity, &archer, glm::vec3(1.f), glm::vec3(1.f, 0.f, 0.f));
		ent_registry.emplace<Archer>(entity, Archer(true));
		ent_registry.emplace<Health>(entity);
		ent_registry.emplace<Velocity>(entity, glm::vec3(glm::vec3(0.1f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 0.f, 0.1f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX))));

		entt::entity entity2 = ent_registry.create();
		ent_registry.emplace<Position>(entity2, glm::vec3(40, 2.5, 40));
		ent_registry.emplace<Orientation>(entity2, glm::angleAxis(0.f, glm::vec3(0, 1, 0)));
		ent_registry.emplace<MeshComponent>(entity2, &archer, glm::vec3(1.f), glm::vec3(0.f, 0.f, 1.f));
		ent_registry.emplace<Archer>(entity2, Archer(false));
		ent_registry.emplace<Health>(entity2);
		ent_registry.emplace<Velocity>(entity2, glm::vec3(glm::vec3(-0.1f - static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 0.f, -0.1f - static_cast <float> (rand()) / static_cast <float> (RAND_MAX))));
	}

	void ShootProjectile(entt::entity archer, glm::vec3 target)
	{
		if (ent_registry.try_get<Archer>(archer) != nullptr)
		{
			glm::vec3 pos = ent_registry.get<Position>(archer) + glm::vec3(0.f, 1.7f, 0.f);
			entt::entity projectile = ent_registry.create();
			Trajectory prj_trj(pos, target, 40);

			float dotProdZ = glm::dot(glm::vec3(0.f, 0.f, 1.f), glm::normalize(target - pos));
			float dotProdX = glm::dot(glm::vec3(0.f, 0.f, 1.f), glm::normalize(target - pos));
			glm::quat q = glm::quatLookAt(glm::normalize(target - pos), glm::vec3(0.f, 1.f, 0.f));

			ent_registry.emplace<Position>(projectile, glm::vec3(pos));
			ent_registry.emplace<Orientation>(projectile, q);
			ent_registry.emplace<Trajectory>(projectile, prj_trj);
			ent_registry.emplace<MeshComponent>(projectile, &arrow, glm::vec3(1.f), glm::vec3(0.f));

			ent_registry.get<Archer>(archer).Reload();
		}
	}

	GLFWwindow* window = nullptr;
	unsigned int shaderProgram;
	entt::registry ent_registry;

	entt::entity camera;
	float camera_angle = 0.f;
	const glm::vec3 camera_sp = {-80, 60, 80};

	Mesh tile;
	Mesh arrow;
	Mesh archer;
	int archers_count = 0;
};