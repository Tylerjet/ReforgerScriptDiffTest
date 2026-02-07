//------------------------------------------------------------------------------------------------
//! Returns true if daytime matches the condition
[BaseContainerProps()]
class SCR_DaytimeCondition : SCR_AvailableActionCondition 
{	
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;
	
	[Attribute("2", UIWidgets.EditBox, "Time of day", "")]
	private float m_fValue;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		if (!timeManager) 
			return false;

		bool current = (int)timeManager.GetTimeOfTheDay();
		
		bool result = false;
		
		result = SCR_Comparer<int>.Compare(m_eOperator, current, (int)m_fValue);
		return GetReturnResult(result);
	}
};