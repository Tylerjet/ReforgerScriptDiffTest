/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Player
* @{
*/

class PlayerControllerClass: GenericControllerClass
{
};

class PlayerController: GenericController
{
	proto external IEntity GetControlledEntity();
	/*!
	Set the new controlled entity for the player.
	Ignored if called from clients in MP.
	*/
	proto external bool SetControlledEntity(IEntity entity);
	proto external PlayerCamera GetPlayerCamera();
	/*!
	When a PlayerController.RequestRespawn() is issued, the controller engages an internal lock
	that waits for response from the authority or timeout and drops any additional requests.
	
	This method returns whether we can currently issue a request respawn or not.
	\return True in case that we have no internal lock engaged and we can request, false otherwise.
	*/
	proto external bool CanRequestRespawn();
	//! Send request to the server to spawn a playable controller for us.
	proto external void RequestRespawn();
	//! Returns the Action Manager associated to this player controller
	proto external ActionManager GetActionManager();
	/*!
	Returns the RespawnComponent attached to this PlayerController or null if none.
	\return RespawnComponent instance or null if none.
	*/
	proto external RespawnComponent GetRespawnComponent();
	proto external HUDManagerComponent GetHUDManagerComponent();
	//! Returns text communication privilege
	proto external bool IsChatAllowed();
	//! Returns voice communication privilege
	proto external bool IsVonAllowed();
	/*!
	Block specified player from any communication
	for the time of one gameplay session.
	It does not invoke blocking on the platform level (eg XBox Live)
	*/
	proto external void SetPlayerBlockedState(int playerId, bool blocked);
	/*!
	Block specified player from voice communication
	for the time of one gameplay session.
	It does not invoke blocking on the platform level (eg XBox Live)
	*/
	proto external void SetPlayerMutedState(int playerId, bool blocked);
	/*!
	Determine if the specified player is able to communicate with this PC in any means
	with this particular player controller.
	*/
	proto external PermissionState GetPlayerBlockedState(int playerId);
	/*!
	Determine if the specified player is able to communicate with this PC using voice
	with this particular player controller.
	*/
	proto external PermissionState GetPlayerMutedState(int playerId);
	//! Returns True if the user has given role assigned
	proto external bool HasRole(EPlayerRole role);
	proto external void SetCharacterCameraRenederActive(bool active);
	proto external int GetPlayerId();
	proto external int GetRplIdentity();
	
	// callbacks
	
	//! Runs every time the controlled entity has been changed.
	event protected void OnControlledEntityChanged(IEntity from, IEntity to);
	//! Runs every time the controlled entity die.
	event protected void OnDestroyed(IEntity killer);
	//! Runs at the beggining of each frame
	//! Character entity might be null
	event void OnPrepareTestCase(ActionManager am, float dt, IEntity characterEntity);
};

/** @}*/
