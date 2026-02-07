[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_ZIGZAG_1");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "ObstacleStart", true);
	}
};