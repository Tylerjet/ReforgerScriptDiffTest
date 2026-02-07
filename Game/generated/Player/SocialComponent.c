/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Player
\{
*/

class SocialComponentClass: GameComponentClass
{
}

/*!
Component responsible for handling social interactions b/w 2 players.
Gathers restrictions from different sources like platform user permissions/privileges.
This component is meant to be on the Player Controller.
*/
class SocialComponent: GameComponent
{
	static ref ScriptInvoker<bool> s_OnBlockListUpdateInvoker = new ScriptInvoker<bool>();
	static ref ScriptInvoker<Room, array<BlockedRoomPlayer>> s_OnCheckedBlockedPlayersInRoomInvoker = new ScriptInvoker<Room, array<BlockedRoomPlayer>>();
	ref ScriptInvoker<int> m_OnBlockedPlayerJoinedInvoker = new ScriptInvoker<int>();
	ref ScriptInvoker<int, bool> m_OnReportPlayerFinishInvoker = new ScriptInvoker<int, bool>();

	/*!
	Is local user privileged to participate in given interaction?
	*/
	static proto bool IsPrivilegedTo(EUserInteraction interaction);
	/*!
	Requests the local user privilege associated with given interaction
	\param interaction Requested interaction.
	\param callback Callback object used to obtain result.
	\return Returns true on successful request placement.
	*/
	static proto bool RequestSocialPrivilege(EUserInteraction interaction, PrivilegeCallback cb);
	/*!
	Is local user allowed to participate in multiplayer?
	*/
	static proto bool IsMultiplayerAllowed();
	/*!
	Requests the local user privilege to participate in multiplayer games
	\param callback Callback object used to obtain result.
	\return Returns true on successful request placement.
	*/
	static proto bool RequestMultiplayerPrivilege(PrivilegeCallback cb);
	/*!
	Is the given interaction b/w this and otherPlayer allowed? Returns false
	in case of invalid arguments.
	*/
	proto external bool IsRestricted(int otherPlayerID, EUserInteraction interaction);
	/*!
	Determines if otherPlayer is blocked by this player. All player interactions
	are restricted if one player is blocked. Returns false in case of invalid argument.
	*/
	proto external bool IsBlocked(int otherPlayerID);
	/*!
	Determines if otherPlayer is blocked by this player. All player interactions
	are restricted if one player is blocked. Returns false in case of invalid argument.
	\param identity Game backend identity id
	\param platform Platform of other player
	\param platformIdHash Hash of platform specific identifier
	*/
	proto external bool IsBlockedIdentity(string identity, PlatformKind platform, string platformIdHash);
	/*!
	Blocks given player using BlockListApi. Uses provided callback as result.
	In case of full block list, error with code = EBERR_STORAGE_IS_FULL and
	apiCode = EACODE_ERROR_REQUEST_ERROR
	*/
	proto external void Block(BlockListRequestCallback callback, int otherPlayerID);
	/*!
	Removes given player from game block list. Invokes callback when requestt completes.
	\return false on failure or when player is not blocked throught a game block list.
	*/
	proto external bool Unblock(BlockListRequestCallback callback, int otherPlayerID);
	/*!
	Fills the provided array with the informations about blocked players.
	\return Number of blocked players
	*/
	static proto int GetBlockedPlayers(notnull array<BlockListItem> outItems);
	/*!
	Requests update of block list using BlockListApi.
	Invokes OnBlockListUpdate when finished.
	*/
	static proto void UpdateBlockList();
	/*!
	Requests to check blokced players in a room.
	Invokes OnCheckedBlockedPlayersInRoom when finished.
	*/
	static proto void CheckBlockedPlayersInRoom(notnull Room room);
	/*!
	Determines if otherPlayer is muted by this player for the duration of
	the game session. Only VoiceChat UserInteraction is affected. Returns
	false in case of invalid argument.
	*/
	proto external bool IsMuted(int otherPlayerID);
	/*!
	Mutes/unmutes otherPlayer for the duration of the game session. Only
	VoiceChat interaction is affected.
	*/
	proto external void SetMuted(int otherPlayerID, bool mute);
	/*!
	Reports inappropriate player behaviour into BackendApi
	*/
	proto external void ReportPlayer(int reportedPlayerID, SCR_EReportReason reason);

	// callbacks

	//! Event invoked as a result of UpdateBlockList
	static event protected void OnBlockListUpdate(bool success) { s_OnBlockListUpdateInvoker.Invoke(success); };
	//! Event invoked as a result of CheckBlockedPlayersInRoom
	static event protected void OnCheckedBlockedPlayersInRoom(Room room, array<BlockedRoomPlayer> blockedPlayers) { s_OnCheckedBlockedPlayersInRoomInvoker.Invoke(room, blockedPlayers); };
	//! Event invoked when player present on a platform or game blocklist joins the server
	event protected void OnBlockedPlayerJoined(int joinedPlayerID) { m_OnBlockedPlayerJoinedInvoker.Invoke(joinedPlayerID); };
	//! Event invoked as a result of ReportPlayer
	event protected void OnReportPlayerFinish(int reportedPlayerID, bool success) { m_OnReportPlayerFinishInvoker.Invoke(reportedPlayerID, success); };
}

/*!
\}
*/
