//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		if (!m_Base || m_Base.IsHQ() || !m_Base.IsInitialized())
			return null;
		
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
		if (!faction)
			return null;
		
		// Players of faction not covering this base with radio signal should not be able to capture or prevent capture
		SCR_CampaignFaction cFaction = SCR_CampaignFaction.Cast(faction);
		
		if (!cFaction)
			return null;
		
		if (faction.IsPlayable() && !m_Base.IsHQRadioTrafficPossible(cFaction))
			return null;
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);
		
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		
		if (!campaignBase || campaignBase.IsHQ())
			return;
		
		m_Base = campaignBase;
	}
};
