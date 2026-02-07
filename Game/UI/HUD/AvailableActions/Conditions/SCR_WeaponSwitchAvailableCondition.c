[BaseContainerProps()]
class SCR_WeaponSwitchAvailableCondition : SCR_AvailableActionsGroupCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsQuickSlotShown());
	}
};