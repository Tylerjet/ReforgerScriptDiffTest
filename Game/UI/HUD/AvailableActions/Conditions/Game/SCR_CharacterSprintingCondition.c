//------------------------------------------------------------------------------------------------
//! Returns true if character is temporarily sprinting for certain time. Ignores toggled sprinting.
[BaseContainerProps()]
class SCR_CharacterSprintingCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Minimum temporary sprinting time to activate this hint", "")]
	private float m_fMinimumTime;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		bool result = data.GetCharacterSprintingTime() > m_fMinimumTime;
		return GetReturnResult(result);
	}
};