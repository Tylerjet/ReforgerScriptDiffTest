[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PingContextAction : SCR_BaseContextAction
{
	[Attribute("0")]
	bool m_IsUrgent;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		// Don't show action if urgent and editor mode is limited
		if ((flags & EEditorContextActionFlags.LIMITED) && m_IsUrgent)
		{
			return false;
		}
		// Only visible on gamepad
		if (flags & EEditorContextActionFlags.USING_MOUSE_AND_KEYBOARD)
		{
			return false;
		}
		
		// Only show action if cursorWorldPosition is valid
		return cursorWorldPosition != vector.Zero;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PingEditorComponent pingComponent = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent));
		
		pingComponent.SendPing(m_IsUrgent, cursorWorldPosition, hoveredEntity);
	}
};