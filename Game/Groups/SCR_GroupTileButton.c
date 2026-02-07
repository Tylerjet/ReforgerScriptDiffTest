class SCR_GroupTileButton : SCR_ButtonBaseComponent
{
	protected int m_iGroupID;
	protected Faction m_GroupFaction;
	
	[Attribute()]
	protected ResourceName m_textLayout;
	
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_BaseTaskManager m_BaseTaskManager;
	protected SCR_NavigationButtonComponent m_JoinGroupButton;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameSelfColor;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameDeadColor;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_GroupFullColor;
	
	protected ResourceName m_sIconsImageSet = "{1F0A6C9C19E131C6}UI/Textures/Icons/icons_wrapperUI.imageset";

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		
#ifdef DEBUG_GROUPS
		SCR_GroupsManagerComponent.GetInstance().UpdateDebugUI();
#endif
		RefreshPlayers();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void InitiateGroupTile()
	{
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupManager)
			return;
		
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;
		m_BaseTaskManager = GetTaskManager();
		
		group.GetOnMemberStateChange().Insert(RefreshPlayers);
		m_BaseTaskManager.s_OnTaskUpdate.Insert(RefreshPlayers);
		
		RichTextWidget squadName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		if (squadName)
			squadName.SetText(group.GetCallsignSingleString());
		RichTextWidget frequency = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		if (frequency)
			frequency.SetText(""+group.GetGroupFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");
		RichTextWidget playerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		if(playerCount)
			playerCount.SetText(group.GetPlayerCount().ToString() + "/" + group.GetMaxMembers());
		if (group.IsFull())
			SetGroupInfoColor(m_GroupFullColor);
		if (group.IsPlayerInGroup(GetGame().GetPlayerController().GetPlayerId()))
			squadName.SetColor(m_PlayerNameSelfColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshPlayers()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;
		
		if (m_ParentSubMenu == null)
			FindParentMenu();
		
		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast( m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;
		RichTextWidget squadName = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailCallsign"));
		if (!squadName)
			return;
		RichTextWidget frequency = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailFrequency"));
		if (!frequency)
			return;
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;
		ImageWidget groupImage = ImageWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupImage"));
		if (!groupImage)
			return;	
		SCR_PlayerControllerGroupComponent groupMenuComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupMenuComponent)
			return;
		
		SCR_PlayerControllerGroupComponent s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!s_PlayerGroupController)
			return;
		if (!s_PlayerGroupController.CanPlayerJoinGroup(GetGame().GetPlayerController().GetPlayerId() , m_GroupManager.FindGroup(m_iGroupID)) )
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);
		groupMenuComponent.SetSelectedGroupID(m_iGroupID);
		squadName.SetText(group.GetCallsignSingleString());
		frequency.SetText(""+group.GetGroupFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");
		
		int x, y;
		SCR_Faction scrFaction = SCR_Faction.Cast(m_GroupFaction);
		if (!scrFaction)
			return;
		groupImage.LoadImageTexture(0, scrFaction.GetFactionFlag());
		groupImage.GetImageSize(0, x, y);
		groupImage.SetSize(x, y);
		
		Widget children = playerList.GetChildren();
		while (children)
		{
			playerList.RemoveChild(children);
			children = playerList.GetChildren();
		}

		array<int> playerIDs = group.GetPlayerIDs();
		Widget playerTile;
		TextWidget playerName, playerFrequency;
		SCR_GadgetManagerComponent gadgetManager;
		SCR_BaseTaskExecutor taskExecutor;
		ImageWidget taskIcon, muteIcon, background, loadoutIcon;
		SCR_RespawnSystemComponent respawnSystem;
		SCR_BasePlayerLoadout playerLoadout;
		Resource res;
		IEntityComponentSource source;
		BaseContainer container;
		SCR_EditableEntityUIInfo info;
		set<int> frequencies = new set<int>();
		
		foreach (int playerID : playerIDs)
		{
			playerTile = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList));
			playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));
			playerFrequency = TextWidget.Cast(playerTile.FindAnyWidget("Frequency"));
			taskIcon = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIcon"));
			muteIcon = ImageWidget.Cast(playerTile.FindAnyWidget("MuteIcon"));
			background = ImageWidget.Cast(playerTile.FindAnyWidget("Background"));
			loadoutIcon = ImageWidget.Cast(playerTile.FindAnyWidget("LoadoutIcon"));

			playerName.SetText(GetGame().GetPlayerManager().GetPlayerName(playerID));
			
			ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!controlledEntity)
			{
			    playerName.SetColor(m_PlayerNameDeadColor);
				playerFrequency.SetText("-");
			}
			else
			{
				CharacterControllerComponent controller = controlledEntity.GetCharacterController();
				DamageManagerComponent damageManager = controlledEntity.GetDamageManager();
				if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
					playerName.SetColor(m_PlayerNameDeadColor);
				gadgetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
				SCR_Global.GetFrequencies(gadgetManager, frequencies);
				if (!frequencies.IsEmpty())
				{
					playerFrequency.SetText(SCR_FormatHelper.FormatFrequencies(frequencies));
					background.SetOpacity(0.85);
				}
			}
			if (playerID == GetGame().GetPlayerController().GetPlayerId())
				playerName.SetColor(m_PlayerNameSelfColor);
			
			//check for task and adjust the icon visibility
			taskExecutor = SCR_BaseTaskExecutor.FindTaskExecutorByID(playerID);
			if (m_BaseTaskManager && taskExecutor.GetAssignedTask())
				taskIcon.SetOpacity(1);
			
			//set the state of mute
			if (muteIcon && GetGame().GetPlayerController().GetPlayerMutedState(playerID) == PermissionState.DISALLOWED)
			{
				muteIcon.SetColor(m_PlayerNameSelfColor);
				muteIcon.LoadImageFromSet(0, m_sIconsImageSet, "sound-off");
			}
			
			//look for loadout and set the appropriate icon
			if (loadoutIcon)
			{
				respawnSystem = SCR_RespawnSystemComponent.GetInstance();
				playerLoadout = respawnSystem.GetPlayerLoadout(playerID);
				if (playerLoadout)
				{
					res = Resource.Load(playerLoadout.GetLoadoutResource());
					source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
					if (source)
					{
						container = source.GetObject("m_UIInfo");
						info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
						info.SetIconTo(loadoutIcon);
					}
				}
			}
		}
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupInfoColor(Color groupColor)
	{
		ImageWidget frequencyImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("FrequencyImage"));
		ImageWidget playerIcon = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PlayerIcon"));
		TextWidget callsign = TextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		TextWidget frequency = TextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		TextWidget playerCount = TextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		if (!frequencyImage || !playerIcon || !callsign || !frequency || !playerCount)
			return;
		frequencyImage.SetColor(groupColor);
		playerIcon.SetColor(groupColor);
		callsign.SetColor(groupColor);
		frequency.SetColor(groupColor);
		playerCount.SetColor(groupColor);
	}

	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (deviceType == EInputDeviceType.GAMEPAD || deviceType == EInputDeviceType.KEYBOARD)
			RefreshPlayers();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupID(int id)
	{
		m_iGroupID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetGroupFaction()
	{
		return m_GroupFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupFaction(Faction groupFaction)
	{
		m_GroupFaction = groupFaction;
	}
	
	void SetJoinGroupButton(SCR_NavigationButtonComponent joinGroupButton)
	{
		m_JoinGroupButton = joinGroupButton;
	}
};
