//------------------------------------------------------------------------------------------------

class SCR_BuildingDedicatedSlotData
{
	protected ECampaignCompositionType m_iCompositionType;
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	void SetData(ECampaignCompositionType compositionType, SCR_CampaignBase base)
	{
		m_iCompositionType = compositionType;
		m_Base = base;
	}
		
	//------------------------------------------------------------------------------------------------
	ECampaignCompositionType GetCompositionType()
	{
		return m_iCompositionType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetBase()
	{
		return m_Base;
	}
};
