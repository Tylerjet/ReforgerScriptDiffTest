[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_BudgetType")]
class SCR_EditableEntityCoreBudgetSetting
{
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	private EEditableEntityBudget m_BudgetType;
	
	[Attribute("1", UIWidgets.Auto, "")]
	private int m_fMinBudgetCost;

	private int m_fCurrentBudget;
		
	EEditableEntityBudget GetBudgetType()
	{
		return m_BudgetType;
	}
	int GetMinBudgetCost()
	{
		return m_fMinBudgetCost;
	}
	int GetCurrentBudget()
	{
		return m_fCurrentBudget;
	}
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	void AddToBudget(SCR_EntityBudgetValue budgetCost = null)
	{
		int budgetCostValue = GetMinBudgetCost();
		if (budgetCost) budgetCostValue = budgetCost.GetBudgetValue();
		
		m_fCurrentBudget += Math.Max(budgetCostValue, GetMinBudgetCost());
	}
	
	void SubtractFromBudget(SCR_EntityBudgetValue budgetCost = null)
	{
		int budgetCostValue = GetMinBudgetCost();
		if (budgetCost) budgetCostValue = budgetCost.GetBudgetValue();
		
		m_fCurrentBudget -= Math.Max(budgetCostValue, GetMinBudgetCost());
		m_fCurrentBudget = Math.Max(m_fCurrentBudget, 0);
	}
};