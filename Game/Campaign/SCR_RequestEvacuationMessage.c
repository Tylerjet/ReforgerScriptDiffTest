//------------------------------------------------------------------------------------------------
class SCR_RequestEvacuationMessage : SCR_RequestMessage
{
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseRadioComponent radio, int freq, float quality, int transcvIdx)
	{
		if (m_RequesterMainBase && radio && radio.GetOwner() == m_RequesterMainBase && GetTaskManager())
		{
			SCR_EvacuateTaskSupportEntity supportEntity = SCR_EvacuateTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_EvacuateTaskSupportEntity));
				if (supportEntity)
					supportEntity.RequestEvacuation(m_iRequesterID, m_vPosition);
		}
	}
};
