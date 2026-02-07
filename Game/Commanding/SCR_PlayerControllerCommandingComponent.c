[EntityEditorProps(category: "GameScripted/Commanding", description: "This component should be attached to player controller and is used by commanding to send requests to server.")]
class SCR_PlayerControllerCommandingComponentClass : ScriptComponentClass
{
}

class SCR_PlayerControllerCommandingComponent : ScriptComponent
{
	[Attribute("{ECC45EC468D76CF4}Configs/Commanding/CommandingMenu.conf")]
	protected ResourceName m_sCommandingMenuConfigPath;
	
	[Attribute("{2FFBD92174DDF3E0}Configs/Commanding/CommandingMapMenu.conf")]
	protected ResourceName m_sCommandingMapMenuConfigPath;
	
	[Attribute()]
    protected ref SCR_RadialMenuController m_RadialMenuController;
	
	protected SCR_RadialMenu m_RadialMenu;
	
	protected ref SCR_PlayerCommandingMenuConfig m_CommandingMenuConfig;
	
	protected IEntity m_SelectedEntity;
	
	protected SCR_CommandingManagerComponent m_CommandingManager;
	protected SCR_MapRadialUI m_MapContextualMenu;
	
	protected string m_sExecutedCommandName;
	
	protected bool m_bIsCommandExecuting = false;
	protected bool m_bSlaveGroupRequested = false;
	protected ref SCR_PhysicsHelper m_PhysicsHelper;
	
	//! Used to offset the position to spawn waypoint so it is not spawned in terrain
	protected static const float ABOVE_TERRAIN_OFFSET = 0.5;

	//! Maximal range for command in meters
	protected static const float COMMANDING_VISUAL_RANGE = 10000;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	static SCR_PlayerControllerCommandingComponent GetPlayerControllerCommandingComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
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
		SetEventMask(owner, EntityEvent.INIT);
			
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		m_CommandingManager = SCR_CommandingManagerComponent.GetInstance();
					
		Resource holder = BaseContainerTools.LoadContainer(m_sCommandingMenuConfigPath);
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CommandingMenuConfig = SCR_PlayerCommandingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		if (!m_RadialMenuController)
			return;
		
		m_RadialMenuController.GetOnTakeControl().Insert(OnControllerTakeControl);
		
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
		
