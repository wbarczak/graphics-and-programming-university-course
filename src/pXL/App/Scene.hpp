#pragma once

#include <optional>
#include <any>

namespace sf
{
	class RenderTarget;
	class Window;
}

namespace px
{
	class InputRaw;
	class Assets;
	class SceneCommands;
	class SceneConfig;
	class Transition;
	class Mapping;
	class Scaling;
	class SoundPlayer;

	struct EngineApi
	{
		SceneCommands& comms;
		Transition& transition;
		SoundPlayer& sounds;
		const Assets& assets;
		const Mapping& mapping;
		const Scaling& scaling;
	};

	class Scene;
	class Engine;

	class SceneInitCtx
	{
	public:

		SceneInitCtx(SceneConfig& properties, Transition& transition, const sf::RenderTarget& window, EngineApi api) :
			properties(properties),
			transition(transition),
			window(window),
			api(api)
		{}

		SceneConfig& properties;
		Transition& transition;// this should not be there and also not be in the update ctx
		const sf::RenderTarget& window;

	private:

		EngineApi api;

		friend Scene;
		friend Engine;
	};

	struct UpdateCtx
	{
		const sf::RenderTarget& window;
		const sf::Time dt;
		Transition& transition;
	};

	struct DrawCtx
	{
		sf::RenderTarget& window;
		const Assets& assets;
		float alpha{};
	};

	class SceneStack;

	class Scene
	{
	public:

		Scene(SceneInitCtx& ctx) : api(ctx.api) {}
		virtual ~Scene() = default;

		virtual void onEnter(std::any&& payload) {}
		virtual void onExit() {}
		virtual void onEvent(const sf::Event& event) {}
		virtual void update(UpdateCtx& ctx) {}
		virtual void fixedUpdate(UpdateCtx& ctx) {}
		virtual void draw(DrawCtx& ctx) const {};

	protected:

		EngineApi api;

	private:

		friend SceneStack;
	};
}