class SCR_CampaignTutorialStage48Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage48 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 12;
		string hintString = "#AR-Tutorial_Hint_MotorPool <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_LightVehicleDepot'/></color></h1>";
		
		SCR_HintManagerComponent.ShowCustomHint(hintString, "", 12, isTimerVisible: true);
	}
};