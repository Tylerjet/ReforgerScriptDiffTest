class SCR_CampaignTutorialStage26Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage26 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_BOARDING");
		m_bCheckWaypoint = false;
		IEntity jeep = GetGame().GetWorld().FindEntityByName("Jeep");
		m_WaypointEntity.SetOrigin(jeep.GetOrigin());
		GetGame().GetCallqueue().CallLater(DelayedPopup, 1000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_WeaponLowering", 12, "", "", "<color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'CharacterRaiseWeapon'/></shadow></color>", "");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Boarding" + CreateString("#AR-Editor_CommandAction_AIWaypoint_GetInNearest_Name","CharacterAction"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.CheckCharacterInVehicle(ECompartmentType.Pilot));
	}
};