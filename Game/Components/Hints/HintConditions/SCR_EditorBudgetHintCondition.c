[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorBudgetHintCondition: SCR_BaseEditorHintCondition
{
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BudgetEditorComponent budgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		if (budgetManager)
			budgetManager.GetOnBudgetMaxReached().Insert(Activate);
	}
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BudgetEditorComponent budgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		if (budgetManager)
			budgetManager.GetOnBudgetMaxReached().Remove(Activate);
	}
};