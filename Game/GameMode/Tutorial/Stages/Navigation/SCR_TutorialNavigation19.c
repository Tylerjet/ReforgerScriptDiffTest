[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation19Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation19 : SCR_BaseCampaignTutorialArlandStage
{	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_fDuration = 12;
		m_bConditionPassCheck = true;
		
		GetGame().GetCallqueue().CallLater(TeleportFromVehicle, m_fDuration * 1000, false);
		
		IEntity waypoint = GetGame().GetWorld().FindEntityByName("WP_GREENHOUSE");
		if (!waypoint)
			return;
		
		PlaySoundSystem("Navigation_Finish", true);
		
		m_TutorialComponent.SetStagesComplete(3, true);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TeleportFromVehicle()
	{
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("Navigation_car");
		if (!vehicle)
			return;
		
		CompartmentAccessComponent compartmentAccessComp = CompartmentAccessComponent.Cast(vehicle.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccessComp)
			return;
		
		IEntity tpPos = GetGame().GetWorld().FindEntityByName("PP_START");
		if (!tpPos)		
			return;
		
		vector mat[4];
		tpPos.GetTransform(mat);
		compartmentAccessComp.GetOutVehicle_NoDoor(mat, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};