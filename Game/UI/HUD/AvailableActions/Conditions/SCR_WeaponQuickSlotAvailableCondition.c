[BaseContainerProps()]
class SCR_WeaponQuickSlotAvailableCondition : SCR_AvailableActionsGroupCondition
{
	[Attribute("4")]
	protected int m_iMaxShowCount;
	protected static int s_iShowCounter;

	[Attribute("1")]
	protected bool m_bPersistent;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsQuickSlotAvailable())
			&& (s_iShowCounter < m_iMaxShowCount);
	}

	static void IncrementCounter()
	{
		s_iShowCounter++;
	}

	void Reset()
	{
		GetGame().GetGameUserSettings().GetModule("SCR_InventoryHintSettings").Set("m_iQuickSlotShowCount", 0);
		GetGame().UserSettingsChanged();
	}

	void SCR_WeaponQuickSlotAvailableCondition()
	{
		if (!m_bPersistent)
			Reset();
		GetGame().GetGameUserSettings().GetModule("SCR_InventoryHintSettings").Get("m_iQuickSlotShowCount", s_iShowCounter);
	}
};