/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class GarbageManagerClass: GenericEntityClass
{
}

/*!
Allows semi-automatic management and disposal of 'garbage'.

Following types support being marked as garbage:
	ChimeraCharacter
	Vehicle
	InventoryItemComponent

Instances of such types can be inserted and removed from automatic collection.
Default life time of individual categories can be set in entity attributes.

Each garbage instance has separate lifetime which can be adjusted by either
inserting the garbage with custom lifetime, or by using the Bump method.

Garbage collection is server-only. Any requests to insert or withdraw objects
on the client side will be ignored. No collection pass occurs on clients.
*/
class GarbageManager: GenericEntity
{
	/*!
	Enqueues provided instance for garbage collection. Not all types are supported!
	\param lifeTime Life time of instance in seconds. If less than 0, default manager value is used.
	\param forceCollection If enabled inserts the instance for collection regardless of whether collection of this type is enabled by the manager.
	\return Returns true on success, false if instance is already enqueued or if it can not be enqueued.
	*/
	proto external bool Insert(IEntity ent, float lifeTime = -1, bool forceCollection = false);
	/*!
	Returns whether provided instance is enqueued for garbage collection.
	\return True if instance is enqueued for garbage collection, false otherwise.
	*/
	proto external bool IsInserted(IEntity ent);
	/*!
	Dequeues provided instance from garbage collection.
	\return True if instance was dequeued, false otherwise.
	*/
	proto external bool Withdraw(IEntity ent);
	/*!
	Adds additional life time to instance, prolonging its life time.
	\param additionalLifetime Lifetime to add to current lifetime (in seconds)
	\return Returns true on success, false if instance is not garbage.
	*/
	proto external bool Bump(IEntity ent, float additionalLifetime);
	/*!
	Returns the lifetime of an instance or -1 if not inserted.
	\return Lifetime duration in seconds or -1 if none.
	*/
	proto external float GetLifetime(IEntity ent);
	/*!
	Returns the remaining lifetime of an instance or -1 if not inserted.
	\return Remaining lifetime in seconds or -1 if none.
	*/
	proto external float GetRemainingLifetime(IEntity ent);
	/*!
	Forcefully disposes of all enqueued garbage instances in a single pass.
	*/
	proto external void Flush();

	// callbacks

	/*
	Returns whether provided instance can be collected by the system.
	Override and implement custom logic in derived classes.
	Does not guarantee insertion, but prevents insertion if false.
	Called prior to insertion.
	\param ent Entity instance to check logic for.
	\return Returns true if item can be inserted, false otherwise.
	*/
	event protected bool CanInsert(IEntity ent) { return true; };
}

/*!
\}
*/
