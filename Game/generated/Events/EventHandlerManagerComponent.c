/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Events
* @{
*/

class EventHandlerManagerComponentClass: GameComponentClass
{
};

class EventHandlerManagerComponent: GameComponent
{
	proto void RegisterScriptHandler(string eventName, Managed inst, func callback, bool delayed = true);
	proto void RemoveScriptHandler(string eventName, Managed inst, func callback, bool delayed = true);
	//! Returns a list and count of all event handlers
	proto external int GetEventHandlers(out notnull array<BaseEventHandler> outEventHandlers);
};

/** @}*/
