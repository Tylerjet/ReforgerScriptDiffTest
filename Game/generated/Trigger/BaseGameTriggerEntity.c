/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Trigger
\{
*/

class BaseGameTriggerEntityClass: GenericEntityClass
{
}

class BaseGameTriggerEntity: GenericEntity
{
	// Injects
	protected ref ScriptInvoker Event_OnQueryFinished = new ScriptInvoker();
	ScriptInvoker GetOnQueryFinished()
	{
		return Event_OnQueryFinished;
	}

	// Check if the given entity is inside the Trigger shape (bypasses the defined query and filter)
	proto external bool QueryEntityInside(notnull IEntity ent);
	// Queries entities which are inside the Trigger shape based on the defined filter
	proto external void QueryEntitiesInside();
	// Compares the given entity with the defined filters (Name and/or Class)
	proto external bool DefaultEntityFilterForQuery(IEntity ent);
	/*!
	// Gets the array of the latest entities detected inside the Trigger
	*/
	proto external int GetEntitiesInside(out notnull array<IEntity> outEntities);
	// Sets the sphere radius of the Trigger
	proto external void SetSphereRadius(float radius);
	// Gets the current sphere radius of the Trigger
	proto external float GetSphereRadius();

	// callbacks

	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	event protected void OnActivate(IEntity ent);
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	event protected void OnDeactivate(IEntity ent);
	//! callback - query finished - occurs when the current query finished being processd and has updated results
	event protected void OnQueryFinished(bool bIsEmpty){ Event_OnQueryFinished.Invoke(this); Event_OnQueryFinished.Clear(); };
}

/*!
\}
*/
