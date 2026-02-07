//------------------------------------------------------------------------------------------------

class SCR_ServerBrowserComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: "10", UIWidgets.EditBox, "How many server entries")]
	protected int m_iEntriesLimit = 10; // Limit of entries count that can be display in list
	protected int m_iEntriesScrollPosition = 0;
	
	protected Room m_JoiningRoom = null;
		
	//ref OnSearchServers searchCallback = new OnSearchServers;
	ref OnJoinRoomSB m_JoinCallback = new OnJoinRoomSB;
	
	override void HandlerAttached(Widget w)
	{
		
	}
	
	
	void RequestServers()
	{
		ChimeraGame game = GetGame();
		BackendApi backend = game.GetBackendApi();
		ClientLobbyApi lobby = backend.GetClientLobby();
		
		lobby.GenerateRooms();
		
		// Generate servers 
		lobby.SetViewSize(m_iEntriesLimit);
	}
};