#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>

template<typename T>
struct TypeID
{
	static size_t Get() 
	{ 
		static char s_id; 
		return reinterpret_cast<size_t>(&s_id); 
	}
};

// === TPB ===
class TypePropertyBase
{
public:
	virtual ~TypePropertyBase() {};
	virtual size_t GetTypeID() const = 0;
	virtual TypePropertyBase* Clone() const = 0;
};

template<typename T>
class TypeProperty : public TypePropertyBase
{
public:
	explicit TypeProperty(const T& value) : m_value(value) {}
	virtual ~TypeProperty() {};

	size_t GetTypeID() const override { return TypeID<T>::Get(); }
	TypePropertyBase* Clone() const override { return new TypeProperty<T>(*this); }

	virtual T Get() const { return m_value; }
	void Set(const T& value) { m_value = value; }

private:
	T m_value;
};

class NamedProperties
{
public:
	NamedProperties() = default;

	NamedProperties(const NamedProperties& other)
	{
		for (const auto& [key, property] : other.m_keyValuePairs) 
		{
			m_keyValuePairs[key] = property->Clone();
		}
	}

	~NamedProperties()
	{
		for (auto& [key, value] : m_keyValuePairs)
		{
			delete value;
		}
	}

	NamedProperties& operator=(const NamedProperties& other)
	{
		if (this != &other)
		{
			for (auto& [key, property] : m_keyValuePairs)
			{
				delete property;
			}
			
			m_keyValuePairs.clear();
			
			for (const auto& [key, property] : other.m_keyValuePairs)
			{
				m_keyValuePairs[key] = property->Clone();
			}
		}
		return *this;
	}

	template<typename T>
	void SetValue(const std::string& keyName, const T& value)
	{
		auto found = m_keyValuePairs.find(keyName);
		if (found != m_keyValuePairs.end())
		{
			delete found->second;
		}
		m_keyValuePairs[keyName] = new TypeProperty<T>(value);
	}

	template<typename T>
	T GetValue(const std::string& keyName, const T& defaultValue) const
	{
		auto found = m_keyValuePairs.find(keyName);
		if (found != m_keyValuePairs.end() && found->second)
		{
			size_t type = found->second->GetTypeID();
			if (type == TypeID<T>::Get())
			{
				return static_cast<TypeProperty<T>*>(found->second)->Get();
			}
		}
		return defaultValue;
	}

	bool HasArgument(const std::string& keyName) const { return m_keyValuePairs.find(keyName) != m_keyValuePairs.end(); }

public:
	std::map<HashedCaseInsensitiveString, TypePropertyBase*> m_keyValuePairs;
};
