//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		if (!m_Base || m_Base.GetIsHQ())
			return null;
		
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
		if (!faction)
			return null;
		
		// Players of faction not covering this base with radio signal should not be able to capture or prevent capture
		SCR_CampaignFaction cFaction = SCR_CampaignFaction.Cast(faction);
		
		if (!cFaction)
			return null;
		
		if (faction.IsPlayable() && !m_Base.IsBaseInFactionRadioSignal(cFaction))
			return null;
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_Base = SCR_CampaignBase.Cast(GetOwner());
		
		if (!m_Base)
		{
			Print("SCR_CampaignSeizingComponent: Parent base not found! Terminating...", LogLevel.ERROR);
			return;
		}
		
		if (m_Base.GetIsHQ())
			return;
		
		super.OnPostInit(owner);
	}
};
