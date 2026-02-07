class SCR_GroupSubMenuPlayerlist : SCR_SubMenuBase
{
	[Attribute("6", params: "1 100 1")]
	protected int m_iMaxColumnNumber;
	
	[Attribute()]
	protected ResourceName m_ButtonLayout;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected Widget m_wGridWidget;
	protected SCR_NavigationButtonComponent m_AddGroupButton;
	protected SCR_NavigationButtonComponent m_JoinGroupButton;
	protected SCR_NavigationButtonComponent m_AcceptInviteButton;
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	
	protected const string CREATE_GROUP = "#AR_DeployMenu_AddNewGroup";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string ACCEPT_INVITE = "#AR-DeployMenu_AcceptInvite";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerGroupController)
			return;
		if (!m_ParentMenu)
			return;
		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateAcceptInviteButton();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
	
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(false);
		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(false);
		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(false);
		
		UpdateGroupsMenu();
		
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupsMenu);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController || !m_ParentMenu)
			return;
		m_wGridWidget = m_ParentMenu.GetRootWidget().FindAnyWidget("GroupList");
		SCR_GroupSubMenu.InitGroups(m_wGridWidget, m_AddGroupButton, m_JoinGroupButton, m_ButtonLayout);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupsMenu);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupsMenu);
		m_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
		
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(true);
		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(true);
		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateNewGroup()
	{
		if (!m_PlayerGroupController)
			return;
		//we reset the selected group so the menu goes to players actual group, in this case newly created one
		m_PlayerGroupController.SetSelectedGroupID(-1);
		m_PlayerGroupController.RequestCreateGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinSelectedGroup()
	{
		if (!m_PlayerGroupController)
			return;	
		m_PlayerGroupController.RequestJoinGroup(m_PlayerGroupController.GetSelectedGroupID());
	}
	
		//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		m_PlayerGroupController.AcceptInvite();
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{
		if (!m_AcceptInviteButton)
			return;
		if (m_PlayerGroupController.GetGroupInviteID() == -1 )
			m_AcceptInviteButton.SetEnabled(false);
		else
			m_AcceptInviteButton.SetEnabled(true);	
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
			m_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
			SetAcceptButtonStatus();
		}
	}

};
