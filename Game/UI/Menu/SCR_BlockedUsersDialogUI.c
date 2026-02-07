class SCR_BlockedUsersDialogUI : SCR_ConfigurableDialogUi
{
	protected const ResourceName ENTRY_WIDGET_NAME = "{3B2D99949BCE0542}UI/layouts/Menus/Dialogs/BlockedUsersEntry.layout";
	protected const string ENTRY_SCROLL_LIST = "Users";
	
	protected const string UNLOCK_BUTTON = "unblock";
	protected const string VIEW_GAMECARD_BUTTON = "viewGamecard";
	
	protected const string PLATFORM_
	
	protected int m_iMaxBlockedUserAmount = 10;
	
	protected Widget m_wUserListWidget;
	Widget m_wCurrentSelectedEntry;
	
	SCR_InputButtonComponent m_UnblockButton;
	protected SCR_InputButtonComponent m_GamecardButton;
	
	protected SocialComponent m_SocialComponent;
	
	protected ref SCR_BlocklistUnblockCallback m_BlocklistCallback;
	
	// Save the widget with the userID it belongs to, to get the correct ID from the focused widget
	ref map<Widget, BlockListItem> m_mUserList = new map<Widget, BlockListItem>();
	protected ref array<Widget> m_aUserList = {};
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		m_wUserListWidget = m_wRoot.FindAnyWidget(ENTRY_SCROLL_LIST);
		if (!m_wUserListWidget)
			return;
		
		ListBlockedUsers();
		
		m_UnblockButton = FindButton(UNLOCK_BUTTON);
		if (m_UnblockButton)
		{
			m_UnblockButton.m_OnActivated.Insert(OnUserUnblock);
			m_UnblockButton.SetEnabled(false);
		}
		
		// Show "View Gamecard" only for ps users
		m_GamecardButton = FindButton(VIEW_GAMECARD_BUTTON);
		if (m_GamecardButton)
			m_GamecardButton.SetVisible(false);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputTypeChanged);
		
	}
		
	//----------------------------------------------------------------------------------------------
	void OnInputTypeChanged(EInputDeviceType old, EInputDeviceType newDevice)
	{
		switch(newDevice){
			case EInputDeviceType.GAMEPAD:
				SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent)).SetButtonsVisibility(false);
				break;
			
			case EInputDeviceType.MOUSE:
				SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent)).SetButtonsVisibility(true);
				break;
			
			case EInputDeviceType.KEYBOARD:
				SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent)).SetButtonsVisibility(false);
				break;
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ListBlockedUsers()
	{
		SocialComponent.s_OnBlockListUpdateInvoker.Insert(OnBlockedListUpdated);
		SocialComponent.UpdateBlockList();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBlockedListUpdated(bool success)
	{
		if (!success)
			return;
		
		SCR_WidgetHelper.RemoveAllChildren(m_wUserListWidget);
		
		array<BlockListItem> outItems = {};
		SocialComponent.GetBlockedPlayers(outItems);
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget entry;
		RichTextWidget entryText;
		SCR_ModularButtonComponent button;
		SCR_BlockedUsersDialogEntryUIComponent entryComp;
		
		m_mUserList.Clear();
		
		foreach(BlockListItem item : outItems)
		{
			if (!item)
				continue;
			
			entry = workspace.CreateWidgets(ENTRY_WIDGET_NAME, m_wUserListWidget);
			if (!entry)
				continue;
			
			entryComp = SCR_BlockedUsersDialogEntryUIComponent.Cast(entry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
			if (!entryComp)
				continue;
			
			entryComp.SetPlatfrom(item.GetPlatform());
			entryComp.SetPlayerName(item.GetName());
			entryComp.GetProfileButton().m_OnClicked.Insert(OnViewGamecard);
			entryComp.GetUnblockButton().m_OnClicked.Insert(OnUserUnblock);

			// Save the widget & player ID
			m_mUserList.Insert(entry, item);
			
			button = SCR_ModularButtonComponent.FindComponent(entry);
			if (button){
				button.m_OnFocus.Insert(OnEntryFocused);
				button.m_OnFocusLost.Insert(OnEntryFocusLost);
			}
			
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocused(SCR_ModularButtonComponent modularButton)
	{
		m_wCurrentSelectedEntry = modularButton.GetRootWidget();
		
		if (m_UnblockButton)
			m_UnblockButton.SetEnabled(true);
		
		SCR_BlockedUsersDialogEntryUIComponent comp = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
		if (!comp)
			return;
		
		if(GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD){
			comp.SetButtonsVisibility(true);
		}else{
			comp.SetButtonsVisibility(false);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocusLost(SCR_ModularButtonComponent modularButton)
	{
		m_wCurrentSelectedEntry = modularButton.GetRootWidget();
		if(!m_wCurrentSelectedEntry)
			return;
		
		SCR_BlockedUsersDialogEntryUIComponent comp = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
		if (!comp)
			return;
		
		comp.SetButtonsVisibility(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUserUnblock()
	{	
		BlockListItem userItem;
		
		m_mUserList.Find(m_wCurrentSelectedEntry, userItem);
		
		if (!userItem)
			return;
		
		m_BlocklistCallback = new SCR_BlocklistUnblockCallback(this);
		userItem.DeleteBlock(m_BlocklistCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnViewGamecard()
	{
		// TODO:@brucknerjul Add functionality to view PSN profile when Button is clicked. API needed.		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		SCR_WidgetHelper.RemoveAllChildren(m_wUserListWidget);
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputTypeChanged);
	}
}

class SCR_BlocklistUnblockCallback : BackendCallback
{
	SCR_BlockedUsersDialogUI m_wDialogUI;
	
	//------------------------------------------------------------------------------------------------
	void SCR_BlocklistUnblockCallback(SCR_BlockedUsersDialogUI dialogUI)
	{
		m_wDialogUI = dialogUI;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		m_wDialogUI.m_wCurrentSelectedEntry.RemoveFromHierarchy();
		m_wDialogUI.m_mUserList.Remove(m_wDialogUI.m_wCurrentSelectedEntry);
		m_wDialogUI.m_wCurrentSelectedEntry = null;
		m_wDialogUI.m_UnblockButton.SetEnabled(false);
		SocialComponent.UpdateBlockList();
	}

	//------------------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{
		SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_AccountWidgetComponent.BLOCKED_USER_DIALOG_CONFIG, "block_failed_general", dialog);
	}
}