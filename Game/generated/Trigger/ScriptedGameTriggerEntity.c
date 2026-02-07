/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Trigger
* @{
*/

class ScriptedGameTriggerEntityClass: BaseGameTriggerEntityClass
{
};

class ScriptedGameTriggerEntity: BaseGameTriggerEntity
{
	// Injects
	//! Override this method in inherited class to define a new filter.
	bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return DefaultEntityFilterForQuery(ent);
	}
	
	event protected event void OnInit(IEntity owner);
	event protected event void OnFrame(IEntity owner, float timeSlice);
};

/** @}*/
