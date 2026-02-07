class SCR_GroupSubMenu : SCR_RespawnSubMenuBase
{
	[Attribute("6", params: "1 100 1")]
	protected int m_iMaxColumnNumber;
	
	[Attribute()]
	protected ResourceName m_ButtonLayout;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected Widget m_wGridWidget;
	
	protected const string CREATE_GROUP = "#AR_DeployMenu_AddNewGroup";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string ACCEPT_INVITE = "#AR-DeployMenu_AcceptInvite";
	
	protected static SCR_RespawnSystemComponent s_RespawnComponent;
	protected static SCR_PlayerControllerGroupComponent s_PlayerGroupController;
	protected SCR_NavigationButtonComponent m_AddGroupButton;
	protected SCR_NavigationButtonComponent m_JoinGroupButton;
	protected SCR_NavigationButtonComponent m_AcceptInviteButton;
	protected SCR_NavigationButtonComponent m_GroupSettingsButton;
	protected Faction m_PlayerFaction;
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_ChatPanel m_ChatPanelComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		s_RespawnComponent = SCR_RespawnSystemComponent.GetInstance();
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		CreateConfirmButton();
		CreateQuickDeployButton();
		m_sConfirmButtonText = m_sContinueButtonText;
		if (m_ConfirmButton)
			m_ConfirmButton.m_OnActivated.Insert(OnConfirm);
		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateAcceptInviteButton();
		CreateGroupSettingsButton();
		SetupNameChangeButton();
		SetupPrivateChecker();
		m_PlayerFaction = s_RespawnComponent.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
		
		Widget chatPanel = parentMenu.GetRootWidget().FindAnyWidget("ChatPanel");
		if (!chatPanel)
			return;
		 m_ChatPanelComponent = SCR_ChatPanel.Cast(chatPanel.FindHandler(SCR_ChatPanel));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool ConfirmSelection()
	{
		SCR_RespawnSuperMenu menu = SCR_RespawnSuperMenu.GetInstance();
		m_GroupManager.SetConfirmedByPlayer(true);
		if (menu)
			menu.UpdateTabs();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void HandleOnConfirm()
	{
		ConfirmSelection();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		UpdateGroupsMenu();
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Insert(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Insert(OnChatClosed);
			if (m_ChatPanelComponent.IsOpen())
				OnChatOpen();
			if (m_ChatPanelComponent.GetFadeOut() == false)
			{
				Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
				if (chatContent)
					chatContent.SetVisible(true);
				SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
				if (chatAnimator)
					chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
			}
		}
		m_GroupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupsMenu);
		m_GroupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Insert(UpdateGroupsMenu);
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		m_wGridWidget = m_ParentMenu.GetRootWidget().FindAnyWidget("GroupList");
		if (!m_wGridWidget || !m_AddGroupButton || !m_ButtonLayout || !m_GroupSettingsButton)
			return;
		InitGroups(m_wGridWidget, m_AddGroupButton, m_JoinGroupButton, m_GroupSettingsButton, m_ButtonLayout);
	}
	
	//------------------------------------------------------------------------------------------------
	static void InitGroups(Widget grid, SCR_NavigationButtonComponent addGroupButton, SCR_NavigationButtonComponent joinGroupButton, SCR_NavigationButtonComponent groupSettingsButton, ResourceName buttonWidgetLayout)
	{
		s_RespawnComponent = SCR_RespawnSystemComponent.GetInstance();
		
		if (!grid)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateGroups, 1, false, grid, addGroupButton, joinGroupButton, groupSettingsButton, buttonWidgetLayout);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu) 
	{ 
		super.OnMenuHide(parentMenu);
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupsMenu);
		}
		
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Remove(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Remove(OnChatClosed);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Remove(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Remove(UpdateGroupsMenu);
		if (s_PlayerGroupController)
			s_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void UpdateGroups(notnull Widget gridWidget, SCR_NavigationButtonComponent addGroupbutton, SCR_NavigationButtonComponent joinGroupButton, SCR_NavigationButtonComponent groupSettingsButton, ResourceName buttonWidgetLayout)
	{
		if (!s_RespawnComponent)
			return;
		Faction playerFaction = s_RespawnComponent.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager || !s_PlayerGroupController)
			return;
		
		//no need to check playerFaction for null, because groups are not enabled for players without faction
		addGroupbutton.SetEnabled(groupManager.CanCreateNewGroup(playerFaction));
		Widget children = gridWidget.GetChildren();
		while (children)
		{
			gridWidget.RemoveChild(children);
			children = gridWidget.GetChildren();
		}
		
		SetGroupSettingsButton(groupSettingsButton);
		
		array<SCR_AIGroup> playableGroups = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!playableGroups)
			return;
		
		int groupCount = playableGroups.Count();
		int selectedGroupID;
		
		for (int i = 0; i < groupCount; i++)
		{ 
			Widget groupTile = GetGame().GetWorkspace().CreateWidgets(buttonWidgetLayout, gridWidget);	
			if (!groupTile)
				continue;
					
			ButtonWidget buttonWidget = ButtonWidget.Cast(groupTile.FindAnyWidget("Button"));
			if (!buttonWidget)
				continue;
			
			SCR_GroupTileButton buttonComponent = SCR_GroupTileButton.Cast(buttonWidget.FindHandler(SCR_GroupTileButton));
			if (buttonComponent)
			{
				buttonComponent.SetGroupID(playableGroups[i].GetGroupID());
				buttonComponent.SetGroupFaction(playerFaction);
				buttonComponent.SetJoinGroupButton(joinGroupButton);
				buttonComponent.InitiateGroupTile();
				selectedGroupID = s_PlayerGroupController.GetSelectedGroupID();
				if (s_PlayerGroupController.GetGroupID() == -1 && i == 0 && s_PlayerGroupController.GetSelectedGroupID() < 0)
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID < 0 && playableGroups[i].GetGroupID() == s_PlayerGroupController.GetGroupID() )
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID == playableGroups[i].GetGroupID())
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAddGroupButton()
	{
		m_AddGroupButton = CreateNavigationButton("MenuAddGroup", CREATE_GROUP, true);
		if (!m_AddGroupButton)
			return;
		m_AddGroupButton.GetRootWidget().SetZOrder(0);
		m_AddGroupButton.m_OnActivated.Insert(CreateNewGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateJoinGroupButton()
	{
		m_JoinGroupButton = CreateNavigationButton("MenuJoinGroup", JOIN_GROUP, true);
		if (!m_JoinGroupButton)
			return;
		m_JoinGroupButton.GetRootWidget().SetZOrder(0);
		m_JoinGroupButton.m_OnActivated.Insert(JoinSelectedGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAcceptInviteButton()
	{
		m_AcceptInviteButton = CreateNavigationButton("GroupAcceptInvite", ACCEPT_INVITE, true);
		if (!m_AcceptInviteButton)
			return;
		m_AcceptInviteButton.GetRootWidget().SetZOrder(0);
		m_AcceptInviteButton.m_OnActivated.Insert(AcceptInvite);
		s_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
		SetAcceptButtonStatus()
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGroupSettingsButton()
	{
		m_GroupSettingsButton = CreateNavigationButton("MenuSettingsGroup", "#AR-Player_Groups_Settings", true);
		if (!m_GroupSettingsButton)
			return;
		m_GroupSettingsButton.GetRootWidget().SetZOrder(0);
		m_GroupSettingsButton.SetVisible(false);
		m_GroupSettingsButton.m_OnActivated.Insert(OpenGroupSettingsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangeGroupPublicState()
	{
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerComponent)
			return;
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		SCR_AIGroup playerGroup = groupsManager.FindGroup(playerComponent.GetGroupID());
		playerComponent.RequestPrivateGroupChange(playerComponent.GetPlayerID() , !playerGroup.IsPrivate());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenGroupSettingsDialog()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupSettingsDialog);
	}
		
	//------------------------------------------------------------------------------------------------
	protected static void CreateNewGroup()
	{
		if (!s_PlayerGroupController)
			return;
		//we reset the actual group so the menu goes to players actual group, in this case newly created one
		s_PlayerGroupController.SetSelectedGroupID(-1);
		s_PlayerGroupController.RequestCreateGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinSelectedGroup()
	{
		if (!s_PlayerGroupController)
			return;	
		s_PlayerGroupController.RequestJoinGroup(s_PlayerGroupController.GetSelectedGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	static void SetSelectedGroup(int groupID)
	{
		s_PlayerGroupController.SetSelectedGroupID(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		s_PlayerGroupController.AcceptInvite();
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{
		if (s_PlayerGroupController.GetGroupInviteID() == -1 )
			m_AcceptInviteButton.SetEnabled(false);
		else
			m_AcceptInviteButton.SetEnabled(true);	
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnConfirm()
	{
		//dont assign group if player already has one
		if (s_PlayerGroupController.GetGroupID() != -1)
			return;
		
		SCR_AIGroup newPlayerGroup = m_GroupManager.GetFirstNotFullForFaction(m_PlayerFaction, null, true);
		if (!newPlayerGroup)
		{
			//we return here, because creationg of the group automatically moves player to the group
			s_PlayerGroupController.RequestCreateGroup();
			return;
		}
		s_PlayerGroupController.RequestJoinGroup(newPlayerGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatOpen()
	{
		Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
		if (chatContent)
			chatContent.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatClosed()
	{
		if (SCR_ChatPanelManager.GetInstance().GetMessages().Count() == 0)
		{
			Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			return;
		}	
		SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
		if (!chatAnimator)
			return;
		chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatorStateChanged(int formerState, int actualState)
	{
		if (formerState == 2 && actualState == 3)
		{
			Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			m_ChatPanelComponent.GetFadeInOutAnimator().GetOnStateChanged().Remove(OnAnimatorStateChanged);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void SetGroupSettingsButton(notnull SCR_NavigationButtonComponent button)
	{
		button.SetVisible(s_PlayerGroupController.IsPlayerLeaderOwnGroup());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupNameChangeButton()
	{
		ButtonWidget nameChangeButton = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("ChangeNameButton"));
		if (!nameChangeButton)
			return;
		
		SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(nameChangeButton.FindHandler(SCR_ButtonImageComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnClicked.Insert(OpenGroupSettingsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupPrivateChecker()
	{
		ButtonWidget privateChecker = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("PrivateChecker"));
		if (!privateChecker)
			return;
		
		SCR_ButtonCheckerComponent buttonComp = SCR_ButtonCheckerComponent.Cast(privateChecker.FindHandler(SCR_ButtonCheckerComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnClicked.Insert(OnPrivateCheckerClicked);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPrivateCheckerClicked()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(s_PlayerGroupController.GetGroupID());
		if (!group)
			return;
		
		s_PlayerGroupController.RequestPrivateGroupChange(s_PlayerGroupController.GetPlayerID() , !group.IsPrivate());
	}
};
