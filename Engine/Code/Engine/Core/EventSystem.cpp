#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

EventSystem* g_theEventSystem = nullptr;

EventSystem::EventSystem(EventSystemConfig const& config)
	:m_config(config)
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::StartUp()
{
}

void EventSystem::ShutDown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

Strings EventSystem::GetRegisteredCommandNames() const
{
	Strings commandNames;
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	for (const std::pair<HashedCaseInsensitiveString, SubscriptionList>& pair : m_subscriptionListsByEventName)
	{
		const std::string& eventName = pair.first.GetOriginalString();
		commandNames.emplace_back(eventName);
	}

	return commandNames;
}

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	EventSubscriptionBase* subscription = new EventSubscriptionForStandaloneFunction(functionPtr);
	HashedCaseInsensitiveString hashedName(eventName);

	auto& subscriptionList = m_subscriptionListsByEventName[hashedName];
	subscriptionList.emplace_back(subscription);
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	// Find the subscription list 
	std::map<HashedCaseInsensitiveString, SubscriptionList>::iterator it = m_subscriptionListsByEventName.find(eventName);

	if (it != m_subscriptionListsByEventName.end())
	{
		SubscriptionList& subscriptions = it->second;
		for (auto iter = subscriptions.begin(); iter != subscriptions.end();)
		{
			if ((*iter)->IsForFunction(functionPtr)) 
			{
				delete* iter;
				iter = subscriptions.erase(iter);
			}
			else 
			{
				iter++;
			}
		}
	}
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	std::lock_guard<std::recursive_mutex> lock(m_eventRecursiveMutex);

	std::map<HashedCaseInsensitiveString, SubscriptionList>::iterator it = m_subscriptionListsByEventName.find(eventName);

	if (it != m_subscriptionListsByEventName.end())
	{
		// Loop through the list of callback functions 
		SubscriptionList& subscribersToThisEvent = it->second;
		for (EventSubscriptionBase* subscription : subscribersToThisEvent)
		{
			if (subscription && subscription->Fire(args))
			{
				break; // event was consumed
			}
		}
	}
	else
	{
		g_theConsole->HandleUnknownCommand(eventName);
	}
}

void EventSystem::FireEvent(std::string const& eventName)
{
	// Create temporary EventArgs
	EventArgs emptyArgs;

	// Fire event 
	FireEvent(eventName, emptyArgs);
}

void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
	}
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
	}
}

void FireEvent(std::string const& eventName, EventArgs& args)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->FireEvent(eventName, args);
	}
}

void FireEvent(std::string const& eventName)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->FireEvent(eventName);
	}
}
