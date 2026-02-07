[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical15: SCR_BaseCampaignTutorialArlandStage
{
	SCR_CharacterInventoryStorageComponent m_PlayerInventoryManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;
		m_bConditionPassCheck = true;
		
		RegisterWaypoint("ambulance");
		
		m_PlayerInventoryManager = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_Saline");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};