/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Code representing a result that occurs during a player respawn.
*/
enum ERespawnResult
{
	//! Respawned with no issues.
	OK,
	//! Respawn failed: Provided entity could not be spawned.
	NO_ENTITY,
	//! Respawn failed: Unknown error occured.
	UNKNOWN,
	//! Respawn failed: No response received, client time out.
	TIMEOUT,
}
