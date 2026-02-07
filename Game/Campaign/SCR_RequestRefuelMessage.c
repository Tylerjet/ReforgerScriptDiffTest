//------------------------------------------------------------------------------------------------
class SCR_RequestRefuelMessage : SCR_RequestMessage
{
	private Vehicle m_RequesterVehicle;
	
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseRadioComponent radio, int freq, float quality, int transcvIdx)
	{
		if (m_RequesterMainBase && radio && radio.GetOwner() == m_RequesterMainBase)
			GetTaskManager().RequestRefuel(m_iRequesterID, m_RequesterVehicle, m_vPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRequesterVehicle(Vehicle vehicle)
	{
		m_RequesterVehicle = vehicle;
	}
};
