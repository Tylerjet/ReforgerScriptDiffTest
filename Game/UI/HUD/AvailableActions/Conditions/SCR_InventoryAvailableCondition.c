[BaseContainerProps()]
class SCR_InventoryAvailableCondition : SCR_AvailableActionsGroupCondition
{
	[Attribute("4")]
	protected int m_iMaxShowCount;
	protected static int s_iShowCounter;

	[Attribute("1")]
	protected bool m_bPersistent;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsInventoryOpen())
			&& (s_iShowCounter < m_iMaxShowCount);
	}

	static void IncrementCounter()
	{
		s_iShowCounter++;
		GetGame().GetGameUserSettings().GetModule("SCR_InventoryHintSettings").Set("m_iInventoryOpenCount", s_iShowCounter);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

	void Reset()
	{
		GetGame().GetGameUserSettings().GetModule("SCR_InventoryHintSettings").Set("m_iInventoryOpenCount", 0);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

	void SCR_InventoryAvailableCondition()
	{
		if (!m_bPersistent)
			Reset();
		GetGame().GetGameUserSettings().GetModule("SCR_InventoryHintSettings").Get("m_iInventoryOpenCount", s_iShowCounter);
	}
};