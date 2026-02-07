/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

//! Main replication API.
sealed class Replication
{
	private void Replication();
	private void ~Replication();

	const RplId INVALID_ID = 0xFFFFFFFF;
	const RplIdentity INVALID_IDENTITY = 0xFFFFFFFF;

	/*!
	Looks for RplId assigned by the Replication layer.
	\warning It is a lookup! Don't use in tight loops!
	\returns id or INVALID_ID if entity is not part of the replication process.
	*/
	static proto RplId FindId(Managed entity);
	/*!
	Looks for Item of the provided id.
	\returns Item or null when no item with provided id exists.
	*/
	static proto Managed FindItem(RplId itemId);
	/*!
	Tries to look for the owner of provided replication id. When successful
	returns its identity. Failure returns INVALID_ID constant.
	*/
	static proto RplIdentity FindOwner(RplId itemId);
	/*!
	Master time for all reaplication instances.
	(Exponentialy smoothed, is dependent on connection quality)
	\deprecated Use WorldTimestamp obtained from World instead.
	*/
	[Obsolete("Use WorldTimestamp obtained from World instead.")]
	static proto float Time();
	//! Tells whenever the replication is active.
	static proto bool IsRunning();
	/*!
	The replication is currently in runtime mode. The inserted nodes will
	receive a runtime IDs they will be automatically replicated to relevant
	clients.
	*/
	static proto bool Runtime();
	/*!
	The replication is currently in state of world loading. The inserted nodes
	will receive a loadtime IDs and have to be spawned/present on both (Server
	and Client).
	*/
	static proto bool Loadtime();
	/*!
	Notifies replication systems about changes in your object and queues him
	for replication in near future.
	*/
	static proto void BumpMe();
	/*!
	Sets the number of virtual connections which are just generating load for the master.
	\warning INTERNAL PROFILING FEATURE ONLY!
	*/
	static proto void SetNumVirtualConnections(int num);
	/*!
	\warning Using the hardcoded IsServer will result in code thats not modular
	and won't adapt when running in different environment consider using the
	replication roles and ownership instead.
	*/
	static proto bool IsServer();
	/*!
	\warning Using the hardcoded IsClient will result in code thats not modular
	and won't adapt when running in different environment consider using the
	replication roles and ownership instead.
	*/
	static proto bool IsClient();
	/*!
	Retrieve snapshot of connection statistics for given `identity` at current moment in time. This
	snapshot is not updated over time. To always get up-to-date statistics, call this function
	repeatedly. Rate of updates of internal statistics is not well defined, but it can be expected
	to happen about once every two seconds (usually more often). Note that only server is able to
	access connection information of all clients connected to it.

	When `identity` is identifier of a real connection (such as from client to server),
	these will be statistics for that connection.

	When `identity` is RplIdentity.Local(), returned statistics are for local player. Local player on
	server does not suffer from any connection quality issues and statistics will report
	perfect values (such as zero round-trip time). Local player on client reports statistics
	from its real connection to the server (as if `identity` was identifier of that connection).

	When `identity` is RplIdentity.Invalid() or it refers to non-existent connection,
	`null` is returned.

	\param[in]  identity  Identifier of queried connection.
	\return     Statistics of a given connection, or `null` when `identity` is wrong.
	*/
	static proto ref RplConnectionStats GetConnectionStats(RplIdentity identity);
}

/*!
\}
*/
