/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class WorldController: ScriptAndConfig
{
	static void InitInfo(WorldControllerInfo outInfo);
	private void WorldController();

	proto external World GetWorld();
	proto external bool IsMyOwn();
	proto external RplRole Role();
	/*!
	Attempts to run a remote procedure call (RPC) of this instance with parameters
	specified in method RplRpc attribute.
	\param      method  Member function to be invoked as an RPC.
	*/
	proto void Rpc(func method, void p0 = NULL, void p1 = NULL, void p2 = NULL, void p3 = NULL, void p4 = NULL, void p5 = NULL, void p6 = NULL, void p7 = NULL);
	proto external BaseSystem FindSystem();
	proto external RplIdentity GetServerSideOwner();
	static proto WorldController FindMyController(World world, typename type);
	static proto WorldController FindController(World world, typename type, RplIdentity serverSideOwner);
	static proto void GetServerSideOwners(World world, notnull array<int> outOwners);

	// callbacks

	event protected void OnAuthorityReady();
	event protected void OnAuthorityClosing();
}

/*!
\}
*/
