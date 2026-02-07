[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_HeldItemAvailableActionContext : SCR_AvailableActionContext
{
	protected LocalizedString m_sItemName;
	protected IEntity m_LastEntity;

	//------------------------------------------------------------------------------------------------
	override string GetUIName()
	{
		if (m_sItemName == string.Empty)
			return super.GetUIName();

		return SCR_StringHelper.Translate(m_sName, m_sItemName);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		IEntity heldItem = data.GetCurrentItemEntity();
		if (!heldItem)
			heldItem = data.GetCurrentWeaponEntity();

		if (!heldItem)
			return false;

		if (m_LastEntity != heldItem)
			m_bActivated = false;

		if (!super.IsAvailable(data))
			return false;

		if (m_LastEntity == heldItem)
			return true;

		InventoryItemComponent itemIIC = InventoryItemComponent.Cast(heldItem.FindComponent(InventoryItemComponent));
		if (!itemIIC)
			return true;

		UIInfo itemUiInfo = itemIIC.GetUIInfo();
		if (!itemUiInfo)
			return true;

		m_LastEntity = heldItem;
		m_sItemName = itemUiInfo.GetName();
		return true;
	}
}