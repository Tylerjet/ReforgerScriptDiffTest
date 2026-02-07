[BaseContainerProps()]
class SCR_TutorialLogic_SquadLeadership : SCR_BaseTutorialCourseLogic
{	

	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("UNIT_REQUESTING_SUPPLIES");
		if (!ent)
			return;

		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(ent);
		if (!resComp)
			return;

		SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
		if (resourceContainer)
			resourceContainer.IncreaseResourceValue(150);
		
		HandleRemnantGroup("OldGroup");
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (commandingManager)
			commandingManager.SetMaxAIPerGroup(2);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		IEntity playerVeh = GetGame().GetWorld().FindEntityByName("PlayerVehicle");
		if (tutorial && playerVeh)
			tutorial.ChangeVehicleLockState(playerVeh, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleRemnantGroup(string groupName)
	{
		IEntity oldGroupEnt = GetGame().GetWorld().FindEntityByName(groupName);
		
		SCR_AIGroup group = SCR_AIGroup.Cast(oldGroupEnt);
		if (!group)
			return;
		
		array <AIAgent> agents = {};
		group.GetAgents(agents);
		
		if (!agents || agents.IsEmpty())
			return;
		
		for (int i = agents.Count()-1; i >= 0; i--)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(agents[i].GetControlledEntity());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(GetGame().GetWorld().FindEntityByName("UNIT_REQUESTING_SUPPLIES"));
		if (resComp)
		{
			SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
			if (resourceContainer)
				resourceContainer.DecreaseResourceValue(resourceContainer.GetResourceValue());
		}
		
		HandleRemnantGroup("REQUESTING_GROUP");
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (commandingManager)
			commandingManager.SetMaxAIPerGroup(0);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		IEntity playerVeh = GetGame().GetWorld().FindEntityByName("PlayerVehicle");
		if (tutorial && playerVeh)
			tutorial.ChangeVehicleLockState(playerVeh, false);
		
		//TODO: GIVE group name and use HandleRemnantGroup, to get rid of redundant code
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!group)
			return;
		
		group = group.GetSlave();
		if (!group)
			return;
		
		group.RemoveWaypoint(group.GetCurrentWaypoint());
		
		array <AIAgent> agents = {};
		group.GetAgents(agents);
		
		if (!agents || agents.IsEmpty())
			return;
		
		for (int i = agents.Count()-1; i >= 0; i--)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(agents[i].GetControlledEntity());
		}
	}
}