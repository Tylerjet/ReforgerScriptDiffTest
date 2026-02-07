[EntityEditorProps(category: "GameScripted/Commanding", description: "This component should be attached to player controller and is used by commanding to send requests to server.")]
class SCR_PlayerControllerCommandingComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerControllerCommandingComponent : ScriptComponent
{
	[Attribute("{ECC45EC468D76CF4}Configs/Commanding/CommandingMenu.conf")]
	protected ResourceName m_sCommandingMenuConfigPath;
	
	protected ref SCR_PlayerCommandingMenuConfig m_CommandingMenuConfig;
	
	protected IEntity m_SelectedEntity;
	protected SCR_RadialMenuHandler m_RadialMenu;
	
	protected SCR_CommandingManagerComponent m_CommandingManager;
		
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
		
		SetupRadialMenu();
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
	void SetupRadialMenu()
	{
		m_RadialMenu = GetRadialMenuHandler();
		if (!m_RadialMenu)
		{
			GetGame().GetCallqueue().CallLater(SetupRadialMenu, 100);
			return;
		}
		
		m_RadialMenu.m_OnActionPerformed.Insert(OnRadialMenuPerformed);
		m_RadialMenu.onMenuToggleInvoker.Insert(UpdateRadialMenu);
		
		SCR_RadialMenuInteractions interactions = m_RadialMenu.GetRadialMenuInteraction();
		if (!interactions)
			return;
		interactions.GetOnMenuOpenFailed().Insert(OnRadialMenuFailed);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnGroupLeaderChanged);
		
		EnableRadialMenu(false);
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
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		
		bool enabled = playerID == GetGame().GetPlayerController().GetPlayerId();

		EnableRadialMenu(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuPerformed(BaseSelectionMenuEntry entry, int i)
	{
		SCR_CommandingMenuEntry element = SCR_CommandingMenuEntry.Cast(entry);
		if (!element)
			return;
		
		ExecuteCommand(element.GetCommandName());
		//UpdateRadialMenu(null, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuFailed()
	{
		SCR_NotificationsComponent.SendLocal(ENotification.COMMANDING_NO_RIGHTS);
	}
	
	//------------------------------------------------------------------------------------------------
	void ExecuteCommand(string commandName, bool slaveGroupRequested = false)
	{
		if (commandName.IsEmpty())
			return;
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		vector targetPosition;
		GetGame().GetPlayerController().GetPlayerCamera().GetCursorTargetWithPosition(targetPosition);
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupController || !groupManager)
			return;
		
		SCR_AIGroup playersGroup = groupManager.FindGroup(groupController.GetGroupID());
		if (!playersGroup)
			return;
		
		RplComponent rplComp;
		if (!playersGroup.GetSlave() && !slaveGroupRequested)
		{
			rplComp = RplComponent.Cast(playersGroup.FindComponent(RplComponent));
			groupController.RequestCreateSlaveGroup(rplComp.Id());
			slaveGroupRequested = true;
		}
		SCR_AIGroup slaveGroup = playersGroup.GetSlave();
		
		if (!slaveGroup)
		{
			GetGame().GetCallqueue().CallLater(ExecuteCommand, 100, false, commandName, slaveGroupRequested);
			return;
		}
	
		rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		RplId groupRplID = rplComp.Id();
		RplId cursorTargetRplID;
	
  		int commandIndex = commandingManager.FindCommandIndex(commandName);
		
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
	void EnableRadialMenu(bool enable)
	{
		if (!m_RadialMenu) 
			return;
		
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