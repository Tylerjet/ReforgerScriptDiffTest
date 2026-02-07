[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_PlayerTeleportedFeedbackComponentClass: ScriptComponentClass
{
};

class SCR_PlayerTeleportedFeedbackComponent: ScriptComponent
{
	protected ref ScriptInvoker Event_OnPlayerTeleportedByEditor = new ScriptInvoker; //param int GM that teleported the player
	protected PlayerManager m_PlayerManager;
	
	/*!
	Get on player Teleported by editor script invoker. 
	*/
	ScriptInvoker GetOnPlayerTeleportedByEditor()
	{
		return Event_OnPlayerTeleportedByEditor;
	}
	
	/*!
	On player teleported send a notification and event that the local player character is teleported
	Will only send event if editor is open
	*/
	void TeleportedByEditor(SCR_EditableCharacterComponent character)
	{
		bool editorIsOpen = SCR_EditorManagerEntity.IsOpenedInstance();
		
		Event_OnPlayerTeleportedByEditor.Invoke(editorIsOpen);
		
		int id = Replication.FindId(character);
		
		//~ Only show notification if editor is closed
		if (!editorIsOpen)
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_GM_TELEPORTED_PLAYER, id);
		
	}
	

};