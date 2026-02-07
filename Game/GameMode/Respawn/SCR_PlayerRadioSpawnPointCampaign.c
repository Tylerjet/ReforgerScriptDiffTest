[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerRadioSpawnPointCampaignClass: SCR_PlayerRadioSpawnPointClass
{
};
class SCR_PlayerRadioSpawnPointCampaign: SCR_PlayerRadioSpawnPoint
{
	//------------------------------------------------------------------------------------------------
	Faction GetCachedFaction()
	{
		return m_CachedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateSpawnPointPublic()
	{
		super.ActivateSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateSpawnPointPublic()
	{
		super.DeactivateSpawnPoint();
	}
};