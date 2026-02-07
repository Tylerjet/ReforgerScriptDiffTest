//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_eBudget")]
class SCR_CampaignBuildingBudgetToEvaluateData
{
	[Attribute(desc: "Fill in the budgets to be used with this provider", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_eBudget;
	
	//------------------------------------------------------------------------------------------------
	//! To be overridden in inherited classes, allow to set an additional custom condition to usage of the budget. For an example only with specific game mode etc.
	bool CanBeUsed()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityBudget GetBudget()
	{
		return m_eBudget;
	}
}
