[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DeleteDestroyedToolbarAction: SCR_EditorToolbarAction
{
	override bool IsServer()
	{
		return true;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return GetGame().GetGarbageManager();
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		GetGame().GetGarbageManager().Flush();
	}
};