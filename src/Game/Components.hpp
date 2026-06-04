#pragma once

#include <string>
#include <cstdint>

#include <SFML/System.hpp>
#include <SFML/Graphics/Rect.hpp>

enum class ColiderType : uint8_t
{
	Physics,
	Solid,
	Platform,
	Hazard,
	Zone
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
};

struct Toggle
{
	bool active{};
};

struct Crumbling
{
	bool active{};
	bool isAir{};
	sf::Time onTime, offTime, accumulated;
};

struct Trampoline{};

struct AnimPlayer{};

struct AnimFlipFlop{};

struct AnimCrumbling{};

struct AnimHitboxDisabled{};