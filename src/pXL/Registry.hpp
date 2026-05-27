#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <cassert>
#include <optional>

namespace px
{
	template <typename T>
	class Registry
	{
	public:

		Registry() = default;
		Registry(const Registry&) = delete;
		Registry& operator=(const Registry&) = delete;

		void setError(T&& resource)
		{
			m_error = std::move(resource);
		}

		bool exists(const std::string& name) const { return m_resources.count(name); }

		const T& set(const std::string& name, T&& resource)
		{
			m_resources.insert_or_assign(name, std::move(resource));
			T* resourcePtr = &m_resources.at(name);
			m_names.insert_or_assign(resourcePtr, name);
			return *resourcePtr;
		}

		const T* tryGet(const std::string& name) const
		{
			if (exists(name))
			{
				return &m_resources.at(name);
			}

			return m_error ? &m_error.value() : nullptr;;
		}

		const T& get(const std::string& name) const
		{
			const T* out = tryGet(name);
			assert(out);
			return *out;
		}

		const std::string* getName(const T& resource)
		{
			return m_names.count(&resource)
				? &m_names.at(&resource)
				: nullptr;
		}

		const std::unordered_map<std::string, T>& data() const
		{
			return m_resources;
		}

	private:

		std::unordered_map<std::string, T> m_resources;
		std::unordered_map<const T*, std::string> m_names;
		std::optional<T> m_error;
	};
}