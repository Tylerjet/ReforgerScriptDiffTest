[EntityEditorProps(category: "GameScripted/Sound", description: "Music component", color: "0 0 255 255")]
class SCR_CampaignMusicComponentClass: SCR_MusicComponentClass
{
};
class SCR_CampaignMusicComponent: SCR_MusicComponent
{
	protected bool m_bPlayerIsInHQ = false;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetOnPlayerEnterBase().Insert(PlayerEnteredHQ);
		campaign.GetOnPlayerLeftBase().Insert(PlayerLeftHQ);
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
		
		m_MusicManager.DisableLocalAmbientMusic(true);
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
		
		m_MusicManager.EnableLocalAmbientMusic();
		m_bPlayerIsInHQ = false;
	}
	
	//~ Make sure that if player died in HQ that the system handles that correctly
	protected void ForceLeaveHQ()
	{
		if (!m_bPlayerIsInHQ)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
				
		if (!campaign)
			return;
		
		SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());
		
		if (!pFaction)
			return;
		
		PlayerLeftHQ(pFaction.GetMainBase());
	}
	
	//~ On spawn make sure that they do not concider themselves in HQ
	protected override void OnPlayerDied(int playerId, IEntity player, IEntity killer)
	{	
		if (playerId != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		ForceLeaveHQ();
		super.OnPlayerDied(playerId, player, killer);
	}
	
	protected override void OnPlayerDeleted(int playerId, IEntity player)
	{	
		if (playerId != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		ForceLeaveHQ();
		super.OnPlayerDeleted(playerId, player);
	}
	
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return;
		
		campaign.GetOnPlayerEnterBase().Remove(PlayerEnteredHQ);
		campaign.GetOnPlayerLeftBase().Remove(PlayerLeftHQ);
		
	}
};
