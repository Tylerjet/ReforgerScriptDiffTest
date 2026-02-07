[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDriving1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDriving1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 9;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "Start", true);

		m_TutorialComponent.ResetStage_VehiclesSimple();
	}
};