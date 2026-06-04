#pragma once

#include <SFML/Graphics.hpp>

#include "pXL/pXL.hpp"

struct Camera
{
	Camera(const sf::RenderTarget& window, const px::Scaling& scaling, sf::Vector2f position = {}) :
		m_window(window),
		m_scaling(scaling)
	{
		setPosition(position);
	}

	sf::Vector2f position{}, oldPosition{};
	float zoom{ 1.0f };

	void setPosition(sf::Vector2f position)
	{
		this->position = position;
		oldPosition = position;
	}

	sf::Vector2f lerp(float alpha) const
	{
		return px::lerp(oldPosition, position, alpha);
	}

	sf::Vector2f screenToWorld(sf::Vector2i position, float alpha = 1.f) const
	{
		return (static_cast<sf::Vector2f>(position) - static_cast<sf::Vector2f>(m_window.getSize()) / 2.f)
			/ m_scaling.getUnit() + lerp(alpha);
	}

	sf::View getView(float alpha) const
	{
		return sf::View{
			lerp(alpha) * m_scaling.getUnit(),
			static_cast<sf::Vector2f>(m_window.getSize()) * zoom
		};
	}

private:

	const sf::RenderTarget& m_window;
	const px::Scaling& m_scaling;
};