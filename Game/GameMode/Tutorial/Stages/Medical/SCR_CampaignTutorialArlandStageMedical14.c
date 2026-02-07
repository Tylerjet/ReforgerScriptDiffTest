[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical14Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical14: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;

		RegisterWaypoint("Figurant");
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_BandageLeg");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;
		
		return !damManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.LEFTLEG, EDamageType.BLEEDING) && !damManager.GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG);
	}
};