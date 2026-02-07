[BaseContainerProps()]
class SCR_TutorialLogic_CombatEngineering : SCR_BaseTutorialCourseLogic

{
	bool m_bBuildingVoiceLinePlayed;
	//------------------------------------------------------------------------------------------------
	void HandleArlevilleSupplies(bool refill)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("ARLEVILLE_SUPPLY_STORAGE");
		if (!ent)
			return;

		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(ent);
		if (!resComp)
			return;

		SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
		if (!resourceContainer)
			return;
		
		resourceContainer.DepleteResourceValue();
		
		if (refill)
			resourceContainer.IncreaseResourceValue(810);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{	
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity service = GetGame().GetWorld().FindEntityByName("VEHICLE_REQUESTING_BOARD");
		if (service)
		{
			SCR_CampaignBuildingProviderComponent buildingComponent = SCR_CampaignBuildingProviderComponent.Cast(service.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (buildingComponent)
				buildingComponent.RemovePlayerCooldowns(1);
		}
		
		SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE"));
		SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("BUILDING_GUNNEST"));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{	
		m_bBuildingVoiceLinePlayed = false;
		HandleArlevilleSupplies(false);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE");
		if (!vehicle)
			return;
		
		tutorial.ChangeVehicleLockState(vehicle, true);
		tutorial.InsertIntoGarbage(vehicle);
	}
}