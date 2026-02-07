[EntityEditorProps(category: "GameScripted/Commanding", description: "This component should be attached to player controller and is used by commanding to send requests to server.")]
class SCR_PlayerControllerCommandingComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerControllerCommandingComponent : ScriptComponent
{
	[Attribute("{ECC45EC468D76CF4}Configs/Commanding/CommandingMenu.conf")]
	protected ResourceName m_sCommandingMenuConfigPath;
	
	[Attribute("{2FFBD92174DDF3E0}Configs/Commanding/CommandingMapMenu.conf")]
	protected ResourceName m_sCommandingMapMenuConfigPath;
	
	protected ref SCR_PlayerCommandingMenuConfig m_CommandingMenuConfig;
	
	protected IEntity m_SelectedEntity;
	protected SCR_RadialMenuHandler m_RadialMenu;
	
	protected SCR_CommandingManagerComponent m_CommandingManager;
	protected SCR_MapContextualMenuUI m_MapContextualMenu;
	
	protected string m_sExecutedCommandName;
	
	protected bool m_bIsCommandExecuting = false;
	protected bool m_bSlaveGroupRequested = false;
	protected ref SCR_PhysicsHelper m_PhysicsHelper;

	
	//used to offset the position to spawn waypoint so it is not spawned in terrain
	const float ABOVE_TERRAIN_OFFSET = 0.5;
	//maximal range for command in meters
	const float COMMANDING_VISUAL_RANGE = 10000;
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerCommandingComponent GetPlayerControllerCommandingComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerControllerCommandingComponent GetLocalPlayerControllerCommandingComponent()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		m_CommandingManager = SCR_CommandingManagerComponent.GetInstance();
				
		if (!groupManager || !m_CommandingManager)
		{
			EnableRadialMenu(false);
			return;
		}
		
		Resource holder = BaseContainerTools.LoadContainer(m_sCommandingMenuConfigPath);
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CommandingMenuConfig = SCR_PlayerCommandingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		SetupPlayerRadialMenu();
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
		
