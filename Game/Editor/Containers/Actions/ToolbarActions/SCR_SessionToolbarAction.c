[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SessionToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "Enable to load session, disable to save it.")]
	protected bool m_bIsLoad;
	
	[Attribute(desc: "Set false if the button is shown in single player and true if shown when Multiplayers.")]
	protected bool m_bMultiplayerOnly;
	
	override bool IsServer()
	{
		return true;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//~ Disable for now
		return GetGame().IsDev();
		
		if (Replication.IsRunning() != m_bMultiplayerOnly)
			return false; 
		
		#ifdef WORKBENCH
			return true;
		#else
			return Replication.IsClient() && !m_bIsLoad; //--- ToDo: Check if Backend Session exists on server; also, load disabled until restarting is implemented
		#endif
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (m_bIsLoad)
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.EditorLoadDialog);
		else
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.EditorSaveDialog);
	}
};