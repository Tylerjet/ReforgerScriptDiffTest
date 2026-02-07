/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class RplComponentClass: BaseRplComponentClass
{
}

//! Base class for entity replication - e.g. vehicles, characters, animals
class RplComponent: BaseRplComponent
{
	/*!
	Deletes a replicated entity.

	When called by the authority it removes the entity from replication delete the entity physically. If releaseFromReplication
	is true, once deleted from the replication on proxies they won't delete the entity physically.

	When called by a proxy no physical delete happens until the authority deletes the entity. If releaseFromReplication is true
	the entity is marked and won't be deleted physically when deleted from the replication (unless the authority managed to
	tell the proxy the entity was deleted on the server already).

	\param pEntity Entity to delete
	\param releaseFromReplication When true after the entity is deleted from replication it won't be deleted from the game.
	*/
	static proto void DeleteRplEntity(IEntity entity, bool releaseFromReplication);
	/*!
	Gives ownership to newOwner if possible. Also notifies listeners if possible.
	Unlike Give this supports also custom data passing between the old and the new owner.

	\warning This method might be replaced by a unified Give method in the future. Use with caution.

	\param newOwner New owner.
	\param alwaysNotify Notifies listeners even if no change in ownership happened.
	*/
	proto external void GiveExt(RplIdentity newOwner, bool alwaysNotify);
	/*!
	Enable owner streaming.
	This is useful for situations such as with player characters. Once valid for streaming
	out we want to keep them untouched. Otherwise, players would have nothing to play with.

	\warning Works only if Network Dynamic Simulation is enabled

	\param enable If true streaming is enabled.
	*/
	proto external void EnableOwnerStreaming(bool enable);
	/*!
	Force scheduler to move node
	This is useful for nodes that do not have networked movement component, but are
	forcefully moved. To be used only in special cases.

	\warning Works only if Network Dynamic Simulation is enabled

	\param previousPos previous position to move the node from
	*/
	proto external void ForceNodeMovement(vector previousPos);
	/*!
	Enable streaming for a specific entity.
	This takes precedence over EnableStreaming for a specific Connection or Connection/Node

	\warning Works only if Network Dynamic Simulation is enabled

	\param enable If true streaming is enabled.
	*/
	proto external void EnableStreaming(bool enable);
	/*!
	Enable streaming of node to a specific player.
	Respects disabled connection rules, respects disabled nodes rules

	\warning Works only if Network Dynamic Simulation is enabled

	\param identity Client connection
	\param enable If true streaming is enabled.
	*/
	proto external void EnableStreamingConNode(RplIdentity identity, bool enable);
	/*!
	Enable streaming for a specific player.
	Ignores Connection/Node rules, but respects disabled nodes

	\warning Use with caution. This could can cause performance issues if used by many players.

	\param identity Client connection
	\param enable If true streaming is enabled.
	*/
	static proto void EnableStreamingForConnection(RplIdentity identity, bool enable);
}

/*!
\}
*/
