[EntityEditorProps(category: "GameScripted/Campaign", description: "Base campaign installation.", color: "0 0 255 255")]
class SCR_BaseCampaignInstallationClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseCampaignInstallation : GenericEntity
{
	protected ref array<SCR_BaseCampaignInstallationComponent> m_aCampaignComponents;
	
	//------------------------------------------------------------------------------------------------
	void UpdateComponents(float updateTime)
	{
		if (!m_aCampaignComponents)
			return;
		
		for (int i = m_aCampaignComponents.Count() - 1; i >= 0; i--)
		{
			if (!m_aCampaignComponents[i].UpdatedByInstallation())
				continue;
			
			m_aCampaignComponents[i].UpdateInstallationComponent(updateTime);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterComponent(SCR_BaseCampaignInstallationComponent component)
	{
		if (!component)
			return;
		
		if (!m_aCampaignComponents)
			m_aCampaignComponents = new ref array<SCR_BaseCampaignInstallationComponent>();
		
		m_aCampaignComponents.Insert(component);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadState(SCR_CampaignBaseStruct baseStruct)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreState(out SCR_CampaignBaseStruct baseStruct)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseCampaignInstallation(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseCampaignInstallation()
	{
	}

};
