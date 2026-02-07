[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical11Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical11: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		RegisterWaypoint("Figurant");

		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_TourniquetApply", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;
		
		return damManager.GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG) || !damManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.LEFTLEG, EDamageType.BLEEDING);
	}
};