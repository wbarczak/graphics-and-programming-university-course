#pragma once

#include <string>
#include <unordered_set>

#include <SFML/Audio.hpp>

#include "Registry.hpp"

namespace px
{
	struct SoundData
	{
		sf::SoundBuffer buffer;
		mutable sf::Sound sound;
		sf::Time delay;
	};

	class Engine;

	class SoundPlayer
	{
	public:

		void request(const std::string& name)
		{
			if (!m_sounds.exists(name))
			{
				return;
			}

			m_requested.insert(name);
			
			if (!m_timeSincePlayed.count(name))
			{
				m_timeSincePlayed.emplace(name, sf::Time::Zero);
			}
		}

	private:

		SoundPlayer(const Registry<SoundData>& sounds)
			: m_sounds(sounds)
		{
			for (const auto& [name, _] : m_sounds.data())
			{
				m_timeSincePlayed.emplace(name, sf::Time::Zero);
			}
		}

		void flush(sf::Time dt)
		{
			for (const auto& requested : m_requested)
			{
				const auto& soundData = m_sounds.get(requested);

				if (m_timeSincePlayed.at(requested) < soundData.delay)
				{
					continue;
				}

				soundData.sound.play();
			}

			m_requested.clear();

			for (auto& [_, timer] : m_timeSincePlayed)
			{
				timer += dt;
			}
		}

		const Registry<SoundData>& m_sounds;
		std::unordered_set<std::string> m_requested;
		std::unordered_map<std::string, sf::Time> m_timeSincePlayed;

		friend Engine;
	};
}