		m_PhysicsHelper.InitPhysicsHelper();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControllerTakeControl(SCR_RadialMenuController controller)
	{
		if (GetGame().GetPlayerController() != GetOwner())
			return;
		
		// Send/update entries in radial menu when control is gained
		controller.UpdateMenuData();
		SetupPlayerRadialMenu(GetOwner());
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	void SetupMapListener()
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;
		
		mapMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RemoveMapListener()
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;
		
		mapMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void SetupPlayerRadialMenu(IEntity owner)
	{
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
    	m_RadialMenu = SCR_RadialMenu.GlobalRadialMenu();	
		if (!m_RadialMenu)
			return;
		
		m_RadialMenu.GetOnBeforeOpen().Insert(OnPlayerRadialMenuOpen);
		m_RadialMenu.GetOnPerform().Insert(OnRadialMenuPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		m_MapContextualMenu = SCR_MapRadialUI.GetInstance();
		if (!m_MapContextualMenu)
			return;
		
		m_MapContextualMenu.GetOnMenuInitInvoker().Insert(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapClose(MapConfiguration config)
	{
		if (!m_MapContextualMenu)
			return;
		
		m_MapContextualMenu.GetOnMenuInitInvoker().Remove(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetupMapRadialMenu()
	{
		if (!m_CommandingMenuConfig || !m_MapContextualMenu)
			return;
		
		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;
				
		AddElementsFromCategoryToMap(rootCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] element
	//! \param[in] worldPos
	void OnMapCommandPerformed(SCR_SelectionMenuEntry element, float[] worldPos)
	{
		SCR_MapMenuCommandingEntry mapEntry = SCR_MapMenuCommandingEntry.Cast(element);
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
	//! \param[in] groupID
	//! \param[in] playerID
	void OnGroupLeaderChanged(int groupID, int playerID)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;
		
		//we do not care if it's outside of our group
		if (playerGroupController.GetGroupID() != groupID)
			return;
		
		bool enabled = playerID == GetGame().GetPlayerController().GetPlayerId();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
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
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] menu
	//! \param[in] entry
	void OnRadialMenuPerformed(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		SCR_SelectionMenuEntry element = SCR_SelectionMenuEntry.Cast(entry);
		if (!element)
			return;
		
		PrepareExecuteCommand(element.GetId());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuFailed()
	{
		SCR_NotificationsComponent.SendLocal(ENotification.COMMANDING_NO_RIGHTS);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	void PrepareExecuteCommand(string commandName, vector targetPosition = "0 0 0")
	{
		//if another command is waiting for trace, do nothing
		if (m_bIsCommandExecuting || commandName.IsEmpty())
			return;
	
		m_sExecutedCommandName = commandName;
		
		if (targetPosition != vector.Zero)
		{
			ExecuteCommand(targetPosition, null);
			return;
		}
		
		PlayerController controller = GetGame().GetPlayerController();
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
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
	//!
	//! \param[in] targetPosition
	//! \param[in] tracedEntity
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
		if (!m_bSlaveGroupRequested && !playersGroup.GetSlave())
		{
			rplComp = RplComponent.Cast(playersGroup.FindComponent(RplComponent));
			groupController.RequestCreateSlaveGroup(rplComp.Id());
			m_bSlaveGroupRequested = true;
		}

		SCR_AIGroup slaveGroup = playersGroup.GetSlave();
		//todo:mour Unlink this, only AI commands should be reliant on slave group, so commands should have that as option, not for all commands		
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
	//!
	//! \param[in] commandIndex
	//! \param[in] cursorTargetID
	//! \param[in] groupRplID
	//! \param[in] targetPoisition
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_RequestExecuteCommand(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPoisition, int playerID)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		commandingManager.RequestCommandExecution(commandIndex, cursorTargetID, groupRplID, targetPoisition ,playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	void CommandExecutedCallback(int commandIndex)
	{
		Rpc(RPC_CommandExecutedCallback, commandIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_CommandExecutedCallback(int commandIndex)
	{
		PlayCommandGesture(commandIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	void PlayCommandGesture(int commandIndex)
	{
		if (commandIndex <= 0)
			return;
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		
		//skip the commanding gesture if player has map open
		if (mapEntity && mapEntity.IsOpen())
			return;
		
		SCR_BaseGroupCommand command = SCR_BaseGroupCommand.Cast(m_CommandingManager.FindCommand(m_CommandingManager.FindCommandNameFromIndex(commandIndex))); 
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
	//!
	//! \param[in] owner
	//! \param[in] isOpen
	void UpdateRadialMenu(IEntity owner, bool isOpen)
	{
		if (!m_RadialMenu || !m_CommandingMenuConfig || !isOpen)
			return;
		
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;
		
		m_SelectedEntity = camera.GetCursorTarget();
		
		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;
		
		AddElementsFromCategory(rootCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] category cannot be null
	//! \param[in] rootCategory
	void AddElementsFromCategory(SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry rootCategory = null)
	{
		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();
		
		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		SCR_SelectionMenuCategoryEntry createdCategory;
		
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				createdCategory = SCR_SelectionMenuCategoryEntry.Cast(AddRadialMenuElement(elementCategory, rootCategory));
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
	//!
	//! \param[in] category cannot be null
	void AddElementsFromCategoryToMap(notnull SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry rootCategory = null)
	{
		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();
		SCR_SelectionMenuCategoryEntry mapEntryCategory = m_MapContextualMenu.AddRadialCategory(category.GetCategoryDisplayText()); // add map category entry
		
		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				if (elementCategory.GetCanShowOnMap())
					AddElementsFromCategoryToMap(elementCategory, mapEntryCategory);
			} 
			else 
			{
				InsertElementToMapRadial(element, category, mapEntryCategory);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] element cannot be null
	//! \param[in] category
	//! \param[in] mapCategory
	void InsertElementToMapRadial(SCR_PlayerCommandingMenuBaseElement element, notnull SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry mapCategory)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		SCR_PlayerCommandingMenuCommand commandElement = SCR_PlayerCommandingMenuCommand.Cast(element);		
		if (!commandingManager.CanShowOnMap(commandElement.GetCommandName()))
			return;
	
		SCR_BaseRadialCommand command = commandingManager.FindCommand(commandElement.GetCommandName());
		
		SCR_MapMenuCommandingEntry mapEntry = new SCR_MapMenuCommandingEntry(commandElement.GetCommandName());
		mapEntry.SetName(commandElement.GetCommandDisplayText());
		if (command)	
			mapEntry.SetIcon(command.GetIconImageset(), command.GetIconName());
		
		m_MapContextualMenu.InsertCustomRadialEntry(mapEntry, mapCategory);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newElement
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddRadialMenuElement(SCR_PlayerCommandingMenuBaseElement newElement, SCR_SelectionMenuCategoryEntry parentCategory = null)
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
	//!
	//! \param[in] category cannot be null
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddCategoryElement(SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_SelectionMenuCategoryEntry newCategory = new SCR_SelectionMenuCategoryEntry();
		
		newCategory.SetName(category.GetCategoryDisplayText());
		
		if (!parentCategory)
		{
			m_RadialMenu.AddCategoryEntry(newCategory);
			return newCategory;
		}
		
		parentCategory.AddEntry(newCategory);
		
		return newCategory;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] command cannot be null
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddCommandElement(SCR_PlayerCommandingMenuCommand command, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return null;
		
		if (!commandingManager.CanShowCommand(command.GetCommandName()))
			return null;
		
		SCR_BaseRadialCommand groupCommand = commandingManager.FindCommand(command.GetCommandName());
		
		SCR_SelectionMenuEntry entry = new SCR_SelectionMenuEntry();

		string displayName = command.GetCommandCustomName();
		if (displayName.IsEmpty())
			displayName = command.GetCommandDisplayText();

		entry.SetName(displayName);
		entry.SetId(command.GetCommandName());
		entry.SetIcon(groupCommand.GetIconImageset(), groupCommand.GetIconName());
		entry.Enable(groupCommand.CanBePerformed());
		
		if (parentCategory)
			parentCategory.AddEntry(entry);
		else
			m_RadialMenu.AddEntry(entry);
		
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerRadialMenuOpen()
	{
		if (!m_RadialMenu || !m_CommandingMenuConfig)
			return;
		
		m_RadialMenu.ClearEntries();
		
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;
		
		m_SelectedEntity = camera.GetCursorTarget();
		
		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;
		
		AddElementsFromCategory(rootCategory);
	}
}
