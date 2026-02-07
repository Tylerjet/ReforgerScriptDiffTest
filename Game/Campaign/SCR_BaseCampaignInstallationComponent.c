[EntityEditorProps(category: "GameScripted/Campaign", description: "Base component to be expanded and attached to SCR_BaseCampaignInstallation.", color: "0 0 255 255")]
class SCR_BaseCampaignInstallationComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseCampaignInstallationComponent : ScriptComponent
{
	[Attribute("1", "Should this component be updated only each x seconds by the installation it's attached to? x = SCR_BaseCampaignInstallation.COMPONENTS_UPDATE_TIME")]
	protected bool m_bUpdatedByInstallation;
	
	protected bool m_bUpdate = true;
	
	//------------------------------------------------------------------------------------------------
	bool UpdatedByInstallation()
	{
		return m_bUpdatedByInstallation;
	}
	
	//------------------------------------------------------------------------------------------------
	//Called each x seconds by the installation it's attached to, x = SCR_BaseCampaignInstallation.COMPONENTS_UPDATE_TIME
	void UpdateInstallationComponent(float timeSinceLastUpdate)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseOwnerChanged(SCR_CampaignFaction newOwner)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_BaseCampaignInstallation campaignInstallation = SCR_BaseCampaignInstallation.Cast(owner);
		
		if (campaignInstallation)
			campaignInstallation.RegisterComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BaseCampaignInstallationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseCampaignInstallationComponent()
	{
	}

};
