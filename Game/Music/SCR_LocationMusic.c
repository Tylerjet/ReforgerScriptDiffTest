class SCR_LocationMusic : LocationMusic
{
	protected bool m_bPlayerIsInHQ = false;
	
	
	override void Init() 
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
	
		if (!campaign)
			return;
		
		campaign.GetOnPlayerEnterBase().Insert(PlayerEnteredHQ);
		campaign.GetOnPlayerLeftBase().Insert(PlayerLeftHQ);
	}

	override void OnDelete()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return;
		
		campaign.GetOnPlayerEnterBase().Remove(PlayerEnteredHQ);
		campaign.GetOnPlayerLeftBase().Remove(PlayerLeftHQ);
	}
		
	//~ Player entered their faction HQ
	protected void PlayerEnteredHQ(SCR_CampaignBase base)
	{
		if (m_bPlayerIsInHQ)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
				
		if (!campaign)
			return;
		
		SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());
		
		if (!pFaction)
			return;
		
		SCR_CampaignBase HQ = pFaction.GetMainBase();
		
		//~ No HQ or the given base is not HQ then ignore
		if (!HQ || base != HQ)
			return;
	
		m_bPlayerIsInHQ = true;
	}
	
	//~ Player left their faction HQ
	protected void PlayerLeftHQ(SCR_CampaignBase base)
	{
		if (!m_bPlayerIsInHQ)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
				
		if (!campaign)
			return;
		
		SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());
		
		if (!pFaction)
			return;
		
		SCR_CampaignBase HQ = pFaction.GetMainBase();
		
		//~ No HQ or the given base is not HQ then ignore
		if (!HQ || base != HQ)
			return;
		
		m_bPlayerIsInHQ = false;
	}
	
	override bool ShouldPlay()
	{	
		return !m_bPlayerIsInHQ;
	}
}