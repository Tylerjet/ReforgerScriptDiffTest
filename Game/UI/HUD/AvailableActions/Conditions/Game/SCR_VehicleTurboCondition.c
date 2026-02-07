//------------------------------------------------------------------------------------------------
//! Returns true if driver is using temporary turbo for ceratain time. Ignores toggled turbo.
[BaseContainerProps()]
class SCR_VehicleTurboCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Minimum temporary turbo time to activate this hint", "")]
	private float m_fMinimumTime;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		bool result = data.GetCurrentVehicleTurboTime() > m_fMinimumTime;
		return GetReturnResult(result);
	}
};