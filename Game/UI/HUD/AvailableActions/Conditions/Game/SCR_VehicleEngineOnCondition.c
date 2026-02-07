//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle engine is on
[BaseContainerProps()]
class SCR_VehicleEngineOnCondition : SCR_AvailableActionCondition
{
	protected int m_iEngineOnID = -1;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		// TODO: get from vehicle controller or simulation instead
		SignalsManagerComponent signals = data.GetCurrentVehicleSignals();
		if (!signals)
			return true;
		
		if (m_iEngineOnID == -1)
			m_iEngineOnID = signals.AddOrFindSignal("engineOn");
		
		float engineOn = 0;
		if (m_iEngineOnID != -1)
			engineOn = signals.GetSignalValue(m_iEngineOnID);

		bool result = (int)engineOn;
		return GetReturnResult(result);
	}
};