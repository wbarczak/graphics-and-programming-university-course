#pragma once

#include <unordered_map>
#include <string>
#include <bitset>
#include <cassert>
#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

namespace px
{
	class Engine;

	constexpr uint32_t k_allButtonsCount = sf::Keyboard::ScancodeCount + sf::Mouse::ButtonCount;

	enum class InputId : uint8_t
	{
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,

		Enter, Escape, Backspace, Tab, Space, Hyphen, Equal, LBracket, RBracket,
		Backslash, Semicolon, Apostrophe, Grave, Comma, Period, Slash,
		
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
		
		CapsLock, PrintScreen, ScrollLock, Pause, Insert, Home, PageUp,
		Delete, End, PageDown, Right, Left, Down, Up,
		
		NumLock, NumpadDivide, NumpadMultiply, NumpadMinus, NumpadPlus, NumpadEqual, NumpadEnter, NumpadDecimal,
		
		Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,
		
		NonUsBackslash, Application, Execute, ModeChange, Help, Menu, Select, Redo, Undo, Cut, Copy, Paste,
		VolumeMute, VolumeUp, VolumeDown, MediaPlayPause, MediaStop, MediaNextTrack, MediaPreviousTrack,
		LControl, LShift, LAlt, LSystem, RControl, RShift, RAlt, System,
		Back, Forward, Refresh, Stop, Search, Favorites, HomePage,
		LaunchApplication1, LaunchApplication2, LaunchMail, LaunchMediaSelect,

		MLeft, MRight, MMiddle, MExtra1, MExtra2
	};

	std::string stringifyInputId(InputId id);

	class Input final
	{
	public:

		bool isPressed(InputId input) const;
		bool isReleased(InputId input) const;
		bool isHeld(InputId input) const;
		std::optional<InputId> getJustPressed() const;
		sf::Vector2i getMousePosition() const;
		sf::Vector2i getMouseDelta() const;

	private:

		void newUpdate();
		void readEvent(const sf::Event& e);

		std::bitset<k_allButtonsCount> m_pressed, m_released, m_held;
		sf::Vector2i m_position{};
		sf::Vector2i m_delta{};
		bool m_inWindow{};

		friend Engine;
	};

	class Mapping final
	{
	public:

		explicit Mapping(const Input& input);

		bool isPressed(const std::string& action) const;
		bool isReleased(const std::string& action) const;
		bool isHeld(const std::string& action) const;

		void set(const std::string& action, InputId input);

		bool isPressed(InputId input) const;
		bool isReleased(InputId input) const;
		bool isHeld(InputId input) const;
		std::optional<InputId> getJustPressed() const;
		sf::Vector2i getMousePosition() const;
		sf::Vector2i getMouseDelta() const;

		const std::unordered_map<std::string, InputId>& data() const;

	private:

		void setUnderlyingInput(const Input& input);
		void bindAssert(const std::string& action) const;

		std::unordered_map<std::string, InputId> m_binds;
		const Input* m_input;

		friend Engine;
	};

#pragma region InputIdImplementation

