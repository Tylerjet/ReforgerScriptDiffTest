[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture19Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture19 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Conflict_MHQDeploy", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetMobileAssembly() != null);
	}
};