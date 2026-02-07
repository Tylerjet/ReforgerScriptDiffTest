[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage16 : SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_SupplyTruck, m_WP;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		m_bCheckWaypoint = false;
		RegisterWaypoint("TownBaseFarm");
		
		m_SupplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Building_Exit2", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_WP)
			m_WP = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		
		return vector.DistanceSq(m_SupplyTruck.GetOrigin(), m_WP.GetOrigin()) <= 300;
	}
};