	inline std::string stringifyInputId(InputId id)
	{
		switch (id)
		{
			using enum InputId;
		case A: return "A";
		case B: return "B";
		case C: return "C";
		case D: return "D";
		case E: return "E";
		case F: return "F";
		case G: return "G";
		case H: return "H";
		case I: return "I";
		case J: return "J";
		case K: return "K";
		case L: return "L";
		case M: return "M";
		case N: return "N";
		case O: return "O";
		case P: return "P";
		case Q: return "Q";
		case R: return "R";
		case S: return "S";
		case T: return "T";
		case U: return "U";
		case V: return "V";
		case W: return "W";
		case X: return "X";
		case Y: return "Y";
		case Z: return "Z";
		case Num1: return "Num1";
		case Num2: return "Num2";
		case Num3: return "Num3";
		case Num4: return "Num4";
		case Num5: return "Num5";
		case Num6: return "Num6";
		case Num7: return "Num7";
		case Num8: return "Num8";
		case Num9: return "Num9";
		case Num0: return "Num0";
		case Enter: return "Enter";
		case Escape: return "Escape";
		case Backspace: return "Backspace";
		case Tab: return "Tab";
		case Space: return "Space";
		case Hyphen: return "Hyphen";
		case Equal: return "Equal";
		case LBracket: return "LBracket";
		case RBracket: return "RBracket";
		case Backslash: return "Backslash";
		case Semicolon: return "Semicolon";
		case Apostrophe: return "Apostrophe";
		case Grave: return "Grave";
		case Comma: return "Comma";
		case Period: return "Period";
		case Slash: return "Slash";
		case F1: return "F1";
		case F2: return "F2";
		case F3: return "F3";
		case F4: return "F4";
		case F5: return "F5";
		case F6: return "F6";
		case F7: return "F7";
		case F8: return "F8";
		case F9: return "F9";
		case F10: return "F10";
		case F11: return "F11";
		case F12: return "F12";
		case F13: return "F13";
		case F14: return "F14";
		case F15: return "F15";
		case F16: return "F16";
		case F17: return "F17";
		case F18: return "F18";
		case F19: return "F19";
		case F20: return "F20";
		case F21: return "F21";
		case F22: return "F22";
		case F23: return "F23";
		case F24: return "F24";
		case CapsLock: return "CapsLock";
		case PrintScreen: return "PrintScreen";
		case ScrollLock: return "ScrollLock";
		case Pause: return "Pause";
		case Insert: return "Insert";
		case Home: return "Home";
		case PageUp: return "PageUp";
		case Delete: return "Delete";
		case End: return "End";
		case PageDown: return "PageDown";
		case Right: return "Right";
		case Left: return "Left";
		case Down: return "Down";
		case Up: return "Up";
		case NumLock: return "NumLock";
		case NumpadDivide: return "NumpadDivide";
		case NumpadMultiply: return "NumpadMultiply";
		case NumpadMinus: return "NumpadMinus";
		case NumpadPlus: return "NumpadPlus";
		case NumpadEqual: return "NumpadEqual";
		case NumpadEnter: return "NumpadEnter";
		case NumpadDecimal: return "NumpadDecimal";
		case Numpad1: return "Numpad1";
		case Numpad2: return "Numpad2";
		case Numpad3: return "Numpad3";
		case Numpad4: return "Numpad4";
		case Numpad5: return "Numpad5";
		case Numpad6: return "Numpad6";
		case Numpad7: return "Numpad7";
		case Numpad8: return "Numpad8";
		case Numpad9: return "Numpad9";
		case Numpad0: return "Numpad0";
		case NonUsBackslash: return "NonUsBackslash";
		case Application: return "Application";
		case Execute: return "Execute";
		case ModeChange: return "ModeChange";
		case Help: return "Help";
		case Menu: return "Menu";
		case Select: return "Select";
		case Redo: return "Redo";
		case Undo: return "Undo";
		case Cut: return "Cut";
		case Copy: return "Copy";
		case Paste: return "Paste";
		case VolumeMute: return "VolumeMute";
		case VolumeUp: return "VolumeUp";
		case VolumeDown: return "VolumeDown";
		case MediaPlayPause: return "MediaPlayPause";
		case MediaStop: return "MediaStop";
		case MediaNextTrack: return "MediaNextTrack";
		case MediaPreviousTrack: return "MediaPreviousTrack";
		case LControl: return "LControl";
		case LShift: return "LShift";
		case LAlt: return "LAlt";
		case LSystem: return "LSystem";
		case RControl: return "RControl";
		case RShift: return "RShift";
		case RAlt: return "RAlt";
		case System: return "System";
		case Back: return "Back";
		case Forward: return "Forward";
		case Refresh: return "Refresh";
		case Stop: return "Stop";
		case Search: return "Search";
		case Favorites: return "Favorites";
		case HomePage: return "HomePage";
		case LaunchApplication1: return "LaunchApplication1";
		case LaunchApplication2: return "LaunchApplication2";
		case LaunchMail: return "LaunchMail";
		case LaunchMediaSelect: return "LaunchMediaSelect";
		case MLeft: return "MLeft";
		case MRight: return "MRight";
		case MMiddle: return "MMiddle";
		case MExtra1: return "MExtra1";
		case MExtra2: return "MExtra2";
		}

		return "Undefined";
	}

#pragma endregion InputIdImplementation
#pragma region InputImplementation

	inline bool Input::isPressed(InputId input) const
	{
		return m_pressed[static_cast<size_t>(input)];
	}

	inline bool Input::isReleased(InputId input) const
	{
		return m_released[static_cast<size_t>(input)];
	}

	inline bool Input::isHeld(InputId input) const
	{
		return m_held[static_cast<size_t>(input)];
	}

	inline std::optional<InputId> Input::getJustPressed() const
	{
		if (m_pressed.none())
		{
			return {};
		}

		for (size_t i = 0; i < m_pressed.size(); ++i)
		{
			if (m_pressed.test(i))
			{
				return static_cast<InputId>(i);
			}
		}

		return {};
	}

