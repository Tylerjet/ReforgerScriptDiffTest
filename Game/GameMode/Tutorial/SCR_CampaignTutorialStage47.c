class SCR_CampaignTutorialStage47Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage47 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_GARAGE");
		m_fWaypointCompletionRadius = 5;
		string custommsg;
				
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom","BuildingPreviewRotationUp") + CreateString("#AR-Keybind_MapPan", "MapPanDrag");
		else
			custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom") + CreateString("#AR-Keybind_MapPan", "MapPanVGamepad", "MapPanHGamepad");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapInfo" + custommsg, "", -1);
		SCR_MapEntity.GetOnMapOpen().Remove(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		string custommsg;
				
		if (switchedToKeyboard)
			custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom","BuildingPreviewRotationUp") + CreateString("#AR-Keybind_MapPan", "MapPanDrag");
		else
			custommsg = CreateString("#AR-Keybind_MapZoom","MapZoom") + CreateString("#AR-Keybind_MapPan","MapPanVGamepad", "MapPanHGamepad");
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MapInfo" + custommsg, duration: -1, isSilent: true);
	}
};