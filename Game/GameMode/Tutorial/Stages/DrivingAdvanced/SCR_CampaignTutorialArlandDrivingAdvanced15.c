[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced15 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_13");
		m_fWaypointCompletionRadius = 1000;
		SCR_HintManagerComponent.HideHint();
		m_TutorialComponent.SetWaypointMiscImage("RIGHT", true);
	}
};