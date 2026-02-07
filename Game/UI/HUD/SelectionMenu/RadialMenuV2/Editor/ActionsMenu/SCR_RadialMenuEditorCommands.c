//------------------------------------------------------------------------------------------------
class SCR_RadialMenuEditorCommands : SCR_RadialMenuEditorBase
{
	protected SCR_CommandActionsEditorComponent m_CommandActionsManager;
	protected SCR_BaseEditableEntityFilter m_CommandedEntityFilter;
	
	override protected int ValidateSelection()
	{
		// Check which input opened radial menu, EditorAddCommandMenu will queue the task, EditorSetCommandMenu will clear exising tasks
		m_ActionFlags = 0;
		if (m_InputManager.GetActionValue("EditorAddCommandMenu"))
		{
			m_ActionFlags |= EEditorCommandActionFlags.IS_QUEUE;
		}
		return m_ActionFlags;
	}
	
	override protected void UpdateEditorEntriesData(IEntity owner, vector cursorWorldPosition)
	{
		array<ref SCR_EditorActionData> actions = {};
		m_CommandActionsManager.GetAndEvaluateActions(cursorWorldPosition, actions, m_ActionFlags);
		
		foreach (SCR_EditorActionData actionData : actions)
		{
			AddEntry(new SCR_EditorActionRadialEntry(actionData, m_CommandActionsManager, cursorWorldPosition, m_ActionFlags), 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{		
		m_CommandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));			
		super.OnOpen(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OpenMenu(IEntity owner, bool isOpen)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
		{
			m_CommandedEntityFilter = SCR_BaseEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.COMMANDED));
		}
		if (m_CommandedEntityFilter && m_CommandedEntityFilter.IsEmpty())
		{
			return;
		}
		
		super.OpenMenu(owner, isOpen);
	}
};