		m_PhysicsHelper.InitPhysicsHelper();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupMapListener()
	{
		SCR_MapContextualMenuUI mapMenu = SCR_MapContextualMenuUI.GetInstance();
		if (!mapMenu)
			return;
		
		mapMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveMapListener()
	{
		SCR_MapContextualMenuUI mapMenu = SCR_MapContextualMenuUI.GetInstance();
		if (!mapMenu)
			return;
		
		mapMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_RadialMenuHandler GetRadialMenuHandler()
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return null;

		array<Managed> entities = {};
		controller.FindComponents(SCR_RadialMenuComponent, entities);
		SCR_RadialMenuComponent comp;
		foreach (Managed entity : entities)
		{
			comp = SCR_RadialMenuComponent.Cast(entity);
			if (comp && comp.m_sInput_Toggle == "OpenCommandMenu")
				return comp.m_pRadialMenu;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupPlayerRadialMenu()
	{
		m_RadialMenu = GetRadialMenuHandler();
		if (!m_RadialMenu)
		{
			//When menu isnt created quick enough try it again
			GetGame().GetCallqueue().CallLater(SetupPlayerRadialMenu, 100);
			return;
		}
		
		m_RadialMenu.m_OnActionPerformed.Insert(OnRadialMenuPerformed);
		m_RadialMenu.onMenuToggleInvoker.Insert(UpdateRadialMenu);
		
		SCR_RadialMenuInteractions interactions = m_RadialMenu.GetRadialMenuInteraction();
		if (!interactions)
			return;

		interactions.GetOnMenuOpenFailed().Insert(OnRadialMenuFailed);
		
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		
		playerGroupController.GetOnGroupChanged().Insert(OnGroupChanged);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnGroupLeaderChanged);
		
		EnableRadialMenu(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen()
	{
		m_MapContextualMenu = SCR_MapContextualMenuUI.GetInstance();
		if (!m_MapContextualMenu)
			return;
		
		m_MapContextualMenu.GetOnMenuOpenInvoker().Insert(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose()
	{
		if (!m_MapContextualMenu)
			return;
		
		m_MapContextualMenu.GetOnMenuOpenInvoker().Remove(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupMapRadialMenu()
	{
		if (!m_CommandingMenuConfig || !m_MapContextualMenu)
			return;
		
		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;
		
		m_MapContextualMenu.AddRadialCategory(rootCategory.GetCategoryDisplayText());
		
		AddElementsFromCategoryToMap(rootCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapCommandPerformed(BaseSelectionMenuEntry element, float[] worldPos)
	{
		SCR_MapMenuEntry mapEntry = SCR_MapMenuEntry.Cast(element);
		if (!mapEntry)
			return;
		
		float height = GetGame().GetWorld().GetSurfaceY(worldPos[0], worldPos[1]);
		
		vector position;
		position[0] = worldPos[0];
		position[1] = height + ABOVE_TERRAIN_OFFSET;
		position[2] = worldPos[1];
		
		PrepareExecuteCommand(mapEntry.GetEntryIdentifier(), position);
		
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupLeaderChanged(int groupID, int playerID)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;
		
		//we do not care if it's outside of our group
		if (playerGroupController.GetGroupID() != groupID)
			return;
		
		bool enabled = playerID == GetGame().GetPlayerController().GetPlayerId();

		EnableRadialMenu(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupChanged(int groupID)
	{
		//check if player is the new leader of the group
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup playersGroup = groupsManager.FindGroup(groupID);
		if (!playersGroup)
			return;
		
		bool enabled = playerGroupController.IsPlayerLeader(GetGame().GetPlayerController().GetPlayerId(), playersGroup);
		
		EnableRadialMenu(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuPerformed(BaseSelectionMenuEntry entry, int i)
	{
		SCR_CommandingMenuEntry element = SCR_CommandingMenuEntry.Cast(entry);
		if (!element)
			return;
		
		PrepareExecuteCommand(element.GetCommandName());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuFailed()
	{
		SCR_NotificationsComponent.SendLocal(ENotification.COMMANDING_NO_RIGHTS);
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareExecuteCommand(string commandName, vector targetPosition = "0 0 0")
	{
		//if another command is waiting for trace, do nothing
		if (commandName.IsEmpty() || m_bIsCommandExecuting)
			return;
	
		m_sExecutedCommandName = commandName;
		
		if (targetPosition != vector.Zero)
		{
			ExecuteCommand(targetPosition, null);
			return;
		}
		
		PlayerController controller = GetGame().GetPlayerController();
		PlayerCamera camera = controller.GetPlayerCamera();
		if (!camera)
			return;
		
		IEntity controlledEntity = controller.GetControlledEntity();
		
		vector mat[4];
		camera.GetTransform(mat);
		vector end = mat[3] + mat[2] * COMMANDING_VISUAL_RANGE;
		
		m_PhysicsHelper = new SCR_PhysicsHelper();
		m_PhysicsHelper.GetOnTraceFinished().Insert(ExecuteCommand);
		
		//prevent other commands being executed while this one is waiting for trace
		//todo:kuceramar: after radialmenurework, make radial menu elements disabled when this is true
		m_bIsCommandExecuting = true;
		m_PhysicsHelper.TraceSegmented(mat[3], end, TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT, EPhysicsLayerDefs.Projectile, controlledEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	void ExecuteCommand(vector targetPosition, IEntity tracedEntity)	
	{
		m_bIsCommandExecuting = false;
		if (m_PhysicsHelper)
		{
			m_PhysicsHelper.GetOnTraceFinished().Remove(ExecuteCommand);
			m_PhysicsHelper = null;
		}
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupController || !groupManager)
			return;
		
		SCR_AIGroup playersGroup = groupManager.FindGroup(groupController.GetGroupID());
		if (!playersGroup)
			return;
		
		RplComponent rplComp;
		if (!playersGroup.GetSlave() && !m_bSlaveGroupRequested)
		{
			rplComp = RplComponent.Cast(playersGroup.FindComponent(RplComponent));
			groupController.RequestCreateSlaveGroup(rplComp.Id());
			m_bSlaveGroupRequested = true;
		}
		SCR_AIGroup slaveGroup = playersGroup.GetSlave();
		
		if (!slaveGroup)
		{
			//if there is not slaveGroup, we try to execute the command later because newly created group takes a bit of time to replicate
			GetGame().GetCallqueue().CallLater(ExecuteCommand, 100, false, targetPosition, tracedEntity);
			return;
		}
	
		m_bSlaveGroupRequested = false;
		rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		RplId groupRplID = rplComp.Id();
		RplId cursorTargetRplID;
	
  		int commandIndex = commandingManager.FindCommandIndex(m_sExecutedCommandName);
		
		if (m_SelectedEntity)
		{
			rplComp = RplComponent.Cast(m_SelectedEntity.FindComponent(RplComponent));
			if (rplComp)
				cursorTargetRplID = rplComp.Id();
		}
		Rpc(RPC_RequestExecuteCommand, commandIndex, cursorTargetRplID, groupRplID, targetPosition, playerID);
	}
	
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_RequestExecuteCommand(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPoisition, int playerID)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		commandingManager.RequestCommandExecution(commandIndex, cursorTargetID, groupRplID, targetPoisition ,playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void CommandExecutedCallback(int commandIndex)
	{
		Rpc(RPC_CommandExecutedCallback, commandIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_CommandExecutedCallback(int commandIndex)
	{
		PlayCommandGesture(commandIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayCommandGesture(int commandIndex)
	{
		if (commandIndex <= 0)
			return;
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		
		//skip the commanding gesture if player has map open
		if (mapEntity && mapEntity.IsOpen())
			return;
		
		SCR_BaseGroupCommand command = m_CommandingManager.FindCommand(m_CommandingManager.FindCommandNameFromIndex(commandIndex)); 
		if (!command)
			return;
		int gestureID = command.GetCommandGestureID();
		
		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		
		if (!playerControlledEntity)
			return;
		
		SCR_CharacterControllerComponent characterComponent = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterComponent)
			return;
		
		characterComponent.TryStartCharacterGesture(gestureID, 3000);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRadialMenu(IEntity owner, bool isOpen)
	{
		if (!m_RadialMenu || !m_CommandingMenuConfig || !isOpen)
			return;
		
		PlayerCamera camera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!camera)
			return;
		
		m_SelectedEntity = camera.GetCursorTarget();
		
		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;
		
		AddElementsFromCategory(rootCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddElementsFromCategory(SCR_PlayerCommandingMenuCategoryElement category, BaseSelectionMenuCategory rootCategory = null )
	{
		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();
		
		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		BaseSelectionMenuCategory createdCategory;
		
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				createdCategory = BaseSelectionMenuCategory.Cast(AddRadialMenuElement(elementCategory, rootCategory));
				if (!createdCategory)
					continue;
				
				AddElementsFromCategory(elementCategory, createdCategory);
			} 
			else 
			{
				AddRadialMenuElement(element, rootCategory);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddElementsFromCategoryToMap(notnull SCR_PlayerCommandingMenuCategoryElement category)
	{
		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();
		
		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				//TODO@kuceramar: map menu doesnt support subcategories right now
				InsertElementToMapRadial(element, category, true);
			} 
			else 
			{
				InsertElementToMapRadial(element, category);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertElementToMapRadial(SCR_PlayerCommandingMenuBaseElement element, notnull SCR_PlayerCommandingMenuCategoryElement category, bool isCategory = false)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		SCR_PlayerCommandingMenuCommand commandElement = SCR_PlayerCommandingMenuCommand.Cast(element);
				
		if (!commandingManager.CanShowOnMap(commandElement.GetCommandName()))
			return;
		
		SCR_MapMenuEntry mapEntry;
		SCR_MapMenuCategory mapCategory;
		
		if (isCategory)
		{
			mapCategory = new SCR_MapMenuCategory(commandElement.GetCommandDisplayText());
			if (!mapCategory)
				return;
		}
		else 
		{
			mapEntry = new SCR_MapMenuEntry(commandElement.GetCommandDisplayText(), category.GetCategoryDisplayText(), commandElement.GetCommandName());
			if (!mapEntry)
				return;
		}
			
		SCR_BaseGroupCommand command = commandingManager.FindCommand(commandElement.GetCommandName());
		if (command)	
			mapEntry.SetIcon(command.GetIconImageset(), command.GetIconName());
		
		if (isCategory)
			m_MapContextualMenu.InsertCustomRadialCategory(mapCategory, category.GetCategoryDisplayText());
		else
			m_MapContextualMenu.InsertCustomRadialEntry(mapEntry, category.GetCategoryDisplayText());
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableRadialMenu(bool enable)
	{
		if (!m_RadialMenu)
		{
			//When menu isnt created quick enough try it again
			GetGame().GetCallqueue().CallLater(SetupPlayerRadialMenu, 100);
			return;
		}
		
		SCR_RadialMenuInteractions interactions = m_RadialMenu.GetRadialMenuInteraction();
		if (!interactions)
			return;

		interactions.SetCanOpenMenu(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuEntry AddRadialMenuElement(SCR_PlayerCommandingMenuBaseElement newElement, BaseSelectionMenuCategory parentCategory = null)
	{
		SCR_PlayerCommandingMenuCommand commandElement = SCR_PlayerCommandingMenuCommand.Cast(newElement);
		if (commandElement)
			return AddCommandElement(commandElement, parentCategory);
		
		SCR_PlayerCommandingMenuCategoryElement categoryElement = SCR_PlayerCommandingMenuCategoryElement.Cast(newElement);
		if (categoryElement)
			return AddCategoryElement(categoryElement, parentCategory);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuEntry AddCategoryElement(SCR_PlayerCommandingMenuCategoryElement category, BaseSelectionMenuCategory parentCategory = null)
	{
		BaseSelectionMenuCategory newCategory = new BaseSelectionMenuCategory();
		
		newCategory.SetCategoryName(category.GetCategoryDisplayText());
		newCategory.SetName(category.GetCategoryDisplayText());
		
		if (!parentCategory)
		{
			m_RadialMenu.AddElementToMenuMap(newCategory, parentCategory);
			return newCategory;
		}
		
		array<ref BaseSelectionMenuCategory> categories = m_RadialMenu.GetCategoriesList();
		
		foreach (BaseSelectionMenuCategory menuCategory : categories)
		{
			if (menuCategory.GetCategoryName() == parentCategory.GetCategoryName())
			{
				m_RadialMenu.AddElementToMenuMap(newCategory, menuCategory);
				return newCategory;
			}
		}	
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuEntry AddCommandElement(SCR_PlayerCommandingMenuCommand command, BaseSelectionMenuCategory parentCategory = null)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return null;
		
		if (!commandingManager.CanShowCommand(command.GetCommandName()))
			return null;
		
		SCR_CommandingMenuEntry element = new SCR_CommandingMenuEntry();
			
		string displayName = command.GetCommandCustomName();
		if (displayName.IsEmpty())
			displayName = command.GetCommandDisplayText();
				
		element.SetName(displayName);
		element.SetCommandName(command.GetCommandName());
		
		m_RadialMenu.AddElementToMenuMap(element, parentCategory);
		return element;
	}
}