//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle speed matches the condition
[BaseContainerProps()]
class SCR_VehicleSpeedCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Speed compare value in km/h", "")]
	private float m_fValue;

	int m_iSpeedID = -1;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle speed matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		// TODO: get from vehicle controller or simulation instead
		SignalsManagerComponent signals = data.GetCurrentVehicleSignals();
		if (!signals)
		{
			m_iSpeedID = -1;
			return false;
		}

		if (m_iSpeedID == -1)
			m_iSpeedID = signals.AddOrFindSignal("speed");

		int speed = 0;
		if (m_iSpeedID != -1)
			speed = signals.GetSignalValue(m_iSpeedID);

		bool result = false;

		result = SCR_Comparer<int>.Compare(m_eOperator, speed, (int)m_fValue);
		return GetReturnResult(result);
	}
};
