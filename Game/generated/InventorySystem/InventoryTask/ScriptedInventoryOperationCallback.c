/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem_InventoryTask
\{
*/

class ScriptedInventoryOperationCallback: InventoryOperationCallback
{
	/*
	Get the item replication id.
	*Only valid in Spawn/Insert/Remove inventory operations*
	*/
	proto external protected RplId GetItem();

	// callbacks

	// Override this method to implement callback logic for completed operation
	event protected void OnComplete();
	// Override this method to implement callback logic for failed operation
	event protected void OnFailed();
}

/*!
\}
*/
