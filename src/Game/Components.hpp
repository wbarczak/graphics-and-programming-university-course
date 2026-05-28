#pragma once

#include <string>
#include <cstdint>

#include <SFML/System.hpp>

enum class ColiderType : uint8_t
{
	Physics,
	Solid,
	Platform,
	Hazard
};

struct Hitbox
{
	sf::FloatRect rect{};
	ColiderType type{};
};

struct Facing
{
	enum : int8_t
	{
		Left = -1,
		Right = 1
	};
};

struct Transform
{
	sf::Vector2f pos{};
	sf::Vector2f vel{};
	sf::Vector2f oldPos{};
	float jumpStartY{};
	int8_t facing{ Facing::Right };
};

struct Stationary
{
	sf::Vector2f position{};
	sf::Vector2f alignment{};
};

struct Controllable
{
	sf::Time cayoteTime{};
	sf::Time jumpBuffer{};
	bool canJump{};
	bool grounded{};
	bool wasGrounded{};
};

struct Lifetime
{
	sf::Time lived;
	sf::Time max;
};

struct EntityType
{
	std::string name;
};

struct IsParticle
{

};

struct Trigger
{
	bool active{};
	sf::Time timer, accumulated;
};

struct Toggle
{
	bool active{};
};

// AS is short for Animated State

struct ASDynamic
{
	Facing facing{};
	enum State : uint8_t
	{
		Grounded,
		Jumping,
		Falling
	}state{};
};

struct ASTriger
{
	bool active;
};

struct ASToggle
{
	bool active;
};

struct ASDecoration
{

};

struct ASTile
{

};