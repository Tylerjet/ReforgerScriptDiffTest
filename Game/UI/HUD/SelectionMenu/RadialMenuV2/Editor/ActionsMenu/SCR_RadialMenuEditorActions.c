//------------------------------------------------------------------------------------------------
class SCR_RadialMenuEditorActions: SCR_RadialMenuEditorBase
{
	protected SCR_ContextActionsEditorComponent m_ActionsManager;
	
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