class SCR_BlockedPlayerPresentDialog : SCR_ConfigurableDialogUi
{
	protected string m_sPlayerNames;
	protected string m_sTextID = "#AR-ServerBrowser_JoinBlockedPlayer";
	
	protected Room m_RoomToJoin;
	
	ref ScriptInvoker m_OnConfirmJoinRoom = new ScriptInvoker();
	
	SCR_ConfigurableDialogUi m_Dialog;
	
	//------------------------------------------------------------------------------------------------
	void SCR_BlockedPlayerPresentDialog(string playerNamesIGuess, Room roomToJoin)
	{
		m_sPlayerNames = playerNamesIGuess;
		m_RoomToJoin = roomToJoin;
		m_Dialog = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "blocked_player_present", this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		TextWidget message = m_Dialog.GetMessageWidget();
		if (message)
			message.SetTextFormat(m_sTextID, m_sPlayerNames);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		m_OnConfirmJoinRoom.Invoke(m_RoomToJoin);
		Close();
	}
};