#pragma once
#include <math.h>
#include <entt.hpp>
#include "GLAPI.h"

struct MeshComponent
{
	MeshComponent(Mesh* pointer, glm::vec3 obj_scale, glm::vec3 color_mask)
	{
		mesh = pointer;
		scl = obj_scale;
		clr = color_mask;
	}

	Mesh* get()
	{
		return mesh;
	}

	glm::vec3 color()
	{
		return clr;
	}

	glm::vec3 scale()
	{
		return scl;
	}

private:
	Mesh* mesh;
	glm::vec3 scl;
	glm::vec3 clr;
};

struct Archer
{
	Archer(bool is_red = true)
	{
		red = is_red;
		reload_time = 2.5;
	}

	bool IsRed()
	{
		return red;
	}

	bool CanShoot()
	{
		return (glfwGetTime() - last_shot) > reload_time;
	}

	void Reload()
	{
		last_shot = glfwGetTime();
	}

private:
	bool red;
	double reload_time;
	double last_shot = 0.f;
};

struct Position
{
	glm::vec3 coord;

	Position() = default;
	Position(const glm::vec3 new_coords) : coord(new_coords)
	{
	};

	operator glm::vec3& ()
	{
		return coord;
	};

	void operator= (glm::vec3 other)
	{
		coord = other;
	};

	glm::vec3 operator+ (glm::vec3 other)
	{
		return coord + other;
	};

	glm::vec3 operator- (glm::vec3 other)
	{
		return coord - other;
	};
};

struct Velocity
{
	glm::vec3 vel;

	Velocity() = default;
	Velocity(const glm::vec3 new_vel) : vel(new_vel)
	{
	};

	operator glm::vec3& ()
	{
		return vel;
	};

	void operator= (glm::vec3 other)
	{
		vel = other;
	};

	glm::vec3 operator+ (glm::vec3 other)
	{
		return vel + other;
	};

	glm::vec3 operator- (glm::vec3 other)
	{
		return vel - other;
	};
};

struct Orientation
{
	glm::quat ori;

	Orientation() = default;
	Orientation(const glm::quat new_ori) : ori(new_ori)
	{
	};

	operator glm::quat& ()
	{
		return ori;
	};

	void operator= (glm::quat other)
	{
		ori = other;
	};
};

struct Health
{
	Health(int starting_health = 100)
	{
		hp = starting_health;
	}

	void Hit(int damage)
	{
		hp -= damage;
	}

	bool IsGreaterThanZero()
	{
		return hp > 0;
	}

private:
	int hp;
};

struct Trajectory
{
	Trajectory(glm::vec3 s, glm::vec3 t, float speed)
	{
		const float gravity = 9.8f;
		glm::vec3 diff = t - s;
		glm::vec3 xz_pl = glm::vec3(diff.x, 0.f, diff.z);
		float dist = glm::length(xz_pl);
		xz_pl = glm::normalize(xz_pl);
		float speed_sq = glm::pow(speed, 2);
		float speed_quad = glm::pow(speed, 4);
		float x = dist;
		float root = speed_quad - gravity * (gravity * glm::pow(x, 2) + 2 * diff.y * speed_sq);

		if (root >= 0)
		{
			float sign = std::rand() % 2 == 0 ? 1.f : -1.f;
			root = glm::sqrt(root);
			float ang = std::atan2f(speed_sq + root * sign, gravity * x);
			vx = speed * glm::cos(ang) * xz_pl.x;
			vz = speed * glm::cos(ang) * xz_pl.z;
			vy = speed * glm::sin(ang);
		}

		s_time = glfwGetTime();
		s_pos = s;
	}

	void UpdatePosition(Position& curPos, Orientation& curOri)
	{
		double c_time = glfwGetTime();
		double time = c_time - s_time;
		glm::vec3 newPos;

		newPos.x = s_pos.x + (vx * time);
		newPos.z = s_pos.z + (vz * time);
		newPos.y = s_pos.y + ((vy * time) - (0.5f * 9.8f * glm::pow(time, 2)));

		curOri = glm::quatLookAt(glm::normalize(newPos - curPos.coord), glm::vec3(0.f, 1.f, 0.f));
		curPos = newPos;
	}

private:
	glm::vec3 s_pos;
	double s_time;
	float vx;
	float vy;
	float vz;
};