/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

sealed class RplNode: pointer
{
	proto external RplId GetId();
	proto external RplRole GetRole();
	proto external bool IsLocked();
	proto external void SetParent(RplNode newParent, bool sendMove = true);
}

/*!
\}
*/
