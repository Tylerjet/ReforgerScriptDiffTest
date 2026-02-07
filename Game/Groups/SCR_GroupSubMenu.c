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
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		m_wGridWidget = m_ParentMenu.GetRootWidget().FindAnyWidget("GroupList");
		if (!m_wGridWidget || !m_AddGroupButton || !m_ButtonLayout)
			return;
		InitGroups(m_wGridWidget, m_AddGroupButton, m_JoinGroupButton, m_ButtonLayout);
	}
	
	//------------------------------------------------------------------------------------------------
	static void InitGroups(Widget grid, SCR_NavigationButtonComponent addGroupButton, SCR_NavigationButtonComponent joinGroupButton, ResourceName buttonWidgetLayout)
	{
		s_RespawnComponent = SCR_RespawnSystemComponent.GetInstance();
		
		if (!grid)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateGroups, 1, false, grid, addGroupButton, joinGroupButton, buttonWidgetLayout);
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
		if (s_PlayerGroupController)
			s_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void UpdateGroups(notnull Widget gridWidget, SCR_NavigationButtonComponent addGroupbutton, SCR_NavigationButtonComponent joinGroupButton, ResourceName buttonWidgetLayout)
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
		
		array<SCR_AIGroup> playableGroups = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!playableGroups)
			return;
		
		int groupCount = playableGroups.Count();
		
		for (int i = 0; i < groupCount; i++)
		{ 
			Widget groupTile = GetGame().GetWorkspace().CreateWidgets(buttonWidgetLayout, gridWidget);
			ButtonWidget buttonWidget = ButtonWidget.Cast( groupTile.FindAnyWidget("Button"));
			SCR_GroupTileButton buttonComponent = SCR_GroupTileButton.Cast(buttonWidget.FindHandler(SCR_GroupTileButton));
			if (buttonComponent)
			{
				buttonComponent.SetGroupID(playableGroups[i].GetGroupID());
				buttonComponent.SetGroupFaction(playerFaction);
				buttonComponent.InitiateGroupTile();
				buttonComponent.SetJoinGroupButton(joinGroupButton);
				
				if (s_PlayerGroupController.GetGroupID() == -1 && i == 0 && s_PlayerGroupController.GetSelectedGroupID() < 0)
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (s_PlayerGroupController.GetSelectedGroupID() < 0 && playableGroups[i].GetGroupID() == s_PlayerGroupController.GetGroupID() )
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (s_PlayerGroupController.GetSelectedGroupID() == playableGroups[i].GetGroupID())
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAddGroupButton()
	{
		m_AddGroupButton = CreateNavigationButton("MenuAddGroup", CREATE_GROUP, true);
		if (m_AddGroupButton)
		{
			m_AddGroupButton.GetRootWidget().SetZOrder(0);
			m_AddGroupButton.m_OnActivated.Insert(CreateNewGroup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateJoinGroupButton()
	{
		m_JoinGroupButton = CreateNavigationButton("MenuJoinGroup", JOIN_GROUP, true);
		if (m_JoinGroupButton)
		{
			m_JoinGroupButton.GetRootWidget().SetZOrder(0);
			m_JoinGroupButton.m_OnActivated.Insert(JoinSelectedGroup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAcceptInviteButton()
	{
		m_AcceptInviteButton = CreateNavigationButton("GroupAcceptInvite", ACCEPT_INVITE, true);
		if (m_AcceptInviteButton)
		{
			m_AcceptInviteButton.GetRootWidget().SetZOrder(0);
			m_AcceptInviteButton.m_OnActivated.Insert(AcceptInvite);
			s_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
			SetAcceptButtonStatus()
		}
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
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		array<SCR_AIGroup> factionGroups = m_GroupManager.GetPlayableGroupsByFaction(m_PlayerFaction);
		if (factionGroups)
		{
			for (int i = factionGroups.Count() - 1; i >= 0; i--)
			{
				if (!factionGroups[i].BelongedToGroup(playerID))
					continue;
				
				s_PlayerGroupController.RequestJoinGroup(factionGroups[i].GetGroupID());
				
				if (s_PlayerGroupController.GetGroupID() >= 0)
					return;
			}
		}
		
		SCR_AIGroup newPlayerGroup = m_GroupManager.GetFirstNotFullForFaction(m_PlayerFaction);
		if (!newPlayerGroup)
			newPlayerGroup = m_GroupManager.CreateNewPlayableGroup(m_PlayerFaction);
		
		// Even the creation could fail
		if (!newPlayerGroup)
			return;
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
};