	inline sf::Vector2i Input::getMousePosition() const
	{
		return m_position;
	}

	inline sf::Vector2i Input::getMouseDelta() const
	{
		return m_delta;
	}

	inline void Input::newUpdate()
	{
		m_pressed.reset();
		m_released.reset();
		m_delta = {};
	}

	inline void Input::readEvent(const sf::Event& e)
	{
		if (e.is<sf::Event::FocusLost>())
		{
			m_pressed.reset();
			
			for (size_t i = 0; i < k_allButtonsCount; ++i)
			{
				if (m_held[i])
				{
					m_released.set(i, true);
				}
			}

			m_held.reset();
		}
		else if (e.is<sf::Event::KeyPressed>())
		{
			const auto scan = e.getIf<sf::Event::KeyPressed>();

			if (scan->scancode == sf::Keyboard::Scancode::Unknown) return;

			m_pressed.set(static_cast<size_t>(scan->scancode), true);
			m_held.set(static_cast<size_t>(scan->scancode), true);
		}
		else if (e.is<sf::Event::KeyReleased>())
		{
			const auto scan = e.getIf<sf::Event::KeyReleased>();

			if (scan->scancode == sf::Keyboard::Scancode::Unknown) return;

			m_released.set(static_cast<size_t>(scan->scancode), true);
			m_held.set(static_cast<size_t>(scan->scancode), false);
		}
		else if (e.is<sf::Event::MouseButtonPressed>())
		{
			const auto button = e.getIf<sf::Event::MouseButtonPressed>();

			m_pressed.set(sf::Keyboard::ScancodeCount + static_cast<size_t>(button->button), true);
			m_held.set(sf::Keyboard::ScancodeCount + static_cast<size_t>(button->button), true);
		}
		else if (e.is<sf::Event::MouseButtonReleased>())
		{
			const auto button = e.getIf<sf::Event::MouseButtonReleased>();

			m_released.set(sf::Keyboard::ScancodeCount + static_cast<size_t>(button->button), true);
			m_held.set(sf::Keyboard::ScancodeCount + static_cast<size_t>(button->button), false);
		}
		else if (e.is<sf::Event::MouseMoved>())
		{
			const auto move = e.getIf<sf::Event::MouseMoved>();

			m_delta = move->position - m_position;
			m_position = move->position;
		}
		else if (e.is<sf::Event::MouseEntered>())
		{
			m_inWindow = true;
		}
		else if (e.is<sf::Event::MouseLeft>())
		{
			m_inWindow = false;
		}
	}

#pragma endregion InputImplementation
#pragma region MappingImplementation

	inline Mapping::Mapping(const Input& input) :
		m_input(&input)
	{}

	inline bool Mapping::isPressed(const std::string& action) const
	{
		bindAssert(action);

		return m_input->isPressed(m_binds.at(action));
	}

	inline bool Mapping::isReleased(const std::string& action) const
	{
		bindAssert(action);

		return m_input->isReleased(m_binds.at(action));
	}

	inline bool Mapping::isHeld(const std::string& action) const
	{
		bindAssert(action);

		return m_input->isHeld(m_binds.at(action));
	}

	inline void Mapping::set(const std::string& action, InputId input)
	{
		m_binds.insert_or_assign(action, input);
	}

	inline bool Mapping::isPressed(InputId input) const
	{
		return m_input->isPressed(input);
	}

	inline bool Mapping::isReleased(InputId input) const
	{
		return m_input->isReleased(input);
	}

	inline bool Mapping::isHeld(InputId input) const
	{
		return m_input->isHeld(input);
	}

	inline std::optional<InputId> Mapping::getJustPressed() const
	{
		return m_input->getJustPressed();
	}

	inline sf::Vector2i Mapping::getMousePosition() const
	{
		return m_input->getMousePosition();
	}

	inline sf::Vector2i Mapping::getMouseDelta() const
	{
		return m_input->getMouseDelta();
	}

	inline void Mapping::setUnderlyingInput(const Input& input)
	{
		m_input = &input;
	}

	inline void Mapping::bindAssert(const std::string& action) const
	{
		assert(m_binds.count(action) && "A binding needs to be set in px::Mapping prior to requesting");
	}

	inline const std::unordered_map<std::string, InputId>& Mapping::data() const
	{
		return m_binds;
	}

#pragma endregion MappingImplementation
}