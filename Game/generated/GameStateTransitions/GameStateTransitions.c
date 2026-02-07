/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameStateTransitions
\{
*/

sealed class GameStateTransitions
{
	private void GameStateTransitions();
	private void ~GameStateTransitions();

	//! Check whether transition to different game state has been requested this frame and will start next frame.
	static proto bool IsTransitionRequested();
	//! Check whether transition to different game state is currently happening.
	static proto bool IsTransitionInProgress();
	static proto bool IsTransitionRequestedOrInProgress();
	//! Request transition from main menu to hosted multiplayer session (registered in server browser).
	static proto bool RequestPublicServerTransition(JsonApiStruct config);
	//! Request transition from main menu to gameplay as client connected to server in multiplayer session.
	static proto bool RequestServerConnectTransition(string IPAddr);
	//! Request change of mission
	static proto bool RequestMissionChangeTransition(MissionHeader mission);
	//! Request change of world, keeping other properties of the game session.
	static proto bool RequestWorldChangeTransition(string worldPath);
	//! Request reload of ongoing multiplayer game.
	static proto bool RequestServerReload();
	//! Request server config change and reload the game.
	static proto bool RequestServerConfigChange(notnull ServerConfigMeta configMeta);
	//! Request transition from whatever is current multiplayer session mode back to main menu.
	static proto void RequestGameplayEndTransition(KickCauseCode code = KickCauseCode.NONE);
	//! Request graceful shut down of the game from whatever is current game state (eg. online vs. offline).
	static proto void RequestGameTerminateTransition();
}

/*!
\}
*/
