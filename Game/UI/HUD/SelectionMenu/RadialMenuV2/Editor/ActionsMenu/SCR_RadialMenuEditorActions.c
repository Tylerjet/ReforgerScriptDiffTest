//------------------------------------------------------------------------------------------------
class SCR_RadialMenuEditorActions: SCR_RadialMenuEditorBase
{
	protected SCR_ContextActionsEditorComponent m_ActionsManager;
	
	override protected int ValidateSelection()
	{
		//--- Selection rules (based on the same condition in SCR_ContextMenuActionsEditorUIComponent)
		SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		SCR_BaseEditableEntityFilter hoveredFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
		if (hoveredFilter && selectedFilter)
		{
			SCR_EditableEntityComponent hoveredEntity = hoveredFilter.GetFirstEntity();
			if (hoveredEntity)
			{
				//--- Open menu over entity outside of the current selection - select it instead
				if (!selectedFilter.Contains(hoveredEntity))
					selectedFilter.Replace(hoveredEntity);
			}
			else
			{
				//--- Opened menu without any entity under cursor - clear the selection
				selectedFilter.Clear();
			}
		}
		return super.ValidateSelection();
	}
	
	override protected void UpdateEditorEntriesData(IEntity owner, vector cursorWorldPosition)
	{
		array<ref SCR_EditorActionData> actions = {};
		EEditorContextActionFlags actionFlags = ValidateSelection();
		m_ActionsManager.GetAndEvaluateActions(cursorWorldPosition, actions, actionFlags);
		
		foreach (SCR_EditorActionData actionData : actions)
		{
			int actionOrder = actionData.GetAction().GetOrder();
			
			AddEntry(new SCR_EditorActionRadialEntry(actionData, m_ActionsManager, cursorWorldPosition, actionFlags), 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{		
		m_ActionsManager = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent));
		super.OnOpen(owner);
	}
};