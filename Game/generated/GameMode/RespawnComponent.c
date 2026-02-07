/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameMode
\{
*/

class RespawnComponentClass: GameComponentClass
{
}

/*!
Respawn component is a component attached to PlayerController serving as
interface for communication between the authority and remote clients in
respawn system context.
*/
class RespawnComponent: GameComponent
{
	/*!
	Returns the player controller this respawn component is attached to.
	*/
	proto external PlayerController GetPlayerController();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\poaram timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeSlice);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
	/*!
	Called when player controller request respawn lock is engaged,
	i.e. a request was fired. Valid until response is received or
	until timeout. Relevant to the owner client.
	*/
	event protected void OnRequestLockEngaged();
	/*!
	Called when player controller request respawn lock is disengaged,
	ie. a response is received from the server or on timeout(s).
	Relevant to the owner client.
	\param response The response from the server why the lock was lifted.
	*/
	event protected void OnRequestLockDisengaged(ERespawnResult result);
}

/*!
\}
*/
