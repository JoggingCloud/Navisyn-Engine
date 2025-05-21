#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include <mutex>
#include <vector>
#include <string>
#include <map>

class Window;
typedef NamedProperties EventArgs;
typedef bool (*EventCallbackFunction)(EventArgs& args); 

struct EventSubscriptionBase
{
	EventSubscriptionBase() = default;
	virtual ~EventSubscriptionBase() = default;
	virtual bool Fire (EventArgs& args) = 0;
	virtual bool IsForFunction([[maybe_unused]]EventCallbackFunction functionPtr) { return false; } // for function callbacks
	virtual bool IsForObject([[maybe_unused]]void* objectPtr) { return false; } // for member-function callbacks
};

// Function-based subscriptions
struct EventSubscriptionForStandaloneFunction : public EventSubscriptionBase
{
	EventSubscriptionForStandaloneFunction(EventCallbackFunction functionPtr)
		: m_functionPtr(functionPtr) {}

	EventCallbackFunction m_functionPtr = nullptr;

	bool Fire(EventArgs& args) override 
	{ 
		if (m_functionPtr)
		{
			return m_functionPtr(args); 
		}
		return false;
	}
	bool IsForFunction(EventCallbackFunction functionPtr) override { return m_functionPtr == functionPtr; }
};

// Method-based subscription
template<typename T_ObjectType>
struct EventSubscriptionObjectMethod : public EventSubscriptionBase
{
	typedef bool (T_ObjectType::*EventCallBackMemberFunctionPtrType)(EventArgs& args);
	EventSubscriptionObjectMethod(T_ObjectType* pointerToObject, EventCallBackMemberFunctionPtrType methodPtr)
		: m_objectPtr(pointerToObject), m_methodPtr(methodPtr) {}

	EventCallBackMemberFunctionPtrType m_methodPtr = nullptr;
	T_ObjectType* m_objectPtr = nullptr;

	bool Fire(EventArgs& args) override { return (m_objectPtr->*m_methodPtr)(args); }
	bool IsForObject(void* objectPtr) override { return objectPtr == (void*)m_objectPtr; }
	bool IsForMethod(void* methodPtr) { return methodPtr == (void*)m_methodPtr; }
};

typedef std::vector<EventSubscriptionBase*> SubscriptionList;

struct EventSystemConfig 
{
	Window* m_window = nullptr;
};

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();

	void StartUp();
	void ShutDown();
	void BeginFrame();
	void EndFrame();

	Strings GetRegisteredCommandNames() const;
	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	
	template<typename T_ObjectType>
	void SubscribeEventCallBackObjectMethod(std::string const& eventName, T_ObjectType* objectPtr, bool (T_ObjectType::*methodPtr)(EventArgs& args));

	template<typename T_ObjectType>
	void UnsubscribeEventCallBackObjectMethod(std::string const& eventName, T_ObjectType* objectPtr, bool (T_ObjectType::* methodPtr)(EventArgs& args));

	void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);

protected:
	EventSystemConfig m_config;
	std::map<HashedCaseInsensitiveString, SubscriptionList> m_subscriptionListsByEventName;

private:
	mutable std::recursive_mutex m_eventRecursiveMutex;
};

template<typename T_ObjectType>
void EventSystem::SubscribeEventCallBackObjectMethod(std::string const& eventName, T_ObjectType* objectPtr, bool (T_ObjectType::* methodPtr)(EventArgs& args))
{
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	EventSubscriptionBase* subscription = new EventSubscriptionObjectMethod<T_ObjectType>(objectPtr, methodPtr);
	m_subscriptionListsByEventName[eventName].emplace_back(subscription);
}

template<typename T_ObjectType>
void EventSystem::UnsubscribeEventCallBackObjectMethod(std::string const& eventName, T_ObjectType* objectPtr, bool (T_ObjectType::* methodPtr)(EventArgs& args))
{
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	auto it = m_subscriptionListsByEventName.find(eventName);
	if (it != m_subscriptionListsByEventName.end())
	{
		SubscriptionList& subscriptions = it->second;
		for (auto iter = subscriptions.begin(); iter != subscriptions.end();)
		{
			EventSubscriptionBase* subscription = *iter;
			if (subscription && subscription->IsForObject(objectPtr))
			{
				delete subscription;
				iter = subscription.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		if (subscriptions.empty())
		{
			m_subscriptionListsByEventName.erase(it);
		}
	}
}

// Standalone global-namepsace helper functions; these forward to "the" event system, if it exists
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
void FireEvent(std::string const& eventName);