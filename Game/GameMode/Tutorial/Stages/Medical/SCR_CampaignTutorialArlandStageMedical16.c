[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical16: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;
		
		RegisterWaypoint("Figurant");;
		
		GetGame().GetCallqueue().Remove(m_TutorialComponent.RefreshVictimBloodLevel);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_SalineApply", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;

		return damManager.GetGroupSalineBagged(ECharacterHitZoneGroup.RIGHTARM) || damManager.GetGroupSalineBagged(ECharacterHitZoneGroup.LEFTARM);
	}
};