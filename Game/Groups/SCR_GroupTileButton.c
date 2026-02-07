class SCR_PlayerTileButtonComponent : SCR_ScriptedWidgetComponent
{
	protected SCR_ComboBoxComponent m_OptionsCombo;
	protected int m_iPlayerId;
	protected SCR_ChimeraCharacter m_AICharacter;

	protected ref ScriptInvokerInt m_OnTileFocus;
	protected ref ScriptInvokerInt m_OnTileFocusLost;
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// TODO: on self, this combo box is empty and invisible. On others allows inviting to your group
		m_OptionsCombo.OpenList();
		return super.OnClick(w, x, y, button);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{	
		if (m_OnTileFocus)
			m_OnTileFocus.Invoke(m_iPlayerId);
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_OnTileFocusLost)
			m_OnTileFocusLost.Invoke(m_iPlayerId);
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnTileFocus()
	{
		if (!m_OnTileFocus)
			m_OnTileFocus = new ScriptInvokerInt();
		
		return m_OnTileFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnTileFocusLost()
	{
		if (!m_OnTileFocusLost)
			m_OnTileFocusLost = new ScriptInvokerInt();
		
		return m_OnTileFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTilePlayerID()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	void SetTilePlayerID(int playerID)
	{
		m_iPlayerId = playerID;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ChimeraCharacter GetCharacter()
	{
		return m_AICharacter;
	}

	//------------------------------------------------------------------------------------------------
	void SetCharacter(SCR_ChimeraCharacter character)
	{
		m_AICharacter = character;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ComboBoxComponent GetOptionsComboComponent()
	{
		return m_OptionsCombo;
	}

	//------------------------------------------------------------------------------------------------
	void SetOptionsComboBoxComponent(SCR_ComboBoxComponent optionsCombo)
	{
		m_OptionsCombo = optionsCombo;
	}

	//------------------------------------------------------------------------------------------------
	void SetOpacity(float opacity)
	{
		Widget comboBox = m_OptionsCombo.GetRootWidget();
		if (!comboBox)
			return;

		ImageWidget comboImage = ImageWidget.Cast(comboBox.FindAnyWidget("Image0"));
		if (!comboImage)
			return;

		comboImage.SetOpacity(opacity);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_GroupTileButton : SCR_ButtonBaseComponent
{
	protected int m_iGroupID;
	protected Faction m_GroupFaction;

	[Attribute()]
	protected ResourceName m_textLayout;

	[Attribute()]
	protected ref SCR_UIInfo m_LeaderInfo;

	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_BaseTaskManager m_BaseTaskManager;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_PlayerControllerGroupComponent m_GroupComponent;
	protected SCR_PlayerFactionAffiliationComponent m_sPlyFactionAffilComp;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameSelfColor;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameDeadColor;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_GroupFullColor;

	protected RichTextWidget wGroupTaskName;

	protected const string OPTIONS_COMBO_INVITE = "#AR-PlayerList_Invite";
	protected const string OPTIONS_COMBO_KICK = "#AR-DeployMenu_Groups_Kick";
	protected const string OPTIONS_COMBO_PROMOTE = "#AR-DeployMenu_Groups_Promote";
	protected const string LOCK_GROUP = "#AR-Player_Groups_Lock";
	protected const string UNLOCK_GROUP = "#AR-Player_Groups_Unlock";
	protected const string PRIVATE_GROUP = "#AR-Player_Groups_Private";
	protected const string PUBLIC_GROUP = "#AR-Player_Groups_Public";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string REQUEST_JOIN_GROUP = "#AR-DeployMenu_RequestJoinGroup";
	protected const string CUSTOM_GROUP_NAME_FORMAT = "#AR-Player_Groups_CustomName_Format"; 

	protected const ResourceName GROUP_FLAG_SELECTION = "{7340FE3C6872C6D3}UI/layouts/Menus/GroupSlection/GroupFlagSelection.layout";

	protected ref array<SCR_PlayerTileButtonComponent> m_aPlayerComponentsList = {};
	protected SCR_PlayerTileButtonComponent m_PlayerTileComponent;
	protected static ref ScriptInvoker s_OnGroupButtonClicked = new ScriptInvoker();
	
	protected static ref ScriptInvokerInt m_OnPlayerTileFocus;
	protected static ref ScriptInvokerInt m_OnPlayerTileFocusLost;
	
	protected bool m_bIsPriority;

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

#ifdef DEBUG_GROUPS
		SCR_GroupsManagerComponent.GetInstance().UpdateDebugUI();
#endif
		m_GroupComponent.SetSelectedGroupID(m_iGroupID);
		s_OnGroupButtonClicked.Invoke();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void InitiateGroupTile()
	{
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupManager)
			return;

		m_GroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_GroupComponent)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SocialComponent  socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;


		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		group.GetOnPlayerLeaderChanged().Insert(SetupJoinGroupButton);
		group.GetOnJoinPrivateGroupRequest().Insert(SetupJoinGroupButton);
		
		//workaround for issues with playerControllerGroupsComponent, needs to be reworked with this whole script mess
		SCR_GroupSubMenu.Init();
		SCR_GroupSubMenu.GetOnJoingGroupRequestSent().Insert(SetupJoinGroupButton);

		m_BaseTaskManager = GetTaskManager();
		if (m_BaseTaskManager)
			m_BaseTaskManager.s_OnTaskUpdate.Insert(RefreshPlayers);

		group.GetOnMemberStateChange().Insert(RefreshPlayers);

		RichTextWidget squadName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		if (squadName)
		{
			string groupName = group.GetCustomNameWithOriginal();
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);

			if (groupName.IsEmpty() || group.GetNameAuthorID() != 0 && socialComp.IsRestricted(group.GetNameAuthorID(), EUserInteraction.UserGeneratedContent))
			{
				squadName.SetTextFormat(WidgetManager.Translate(format, company, platoon, squad, character));
			}
			else
			{
				string customName = group.GetCustomName().Trim();
				
				if (customName.IsEmpty())
					squadName.SetText(WidgetManager.Translate(format, company, platoon, squad, character));
				else 
					squadName.SetTextFormat(CUSTOM_GROUP_NAME_FORMAT, customName, WidgetManager.Translate(format, company, platoon, squad, character));
				
				//Translate is used because to change language the group menu has to be closed and then opened so it will renew.
			}
		}

		RichTextWidget frequency = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		if (frequency)
			frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		RichTextWidget playerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		if (playerCount)
			playerCount.SetText(group.GetPlayerCount().ToString() + "/" + group.GetMaxMembers());

		if (group.IsFull())
			SetGroupInfoColor(m_GroupFullColor);

		ImageWidget background = ImageWidget.Cast(GetRootWidget().FindAnyWidget("Background"));

		if (background)
			background.SetVisible(m_GroupComponent.GetSelectedGroupID() == m_iGroupID);

		if (group.IsPlayerInGroup(playerController.GetPlayerId()))
			squadName.SetColor(m_PlayerNameSelfColor);

		ImageWidget privateIcon = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PrivateIcon"));
		if (privateIcon)
			privateIcon.SetVisible(group.IsPrivate());

		if (!m_ParentSubMenu)
			FindParentMenu();
	}

	//------------------------------------------------------------------------------------------------
	void SetupSelectGroupFlagButton(SCR_AIGroup group)
	{
		if (!m_ParentSubMenu)
			return;

		Widget groupImage = m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupImage");
		if (!groupImage)
		return;

		SCR_GroupFlagImageComponent imageButton = SCR_GroupFlagImageComponent.Cast(groupImage.FindHandler(SCR_GroupFlagImageComponent));
		if (!imageButton)
			return;
		
		imageButton.SetVisible(true);
		
		imageButton.m_OnClicked.Insert(OnSelectGroupFlagButtonClicked);

		SCR_Faction scrFaction = SCR_Faction.Cast(m_GroupFaction);
		if (!scrFaction)
			return;
		
		ResourceName flag = group.GetGroupFlag();
		
		if (flag.IsEmpty())
		{
			array<ResourceName> textures = {};
			
			scrFaction.GetGroupFlagTextures(textures);
			
			if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
			{
				array<string> names = {};
			
				scrFaction.GetFlagNames(names);
								
				imageButton.SetFlagButtonFromImageSet(names[0]);
			}
			else if (!textures.IsEmpty())
			{
				imageButton.SetImage(textures[0]);
			}
			else
			{
				array<ResourceName> images = {};
			
				scrFaction.GetGroupFlagTextures(images);
			
				if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
					{
					array<string> names = {};
			
					scrFaction.GetFlagNames(names);
						
					if (!names.IsEmpty())
						imageButton.SetFlagButtonFromImageSet(names[0]);
				}
				else if (!images.IsEmpty())
				{
					imageButton.SetImage(images[0]);
				}
				else
				{
					imageButton.SetImage(scrFaction.GetFactionFlag());			
				}
			}
		}
		else 
		{
			if (group.GetFlagIsFromImageSet())
			{
				imageButton.SetFlagButtonFromImageSet(flag);
			}
			else
			{
				imageButton.SetImage(flag);				
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSelectGroupFlagButtonClicked()
	{
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerComponent)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		if (!group.IsPlayerLeader(playerComponent.GetPlayerID()))
		{
			SCR_NotificationsComponent.SendLocal(ENotification.GROUPS_PLAYER_IS_NOT_LEADER);
			return;
		}
		
		SCR_Faction playerFaction = SCR_Faction.Cast(group.GetFaction());
		
		array<string> flagNames = {};
		array<ResourceName> flagTextures = {};
		playerFaction.GetFlagNames(flagNames);
		playerFaction.GetGroupFlagTextures(flagTextures);
		
		
	    if (!flagNames.IsEmpty() || !flagTextures.IsEmpty())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupFlagDialog, DialogPriority.CRITICAL, 0, true);
		else
			SCR_NotificationsComponent.SendLocal(ENotification.GROUPS_NO_FLAGS);
	}

	//------------------------------------------------------------------------------------------------
	void RefreshPlayers()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SetupSelectGroupFlagButton(group);

		if (m_ParentSubMenu == null)
			FindParentMenu();

		m_aPlayerComponentsList.Clear();

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		VerticalLayoutWidget leaderList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Leader"));
		if (!leaderList)
			return;

		RichTextWidget squadName = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailCallsign"));
		if (!squadName)
			return;

		RichTextWidget description = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Description"));
		if (!description)
			return;

		RichTextWidget frequency = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailFrequency"));
		if (!frequency)
			return;


		SCR_PlayerControllerGroupComponent s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!s_PlayerGroupController)
			return;
		
		SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;
		
		if (!s_PlayerGroupController.CanPlayerJoinGroup(playerController.GetPlayerId(), m_GroupManager.FindGroup(m_iGroupID)))
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);

		string groupName = group.GetCustomName();
		string company, platoon, squad, character, format;
		group.GetCallsigns(company, platoon, squad, character, format);
		
		if (groupName.IsEmpty() || group.GetNameAuthorID() != 0 && socialComp.IsRestricted(group.GetNameAuthorID(), EUserInteraction.UserGeneratedContent))
		{
			squadName.SetTextFormat(WidgetManager.Translate(format, company, platoon, squad, character));
		}
		else
		{
			string customName = group.GetCustomName().Trim();
			
			if (customName.IsEmpty())
				squadName.SetTextFormat(WidgetManager.Translate(format, company, platoon, squad, character));
			else 
				squadName.SetTextFormat(CUSTOM_GROUP_NAME_FORMAT, customName, WidgetManager.Translate(format, company, platoon, squad, character));
		}

		if (!group.GetCustomDescription().IsEmpty())
			description.SetText(group.GetCustomDescription());
		else
			description.SetText(string.Empty);

		CheckLeaderOptions();

		frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		Widget children = playerList.GetChildren();
		while (children)
		{
			playerList.RemoveChild(children);
			children = playerList.GetChildren();
		}

		children = leaderList.GetChildren();
		while (children)
		{
			leaderList.RemoveChild(children);
			children = leaderList.GetChildren();
		}

		array<int> playerIDs = group.GetPlayerIDs();
		Widget playerTile;

		int leaderID = group.GetLeaderID();
		if (leaderID >= 0)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, leaderList);
			SetupPlayerTile(playerTile, leaderID);
		}

		foreach (int playerID : playerIDs)
		{
			if (playerID == leaderID)
				continue;

			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			SetupPlayerTile(playerTile, playerID);
		}

		//disable this for now as it has issues with replication of aigroups
		//ShowAIsInGroup();

		SetupJoinGroupButton();

		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	void SetupJoinGroupButton()
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_AIGroup group;
		if (playerGroupController.GetSelectedGroupID() == -1)
		{
			group = m_GroupManager.FindGroup(m_iGroupID);
		}
		else
		{
			group = m_GroupManager.FindGroup(playerGroupController.GetSelectedGroupID());
		}

		if (!group)
			return;

		m_JoinGroupButton.m_OnActivated.Clear();

		if (group.IsPrivate())
		{
			m_JoinGroupButton.SetLabel(REQUEST_JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.RequestJoinPrivateGroup);
		}
		else
		{
			m_JoinGroupButton.SetLabel(JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.JoinSelectedGroup);
		}

		array<int> denied = {};
		group.GetDeniedRequesters(denied);
		array<int> requesters = {};
		group.GetRequesterIDs(requesters);

		bool requesterDenied = denied.Contains(playerGroupController.GetPlayerID());
		bool requestSent = requesters.Contains(playerGroupController.GetPlayerID());

		if (
				requesterDenied ||
				requestSent ||
				!m_GroupManager.FindGroup(m_iGroupID) ||
				!playerGroupController.CanPlayerJoinGroup( SCR_PlayerController.GetLocalPlayerId() ,m_GroupManager.FindGroup(m_iGroupID)))
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//TODO: setup should be taken care of by the player tile component
	void SetupPlayerTile(Widget playerTile, int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SCR_GadgetManagerComponent gadgetManager;
		TextWidget playerName, playerFrequency;
		ImageWidget taskIcon, muteIcon, background, loadoutIcon;
		string m_sTaskText;
		SizeLayoutWidget m_wTaskLayout;
		ButtonWidget playerButton;
		SCR_BaseTaskExecutor taskExecutor;
		SCR_BasePlayerLoadout playerLoadout;
		Resource res;
		IEntityComponentSource source;
		BaseContainer container;
		SCR_EditableEntityUIInfo info;
		set<int> frequencies = new set<int>();
		
		playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));
		playerFrequency = TextWidget.Cast(playerTile.FindAnyWidget("Frequency"));
		m_wTaskLayout = SizeLayoutWidget.Cast(playerTile.FindAnyWidget("TaskLayout"));
		Widget wBackground = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconBackground"));
		Widget m_wOutline = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconOutline"));
		Widget m_wSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol"));
		muteIcon = ImageWidget.Cast(playerTile.FindAnyWidget("MuteIcon"));
		background = ImageWidget.Cast(playerTile.FindAnyWidget("Background"));
		loadoutIcon = ImageWidget.Cast(playerTile.FindAnyWidget("LoadoutIcon"));
		ImageWidget m_wIconSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol")); 
		ImageWidget platformIcon = ImageWidget.Cast(playerTile.FindAnyWidget("PlatformIcon"));

		playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
		if (!playerButton)
			return;

		m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
		m_PlayerTileComponent.SetTilePlayerID(playerID);

		SetupOptionsCombo(playerTile);

		playerName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
		
		SCR_PlayerController playerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerCtrl)
			playerCtrl.SetPlatformImageTo(playerID, platformIcon);

		ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
		if (controlledEntity)
		{
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
		if(!taskExecutor)
			return;	
		
		if (m_BaseTaskManager)
		{
			PlayerController m_PlayerController = GetGame().GetPlayerController();
			if (!m_PlayerController) 
				return;
			
			// TODO: check for good const usage
			const ResourceName TaskIconImageset = "{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset";	
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager) 
				return;
			
			Faction playerFaction = factionManager.GetPlayerFaction(m_PlayerController.GetPlayerId());
			if (!playerFaction) 
				return;
			
			Faction localPlayerFaction = factionManager.GetLocalPlayerFaction();
			if (!localPlayerFaction) 
				return;
			
			SCR_EditorTask gmTask = SCR_EditorTask.Cast(taskExecutor.GetAssignedTask());
			SCR_BaseTask baseTask = taskExecutor.GetAssignedTask();
			SCR_CampaignBaseTask task = SCR_CampaignBaseTask.Cast(taskExecutor.GetAssignedTask());
			
			PlayerController playerController = GetGame().GetPlayerController();	
			if (!playerController)
				return;
			
			m_sPlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (!m_sPlyFactionAffilComp)
				return;
			
			SCR_Faction m_sfactionCol = SCR_Faction.Cast(m_sPlyFactionAffilComp.GetAffiliatedFaction());
			if (!m_sfactionCol)
				return;
			
			if(gmTask || baseTask || task && playerFaction == localPlayerFaction)
			{
				m_wTaskLayout.SetVisible(true);
				m_wTaskLayout.SetColor(localPlayerFaction.GetFactionColor());
				wGroupTaskName = RichTextWidget.Cast(playerTile.FindAnyWidget("TaskDescription"));
		
				if (task) // When task is assigned and when player has the same faction with entry faction for campaign.
				{	
					m_wIconSymbol.LoadImageFromSet(0,TaskIconImageset,task.GetIconName());
					m_sTaskText = task.GetTaskListTaskText();
					task.SetTitleWidgetText(wGroupTaskName, task.GetTaskListTaskTitle());

					if (task.IsPriority())
					{	
						wBackground.SetColor(m_sfactionCol.GetOutlineFactionColor());
						m_wOutline.SetColor(m_sfactionCol.GetFactionColor());
						m_wIconSymbol.SetColor(m_sfactionCol.GetFactionColor());
					}
				}

				if (gmTask) // When task is assigned and when player has the same faction with entry faction for Game Master.
				{
					m_wIconSymbol.LoadImageFromSet(0,TaskIconImageset,gmTask.GetIconName());
					m_sTaskText = gmTask.GetTaskListTaskText();
					wGroupTaskName.SetTextFormat(gmTask.GetTitle(), gmTask.GetLocationName());
				}
			}
		}

		//set the state of mute
		if (muteIcon && GetGame().GetPlayerController().GetPlayerMutedState(playerID) == PermissionState.DISALLOWED)
		{
			muteIcon.SetColor(m_PlayerNameSelfColor);
			muteIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "sound-off");
		}

		//look for loadout and set the appropriate icon
		if (loadoutIcon)
		{
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (loadoutManager)
				playerLoadout = loadoutManager.GetPlayerLoadout(playerID);			
			
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

			//set leader icon if given player is a leader
			if (group.IsPlayerLeader(playerID))
			{
				m_LeaderInfo.SetIconTo(loadoutIcon);
				loadoutIcon.SetColor(m_PlayerNameSelfColor);
			}

			//set badge color
			Widget badge = playerTile.FindAnyWidget("PlayerBadge");
			if (badge)
			{
				Color badgeColor = m_GroupFaction.GetFactionColor();
				if (group.IsPlayerLeader(playerID))
					badgeColor = (m_PlayerNameSelfColor);

				SetBadgeColor(badge, badgeColor);
			}
		}

		m_PlayerTileComponent.GetOnTileFocus().Insert(OnPlayerTileFocus);
		m_PlayerTileComponent.GetOnTileFocusLost().Insert(OnPlayerTileFocusLost);

		m_aPlayerComponentsList.Insert(m_PlayerTileComponent);
	}
	//------------------------------------------------------------------------------------------------
	void SetIsPriority(bool isPriority)
	{
		m_bIsPriority = isPriority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPriority()
	{
		return m_bIsPriority;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocus(int id)
	{
		if (m_OnPlayerTileFocus)
			m_OnPlayerTileFocus.Invoke(id);
		
		 DisableConfirmButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocusLost(int id)
	{
		if (m_OnPlayerTileFocusLost)
			m_OnPlayerTileFocusLost.Invoke(id);
		
		EnableConfirmButton();
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
		if (m_GroupComponent.GetSelectedGroupID() == -1)
			m_GroupComponent.SetSelectedGroupID(m_iGroupID);
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (deviceType == EInputDeviceType.GAMEPAD || deviceType == EInputDeviceType.KEYBOARD)
		{
			RefreshPlayers();
			m_GroupComponent.SetSelectedGroupID(m_iGroupID);
		}
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

	//------------------------------------------------------------------------------------------------
	void SetJoinGroupButton(SCR_InputButtonComponent joinGroupButton)
	{
		m_JoinGroupButton = joinGroupButton;
	}

	//------------------------------------------------------------------------------------------------
	void SetupOptionsCombo(Widget playerTile)
	{
		ButtonWidget playerOptionsButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerOptions"));
		if (!playerOptionsButton)
			return;

		SCR_ComboBoxComponent playerOptionsCombo = SCR_ComboBoxComponent.Cast(playerOptionsButton.FindHandler(SCR_ComboBoxComponent));
		if (!playerOptionsCombo)
			return;

		m_PlayerTileComponent.SetOptionsComboBoxComponent(playerOptionsCombo);

		int playerID = GetGame().GetPlayerController().GetPlayerId();
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(playerID);

		if (group && m_iGroupID != group.GetGroupID() && m_PlayerTileComponent.GetTilePlayerID() != -1)
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_INVITE);


		group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		if (group.IsPlayerLeader(playerID) && playerID != m_PlayerTileComponent.GetTilePlayerID() && m_PlayerTileComponent.GetTilePlayerID() != -1)
		{
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_KICK);
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_PROMOTE);
		}

		if (m_PlayerTileComponent.GetOptionsComboComponent().GetNumItems() == 0)
			m_PlayerTileComponent.SetOpacity(0.3);

		m_PlayerTileComponent.GetOptionsComboComponent().m_OnOpened.Insert(DisableConfirmButton);
		m_PlayerTileComponent.GetOptionsComboComponent().m_OnClosed.Insert(EnableConfirmButton);

		m_PlayerTileComponent.GetOptionsComboComponent().m_OnChanged.Insert(OnComboBoxConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxConfirm(SCR_ComboBoxComponent combo, int index)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();

		if (!playerGroupController)
			return;

		int playerID;
		for (int i = m_aPlayerComponentsList.Count()-1; i >= 0; i--)
		{
			if (m_aPlayerComponentsList[i].GetOptionsComboComponent() == combo)
				playerID = m_aPlayerComponentsList[i].GetTilePlayerID();
		}

		switch (combo.GetItemName(index))
		{
			case OPTIONS_COMBO_INVITE:
			{
				playerGroupController.InvitePlayer(playerID);
				break;
			}
			case OPTIONS_COMBO_KICK:
			{
				playerGroupController.RequestKickPlayer(playerID);
				break;
			}
			case OPTIONS_COMBO_PROMOTE:
			{
				playerGroupController.RequestPromoteLeader(playerID);
				break;
			}
		}

		combo.SetCurrentItem(-1, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckLeaderOptions()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		Widget privateChecker = m_ParentSubMenu.GetRootWidget().FindAnyWidget("PrivateCheckLayout");
		if (!privateChecker)
			return;

		Widget privateCheckerButton = m_ParentSubMenu.GetRootWidget().FindAnyWidget("PrivateChecker");
		if (!privateCheckerButton)
			return;

		SCR_ButtonCheckerComponent checkerComp = SCR_ButtonCheckerComponent.Cast(privateCheckerButton.FindHandler(SCR_ButtonCheckerComponent));
		if (!checkerComp)
			return;

		ButtonWidget nameChangeButton = ButtonWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("ChangeNameButton"));
		if (!nameChangeButton)
			return;

		if (!group.IsPlayerLeader(GetGame().GetPlayerController().GetPlayerId()) || !m_GroupManager.CanPlayersChangeAttributes())
		{
			privateChecker.SetOpacity(0);
			privateChecker.SetEnabled(false);
			nameChangeButton.SetOpacity(0);
			nameChangeButton.SetEnabled(false);
			return;
		}

		checkerComp.SetToggled(group.IsPrivate(), instant: true);
		if (group.IsPrivate())
			checkerComp.SetText(PRIVATE_GROUP);
		else
			checkerComp.SetText(PUBLIC_GROUP);

		privateChecker.SetOpacity(1);
		privateChecker.SetEnabled(true);
		nameChangeButton.SetOpacity(1);
		nameChangeButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnGroupTileClicked()
	{
		return s_OnGroupButtonClicked;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnPlayerTileFocus()
	{
		if (!m_OnPlayerTileFocus)
			m_OnPlayerTileFocus = new ScriptInvokerInt();
		
		return m_OnPlayerTileFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnPlayerTileFocusLost()
	{
		if (!m_OnPlayerTileFocusLost)
			m_OnPlayerTileFocusLost = new ScriptInvokerInt();
		
		return m_OnPlayerTileFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableConfirmButton()
	{
		SetConfirmButtonStatus(true);
	}

	//------------------------------------------------------------------------------------------------
	void DisableConfirmButton()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager && inputManager.GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
			SetConfirmButtonStatus(false);
	}

	//------------------------------------------------------------------------------------------------
	void SetConfirmButtonStatus(bool status)
	{
//		SCR_RespawnSubMenuBase subMenu = SCR_RespawnSubMenuBase.Cast(m_ParentSubMenu);
//		if (!subMenu)
//			return;

//		subMenu.SetConfirmButtonEnabled(status);
	}

	//------------------------------------------------------------------------------------------------
	void SetBadgeColor(Widget badge, Color color)
	{
		if (!badge)
			return;

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		badgeTop = ImageWidget.Cast(badge.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(badge.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(badge.FindAnyWidget("BadgeBottom"));

		if (!badgeMiddle || !badgeTop || !badgeBottom)
			return;

		badgeTop.SetColor(color);
		badgeMiddle.SetColor(color);
		badgeBottom.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void ShowAIsInGroup()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		Widget playerTile;
		TextWidget playerName;
		ButtonWidget playerButton;

		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;

		array<SCR_ChimeraCharacter> aiMembers = slaveGroup.GetAIMembers();
		SCR_CharacterIdentityComponent identityComponent;

		foreach (SCR_ChimeraCharacter AIcharacter : aiMembers)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));

			identityComponent = SCR_CharacterIdentityComponent .Cast(AIcharacter.FindComponent(SCR_CharacterIdentityComponent));
			if (!identityComponent)
				return;
			string name;
			array<string> nameParams = {};
			identityComponent.GetFormattedFullName(name, nameParams);
			playerName.SetTextFormat(name, nameParams[0], nameParams[1], nameParams[2]);

			playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
			if (!playerButton)
				return;

			m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
			if (!m_PlayerTileComponent)
				return;

			m_PlayerTileComponent.SetTilePlayerID(-1);

			m_PlayerTileComponent.SetCharacter(AIcharacter);

			//set badge color
			Widget badge = playerTile.FindAnyWidget("PlayerBadge");

			if (badge)
				SetBadgeColor(badge, m_GroupFaction.GetFactionColor());
			
			SetupOptionsCombo(playerTile);
		}
	}
}
