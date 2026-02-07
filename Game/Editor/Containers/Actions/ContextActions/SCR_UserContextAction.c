[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_UserContextAction: SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!hoveredEntity)
			return false;
		
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(hoveredEntity.GetOwnerScripted().FindComponent(BaseActionsManagerComponent));
		if (!actionsManager || actionsManager.GetActionsCount() != 1)
			return false;
		
		ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
		if (!action || !action.CanBePerformed(SCR_EditorManagerEntity.GetInstance()))
			return false;
		
		//--- Update info with action name
		SCR_OverridableUIInfo info = SCR_OverridableUIInfo.Cast(m_Info);
		if (info)
			info.SetName(action.GetActionName());
		
		return true;
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!hoveredEntity)
			return;
		
		IEntity owner = hoveredEntity.GetOwnerScripted();
		if (!owner)
			return;
		
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(owner.FindComponent(BaseActionsManagerComponent));
		if (!actionsManager || actionsManager.GetActionsCount() != 1)
			return;
		
		ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
		if (!action || !action.CanBePerformed(SCR_EditorManagerEntity.GetInstance()))
			return;
		
		SCR_WorldEntityDelegateEditorComponent manager = SCR_WorldEntityDelegateEditorComponent.Cast(SCR_WorldEntityDelegateEditorComponent.GetInstance(SCR_WorldEntityDelegateEditorComponent));
		if (manager)
			manager.PerformFirstAction(owner);
	}
};