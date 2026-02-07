enum EPlayerListTab
{
	ALL = 0,
	GROUPS = 1
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerListEntry
{
	SCR_ScoreInfo m_Info;
	int m_iID;
	Widget m_wRow;
	Faction m_Faction;
	int m_iSortFrequency;
	
	SCR_ButtonBaseComponent m_Mute;
	SCR_ButtonBaseComponent m_Block;
	SCR_ButtonBaseComponent m_Friend;
	SCR_ComboBoxComponent m_VotingCombo;
	
	TextWidget m_wName;
	TextWidget m_wKills;
	TextWidget m_wFreq;
	TextWidget m_wDeaths;
	TextWidget m_wScore;
	ImageWidget m_wLoadoutIcon;
	ImageWidget m_wPlatformIcon;
	Widget m_wTaskIcon;
	Widget m_wFactionImage;
	Widget m_wVotingNotification;
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerListMenu : SCR_SuperMenuBase
{
	protected ResourceName m_sScoreboardRow = "{65369923121A38E7}UI/layouts/Menus/PlayerList/PlayerListEntry.layout";
	
	protected ref array<ref SCR_PlayerListEntry> m_aEntries = new array<ref SCR_PlayerListEntry>();
	protected ref map<int, SCR_ScoreInfo> m_aAllPlayersInfo = new ref map<int, SCR_ScoreInfo>();
	protected ref array<Faction> m_aFactions = {null};
	
	protected SCR_NavigationButtonComponent m_Mute;
	protected SCR_NavigationButtonComponent m_Block;
	protected SCR_NavigationButtonComponent m_Friend;
	protected SCR_NavigationButtonComponent m_Vote;
	protected SCR_NavigationButtonComponent m_Invite;
	
	protected SCR_VotingManagerComponent m_VotingManager;
	protected SCR_VoterComponent m_VoterComponent;
	protected SCR_RespawnSystemComponent m_RespawnSystem;
	protected SCR_BaseScoringSystemComponent m_ScoringSystem;
	protected SCR_PlayerListEntry m_SelectedEntry;
	protected SCR_PlayerControllerGroupComponent m_playerGroupController;
	SCR_SortHeaderComponent m_Header;
	protected Widget m_wTable;
	protected Widget m_wRoot;
	protected bool m_bFiltering;
	protected float m_fTimeSkip;
	
	protected const float TIME_STEP = 1.0;
	
	protected const string ADD_FRIEND = "#AR-PlayerList_AddFriend";
	protected const string REMOVE_FRIEND = "#AR-PlayerList_RemoveFriend";
	protected const string MUTE = "#AR-PlayerList_Mute";
	protected const string UNMUTE = "#AR-PlayerList_Unmute";
	protected const string BLOCK = "#AR-PlayerList_Block";
	protected const string UNBLOCK = "#AR-PlayerList_Unblock";
	protected const string INVITE_PLAYER_VOTE = "#AR-PlayerList_Invite";
	protected const string MUTE_TEXTURE = "sound-off";
	
	protected const string FILTER_FAV = "Favourite";
	protected const string FILTER_NAME = "Name";
	protected const string FILTER_FREQ = "Freq";
	protected const string FILTER_KILL = "Kills";
	protected const string FILTER_DEATH = "Deaths";
	protected const string FILTER_SCORE = "Score";
	protected const string FILTER_MUTE = "Mute";
	protected const string FILTER_BLOCK = "Block";
	
	protected const int DEFAULT_SORT_INDEX = 1;
	
	protected string m_sGameMasterIndicatorName = "GameMasterIndicator";
	protected ref array<EVotingType> m_aVotingTypes = {};
	
	protected static ref ScriptInvoker s_OnPlayerListMenu = new ScriptInvoker();
	
	protected ref Color m_PlayerNameSelfColor = new Color(0.898, 0.541, 0.184, 1);
	
	/*!
	Get event called when player list opens or closes.
	\return Script invoker
	*/
	static ScriptInvoker GetOnPlayerListMenu()
	{
		return s_OnPlayerListMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitSorting()
	{
		if (!m_wRoot)
			return;
		
		Widget w = m_wRoot.FindAnyWidget("SortHeader");
		if (!w)
			return;
		
		m_Header = SCR_SortHeaderComponent.Cast(w.FindHandler(SCR_SortHeaderComponent));
		if (!m_Header)
			return;
		
		m_Header.m_OnChanged.Insert(OnHeaderChanged);
			
		if (m_ScoringSystem)
			return;

		// Hide K/D/S sorting headers if the re is no scoreboard		
		ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
		ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
		ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));
		
		if (sortKills)
			sortKills.SetOpacity(0);
		if (sortDeaths)
			sortDeaths.SetOpacity(0);
		if (sortScore)
			sortScore.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHeaderChanged(SCR_SortHeaderComponent sortHeader)
	{
		string filterName = sortHeader.GetSortElementName();
		bool sortUp = sortHeader.GetSortOrderAscending();
		Sort(filterName, sortUp);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void Sort(string filterName, bool sortUp)
	{
		if (filterName == FILTER_NAME)
			SortByName(sortUp);
		else if (filterName == FILTER_FAV)
			SortByFriends(sortUp);
		else if (filterName == FILTER_FREQ)
			SortByFrequency(sortUp);
		else if (filterName == FILTER_KILL)
			SortByKills(sortUp);
		else if (filterName == FILTER_DEATH)
			SortByDeaths(sortUp);
		else if (filterName == FILTER_SCORE)
			SortByScore(sortUp);
		else if (filterName == FILTER_MUTE)
			SortByMuted(sortUp);
		else if (filterName == FILTER_BLOCK)
			SortByBlocked(sortUp);
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByMuted(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_Mute.IsToggled() * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByBlocked(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_Block.IsToggled() * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByName(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		array<string> names = {};
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			if (entry.m_wName)
				names.Insert(entry.m_wName.GetText());
		}
		
		names.Sort();
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			if (!entry.m_wName)
				continue;
			
			string text = entry.m_wName.GetText();
			
			foreach (int i, string s : names)
			{
				if (s != text)
					continue;
				
				if (entry.m_wRow)
					entry.m_wRow.SetZOrder(i * direction);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByFriends(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_Friend.IsToggled() * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByFrequency(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_iSortFrequency * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByKills(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iKills * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByDeaths(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iDeaths * direction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SortByScore(bool reverseSort = false)
	{
 		int direction = 1;
		if (reverseSort)
			direction = -1;
		
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			SCR_ScoreInfo info = entry.m_Info;
			if (info)
			{
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);
				else
					score = 0;
				
				entry.m_wRow.SetZOrder(score * direction);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBlock()
	{
		if (!m_SelectedEntry)
			return;
		
		SCR_ButtonBaseComponent block = m_SelectedEntry.m_Block;
		if (!block)
			return;
		
		block.SetToggled(!block.IsToggled());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMute()
	{
		if (!m_SelectedEntry)
			return;
		
		SCR_ButtonBaseComponent mute = m_SelectedEntry.m_Mute; // This is temporary, until mute and block are not the same
		if (!mute)
			return;
		mute.SetToggled(!mute.IsToggled());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAddFriend()
	{
  		if (!m_SelectedEntry)
			return;
		
		SCR_ButtonBaseComponent friend = m_SelectedEntry.m_Friend;
		if (!friend)
			return;
		
		friend.SetToggled(!friend.IsToggled());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabChanged(SCR_TabViewComponent comp, Widget w, int selectedTab)
	{
		if (selectedTab < 0)
			return;
		
		Faction faction = null;
		foreach (Faction playableFaction : m_aFactions)
		{
			if (comp.GetShownTabComponent().m_sTabButtonContent == playableFaction.GetFactionName())
				faction = playableFaction;
		}
		
		int lowestZOrder = int.MAX;
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			if (!entry.m_wRow)
				continue;
	
			//if the tab is the first one, it's the All tab for now
			if (comp.GetShownTab() == 0)
				entry.m_wRow.SetVisible(true);
			else if (faction == entry.m_Faction)
				entry.m_wRow.SetVisible(true);
			else
				entry.m_wRow.SetVisible(false);
		}
		
		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
		CloseAllVoting();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		if (m_bFiltering)
			OnFilter();
		else
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFilter()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVoting()
	{
		if (!m_SelectedEntry)
			return;
		
		SCR_ComboBoxComponent comp = m_SelectedEntry.m_VotingCombo;
		if (!comp)
			return;
		
		if (comp.IsOpened())
			comp.CloseList();
		else
			comp.OpenList();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInvite()
	{
		if (!m_SelectedEntry)
			return;
		SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent().InvitePlayer(m_SelectedEntry.m_iID);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMuteClick(SCR_ButtonBaseComponent comp, bool selected)
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;
		
		int id = -1;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Mute != comp)
				continue;
			
			id = entry.m_iID;
			break;
		}
		
		controller.SetPlayerMutedState(id, selected);
		
		if (GetGame().GetPlayerController().GetPlayerMutedState(id) == PermissionState.DISALLOWED)
			m_Mute.SetLabel(UNMUTE);
		else
			m_Mute.SetLabel(MUTE);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBlockClick(SCR_ButtonBaseComponent comp, bool selected)
	{
		PlayerController controller = GetGame().GetPlayerController();
		SCR_ButtonBaseComponent mute;
		if (!controller)
			return;
		
		int id = -1;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Block != comp)
				continue;
			mute = entry.m_Mute;
			id = entry.m_iID;
			break;
		}
		
		controller.SetPlayerBlockedState(id, selected);
		
		// Set button state, set nav button state
		if (GetGame().GetPlayerController().GetPlayerBlockedState(id) == PermissionState.DISALLOWED)
			m_Block.SetLabel(UNBLOCK);
		else
			m_Block.SetLabel(BLOCK);
		
		if (mute)
		{
			mute.SetToggled(true);
			mute.SetEnabled(!mute.IsEnabled());
			m_Mute.SetEnabled(!m_Mute.IsEnabled());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFriendClick(SCR_ButtonBaseComponent comp, bool selected)
	{
		// TODO: waiting for API
		if (selected)
			m_Friend.SetLabel(REMOVE_FRIEND);
		else
			m_Friend.SetLabel(ADD_FRIEND);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEntryFocused(Widget w)
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;
			
			Widget row = entry.m_wRow;
			if (row != w)
				continue;
			
			m_SelectedEntry = entry;
			break;
		}

		bool enableFriend;
		bool enableBlock;
		bool enableMute;
		bool enableVoting = CanOpenVoteList(m_SelectedEntry);
		
		if (m_SelectedEntry)
		{
			if (m_SelectedEntry.m_Friend)
				enableFriend = m_SelectedEntry.m_Friend.IsEnabled();
			if (m_SelectedEntry.m_Mute)
				enableMute = m_SelectedEntry.m_Mute.IsEnabled();
			if (m_SelectedEntry.m_Block)
			{
				enableBlock = m_SelectedEntry.m_Block.IsEnabled();
				if (m_SelectedEntry.m_Block.IsToggled())
					m_Friend.SetLabel(REMOVE_FRIEND);
				else
					m_Friend.SetLabel(ADD_FRIEND);
			}
			if (m_SelectedEntry.m_VotingCombo)
				m_SelectedEntry.m_VotingCombo.SetEnabled(enableVoting);
		}
		
		if (m_Friend)
			m_Friend.SetEnabled(enableFriend);
		if (m_Block)
		{
			m_Block.SetEnabled(enableBlock);
			if (GetGame().GetPlayerController().GetPlayerBlockedState(m_SelectedEntry.m_iID) == PermissionState.DISALLOWED)
			{
				enableMute = false;
				m_Block.SetLabel(UNBLOCK);
			}
			else
				m_Block.SetLabel(BLOCK);
		}
		if (m_Mute)
		{
			m_Mute.SetEnabled(enableMute);
			if (GetGame().GetPlayerController().GetPlayerMutedState(m_SelectedEntry.m_iID) == PermissionState.DISALLOWED)
				m_Mute.SetLabel(UNMUTE);
			else
				m_Mute.SetLabel(MUTE);
		}
		if (m_Vote)
			m_Vote.SetEnabled(enableVoting);
		if (m_Invite && m_playerGroupController)
			m_Invite.SetEnabled(m_playerGroupController.CanInvitePlayer(m_SelectedEntry.m_iID));
			
	}
	
	//------------------------------------------------------------------------------------------------
	void FocusFirstItem()
	{
		Widget firstEntry;
		int lowestZOrder = int.MAX;
		foreach (SCR_PlayerListEntry entry:  m_aEntries)
		{
			if (!entry.m_wRow.IsVisible())
				continue;
			
			int z = entry.m_wRow.GetZOrder();
			if (z < lowestZOrder)
			{
				lowestZOrder = z;
				firstEntry = entry.m_wRow;
			}
		}
		
		if (firstEntry)
			GetGame().GetWorkspace().SetFocusedWidget(firstEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupVoteComboBox(SCR_ComboBoxComponent combo)
	{
		if (!m_VotingManager || !m_VoterComponent)
			return;
		
		combo.ClearAll();
		
		int playerID = GetVotingPlayerID(combo);
		SCR_VotingUIInfo info;
		for (int i, count = m_VotingManager.GetVotingsAboutPlayer(playerID, m_aVotingTypes, true, true); i < count; i++)
		{
			EVotingType votingType = m_aVotingTypes[i];
			info = m_VotingManager.GetVotingInfo(votingType);
			
			if (!m_VotingManager.IsVoting(votingType, playerID))
			{
				//--- Voting not in progress, start it
				combo.AddItem(info.GetStartVotingName());
			}
			else
			{
				//--- Voting in progress
				if (m_VoterComponent.DidVote(votingType, playerID))
				{
					//--- Did cast a vote, withdraw it
					combo.AddItem(info.GetCancelVotingName());
				}
				else
				{
					//--- Did not cast a vote, do it now
					combo.AddItem(info.GetName());
				}
			}
		}
		if (m_playerGroupController.CanInvitePlayer(playerID))
			combo.AddItem(INVITE_PLAYER_VOTE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxConfirm(SCR_ComboBoxComponent combo, int index)
	{
		if (!m_VoterComponent)
			return;
		
		if (combo.GetItemName(index) == INVITE_PLAYER_VOTE)
		{
			SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
			if (groupComponent)
				groupComponent.InvitePlayer(GetVotingPlayerID(combo));
		}
		else 
		{
			EVotingType votingType = m_aVotingTypes[index];
			int playerID = GetVotingPlayerID(combo);
			
			if (m_VoterComponent.DidVote(votingType, playerID))
				m_VoterComponent.RemoveVote(votingType, playerID);
			else
				m_VoterComponent.Vote(votingType, playerID);
		}
		combo.SetCurrentItem(-1, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetVotingPlayerID(SCR_ComboBoxComponent combo)
	{
		for (int i, count = m_aEntries.Count(); i < count; i++)
		{
			if (m_aEntries[i].m_VotingCombo == combo)
				return m_aEntries[i].m_iID;
		}
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateEntry(int id, SCR_PlayerDelegateEditorComponent editorDelegateManager)
	{
		//check for existing entry, return if it exists already
		foreach (SCR_PlayerListEntry entry : m_aEntries )
		{
			if (entry.m_iID == id)
				return;
		}
		
		ImageWidget badgeTop, badgeMiddle, badgeBottom;
		
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		if (!w)
			return;
		
		SCR_PlayerListEntry entry = new SCR_PlayerListEntry();
		entry.m_iID = id;
		entry.m_wRow = w;
		
		//--- Initialize voting combo box
		entry.m_VotingCombo = SCR_ComboBoxComponent.GetComboBoxComponent("VotingCombo", w);
		if (m_VotingManager)
		{
			entry.m_VotingCombo.m_OnOpened.Insert(SetupVoteComboBox);
			entry.m_VotingCombo.m_OnChanged.Insert(OnComboBoxConfirm);
			entry.m_VotingCombo.SetEnabled(CanOpenVoteList(entry));
				
			entry.m_wVotingNotification = entry.m_wRow.FindAnyWidget("VotingNotification");
			entry.m_wVotingNotification.SetVisible(IsVotedAbout(entry));
		}
		else
		{
			entry.m_VotingCombo.SetVisible(false);
		}
		
		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
			handler.m_OnFocus.Insert(OnEntryFocused);
		
		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId  != id)
					continue;
				
				entry.m_Info = info;
				break;
			}
		}
		
		// Find faction
		if (m_RespawnSystem)
		{
			Faction faction = m_RespawnSystem.GetPlayerFaction(entry.m_iID);
			entry.m_Faction = faction;
		}
		
		Widget factionImage = w.FindAnyWidget("FactionImage");
		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}
		entry.m_wName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		if (entry.m_wName)
		{
			entry.m_wName.SetText(GetGame().GetPlayerManager().GetPlayerName(id));
			if (entry.m_iID == GetGame().GetPlayerController().GetPlayerId())
				entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}

		if (editorDelegateManager)
		{
			SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(id);
			
			if (playerEditorDelegate)
			{
				playerEditorDelegate.GetOnLimitedEditorChanged().Insert(OnEditorRightsChanged);
				UpdateGameMasterIndicator(entry, playerEditorDelegate.HasLimitedEditor());
			}
		}
		
		entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));		
		entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore) 
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);
				
				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{
			
			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			/*
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			*/
		}
		
		entry.m_Mute = SCR_ButtonBaseComponent.GetButtonBase("Mute", w);
		entry.m_Friend = SCR_ButtonBaseComponent.GetButtonBase("Friend", w);
		entry.m_Block = SCR_ButtonBaseComponent.GetButtonBase("Block", w);
		entry.m_wTaskIcon = entry.m_wRow.FindAnyWidget("TaskIcon");
		entry.m_wLoadoutIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("LoadoutIcon"));
		entry.m_wPlatformIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("PlatformIcon"));
		if (entry.m_wTaskIcon && GetTaskManager())
		{
			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.FindTaskExecutorByID(entry.m_iID);
			if (taskExecutor.GetAssignedTask())
			{
				entry.m_wTaskIcon.SetColor(entry.m_Faction.GetFactionColor());
			}
			else 
			{
				entry.m_wTaskIcon.SetOpacity(0);
			}
		}
		
		
		Faction playerFaction = m_RespawnSystem.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
		SCR_BasePlayerLoadout playerLoadout = m_RespawnSystem.GetPlayerLoadout(entry.m_iID);
		
		if (entry.m_wLoadoutIcon && (playerFaction != m_RespawnSystem.GetPlayerFaction(entry.m_iID)))
			entry.m_wLoadoutIcon.SetVisible(false);
		
		//enable GM to see everyones loadout icon, temporary solution until we get improved ways to say who's GM
		if (SCR_EditorManagerEntity.IsOpenedInstance())
			entry.m_wLoadoutIcon.SetVisible(true);
		
		if (entry.m_wLoadoutIcon && playerLoadout && entry.m_wLoadoutIcon.IsVisible())
		{
			Resource res = Resource.Load(playerLoadout.GetLoadoutResource());
			IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
			if (!source)
				return;
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			info.SetIconTo(entry.m_wLoadoutIcon);
		}
		
		if (entry.m_wPlatformIcon)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			PlatformKind playerPlatform = playerManager.GetPlatformKind(entry.m_iID);
			string iconName = SCR_Global.GetPlatformName(playerPlatform);
			entry.m_wPlatformIcon.LoadImageFromSet(0, "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset", iconName);
			entry.m_wPlatformIcon.SetImage(0);
		}
		else
		{
			Print("No platform icon detected in players list. This is against MS Xbox Live rules.", LogLevel.WARNING);
		}
		
		
		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;
		
		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}
		
		// If this is player, set all buttons invisible
		if (IsLocalPlayer(entry.m_iID))
		{
			if (entry.m_Friend)
				entry.m_Friend.SetEnabled(false);
			
			if (entry.m_Mute)
				entry.m_Mute.SetEnabled(false);
			
			if (entry.m_Block)
				entry.m_Block.SetEnabled(false);
		}
		else
		{
			PlayerController controller = GetGame().GetPlayerController();
			if (entry.m_Mute)
			{
				// Find out if the person is aleady muted
				//entry.m_Mute.SetEnabled(false);
				
				if (controller)
				{
					entry.m_Mute.SetEnabled(true);
					if (controller.GetPlayerMutedState(entry.m_iID) == PermissionState.DISALLOWED)
						entry.m_Mute.SetToggled(true);

					else if (controller.GetPlayerMutedState(entry.m_iID) == PermissionState.ALLOWED)
					
						entry.m_Mute.SetToggled(false);						
						
					if (controller.GetPlayerMutedState(entry.m_iID) ==  PermissionState.DISABLED)
							entry.m_Mute.SetEnabled(false);
						
				}
				entry.m_Mute.m_OnToggled.Insert(OnMuteClick);
			}
			
			if (entry.m_Friend)
			{
				entry.m_Friend.SetEnabled(false);
				entry.m_Friend.m_OnToggled.Insert(OnFriendClick);
			}
			
			if (entry.m_Block)
			{
				if (controller)
				{
					entry.m_Block.SetEnabled(true);
					if (controller.GetPlayerBlockedState(entry.m_iID) == PermissionState.DISALLOWED)
						entry.m_Block.SetToggled(true);

					else if (controller.GetPlayerBlockedState(entry.m_iID) == PermissionState.ALLOWED)
					
						entry.m_Block.SetToggled(false);						
						
					if (controller.GetPlayerBlockedState(entry.m_iID) ==  PermissionState.DISABLED)
							entry.m_Block.SetEnabled(false);
						
				}
	
				entry.m_Block.m_OnToggled.Insert(OnBlockClick);
			}
		}

		m_aEntries.Insert(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorRightsChanged(int playerID, bool newLimited)
	{
		foreach (SCR_PlayerListEntry entry: m_aEntries)
		{
			if (entry.m_iID == playerID)
			{
				UpdateGameMasterIndicator(entry, newLimited);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateGameMasterIndicator(SCR_PlayerListEntry entry, bool editorIslimited)
	{
		Widget gameMasterIndicator = entry.m_wRow.FindAnyWidget(m_sGameMasterIndicatorName);
		if (gameMasterIndicator)
			gameMasterIndicator.SetVisible(!editorIslimited);
	}
	
	//------------------------------------------------------------------------------------------------
	// IsLocalPlayer would be better naming
	protected bool IsLocalPlayer(int id)
	{
		if (id <= 0)
			return false;

		return SCR_PlayerController.GetLocalPlayerId() == id;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_PlayerListEntry entry)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();
		
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		
		//Remove the subscription to player right changed
		if (editorDelegateManager)
		{
			SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(entry.m_iID);
			
			if (playerEditorDelegate)
			{
				playerEditorDelegate.GetOnLimitedEditorChanged().Remove(OnEditorRightsChanged);
			}
		}
		
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenPauseMenu()
	{
		GetGame().OpenPauseMenu();
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingChanged(EVotingType type, int value, int playerID)
	{
		UpdateVoting();
	}
	protected void GetOnVotingStart(EVotingType type, int value)
	{
		UpdateVoting();
	}
	protected void UpdateVoting()
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			entry.m_VotingCombo.SetEnabled(CanOpenVoteList(entry));
			entry.m_wVotingNotification.SetVisible(IsVotedAbout(entry));
		}
		
		if (m_SelectedEntry)
			m_Vote.SetEnabled(CanOpenVoteList(m_SelectedEntry));
	}
	protected bool IsVotedAbout(SCR_PlayerListEntry entry)
	{
		if (!entry || !m_VotingManager)
			return false;
		
		array<EVotingType> votingTypes = {};
		int count = m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes);
		return count > 0;
	}
	protected bool CanOpenVoteList(SCR_PlayerListEntry entry)
	{
		if (!entry || !m_VotingManager)
			return false;
		
		array<EVotingType> votingTypes = {};
		int count = m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes, true, true);
		return count > 0;
	}
	//------------------------------------------------------------------------------------------------
	
	
	private void OnPlayerAdded(int playerId)
	{
		UpdatePlayerList(true, playerId);
	}	
	private void OnPlayerRemoved(int playerId)
	{
		UpdatePlayerList(false, playerId);
	}
	private void OnScoreChanged()
	{
		UpdateScore();
	}	
	private void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}
	private void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen() 
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		
		//if (!m_ChatPanel)
		//	m_ChatPanel = SCR_ChatPanel.Cast(m_wRoot.FindAnyWidget("ChatPanel").FindHandler(SCR_ChatPanel));
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_HUDManagerComponent));
		hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~EHudLayers.HIGH);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		m_playerGroupController =  SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_VoterComponent = SCR_VoterComponent.GetInstance();
		m_VotingManager = SCR_VotingManagerComponent.GetInstance();
		
		if (m_VotingManager)
		{
			m_VotingManager.GetOnVotingEnd().Insert(OnVotingChanged);
			m_VotingManager.GetOnVotingStart().Insert(GetOnVotingStart);
			m_VotingManager.GetOnRemoveVote().Insert(OnVotingChanged);
		}
		
		//gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerRegistered().Insert(OnPlayerConnected);
		
		m_ScoringSystem = gameMode.GetScoringSystemComponent();
		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Insert(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Insert(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Insert(OnFactionScoreChanged);
			
			array<int> players = {};
			PlayerManager playerManager = GetGame().GetPlayerManager();
			playerManager.GetPlayers(players);
			
			m_aAllPlayersInfo.Clear();
			foreach (int playerId : players)
				m_aAllPlayersInfo.Insert(playerId, m_ScoringSystem.GetPlayerScoreInfo(playerId));
		}
		
		m_RespawnSystem = SCR_RespawnSystemComponent.Cast(gameMode.FindComponent(RespawnSystemComponent));
		
		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}

		m_wTable = m_wRoot.FindAnyWidget("Table");
		
		// Create navigation buttons
		Widget footer = m_wRoot.FindAnyWidget("FooterLeft");
		Widget footerBack = m_wRoot.FindAnyWidget("Footer");
		SCR_NavigationButtonComponent back = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", footerBack);
		if (back)
			back.m_OnActivated.Insert(OnBack);
		
		m_Friend = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Friend", footer);
		if (m_Friend)
		{
			m_Friend.SetEnabled(false);
			m_Friend.m_OnActivated.Insert(OnAddFriend);
		}
		
		m_Block = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Block", footer);
		if (m_Block)
		{
			m_Block.SetEnabled(false);
			m_Block.m_OnActivated.Insert(OnBlock);
		}

		m_Mute = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Mute", footer);
		if (m_Mute)
		{
			m_Mute.SetEnabled(false);
			m_Mute.m_OnActivated.Insert(OnMute);
		}

		SCR_NavigationButtonComponent filter = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Filter", footer);
		if (filter)
			filter.m_OnActivated.Insert(OnFilter);
		
		m_Vote = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Vote", footer);
		if (m_Vote)
		{
			if (m_VotingManager)
				m_Vote.m_OnActivated.Insert(OnVoting);
			else
				m_Vote.SetVisible(false,false);
		}
		
		m_Invite = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Invite", footer);
		if (m_Invite)
		{
			if (m_playerGroupController)
				m_Invite.m_OnActivated.Insert(OnInvite);
			else 
				m_Invite.SetVisible(false, false);
		}		
		

		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		
		
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);		
	
			foreach (int id : ids)
			{
				CreateEntry(id, editorDelegateManager);
			}
		
		InitSorting();
		
		Widget tab = m_wRoot.FindAnyWidget("TabViewRoot0");
		if (!tab)
			return;
		
		SCR_TabViewComponent tabView = SCR_TabViewComponent.Cast(tab.FindHandler(SCR_TabViewComponent));
		if (!tabView)
			return;
		
		tabView.m_OnChanged.Insert(OnTabChanged);

		// Create new tabs
		SCR_Faction scrFaction;
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;
			
			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string name = faction.GetFactionName();
			tabView.AddTab(ResourceName.Empty,name);
		}
		
		//handle groups tab
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || !SCR_RespawnSystemComponent.GetInstance().GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId()) )
			tabView.SetTabVisible(EPlayerListTab.GROUPS, false);
		UpdateFrequencies();
		
		s_OnPlayerListMenu.Invoke(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		m_fTimeSkip = m_fTimeSkip + tDelta;
		
		if (m_fTimeSkip >= TIME_STEP)
		{
			DeleteDisconnectedEntries();
			m_fTimeSkip = 0.0;
		}
		
		GetGame().GetInputManager().ActivateContext("PlayerMenuContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained() 
	{
		GetGame().GetInputManager().AddActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);
		GetGame().GetInputManager().AddActionListener("MenuOpen", EActionTrigger.DOWN, OpenPauseMenu);
#ifdef WORKBENCH
		GetGame().GetInputManager().AddActionListener("MenuOpenWB", EActionTrigger.DOWN, OpenPauseMenu);
#endif
		
		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
		
		FocusFirstItem();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);
		GetGame().GetInputManager().RemoveActionListener("MenuOpen", EActionTrigger.DOWN, OpenPauseMenu);
#ifdef WORKBENCH
		GetGame().GetInputManager().RemoveActionListener("MenuOpenWB", EActionTrigger.DOWN, OpenPauseMenu);
#endif
		
		//--- Close when some other menu is opened on top
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose() 
	{
		super.OnMenuClose();

		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(controller.FindComponent(SCR_HUDManagerComponent));
		if (hudManager)
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() | EHudLayers.HIGH);
		
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		
		//Remove the subscriptions to player right changed
		if (editorDelegateManager)
		{
			foreach (SCR_PlayerListEntry entry: m_aEntries)
			{
				if (entry)
				{
					SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(entry.m_iID);
					
					if (playerEditorDelegate)
					{
						playerEditorDelegate.GetOnLimitedEditorChanged().Remove(OnEditorRightsChanged);
					}
				}
			}
		}
		
		m_aAllPlayersInfo.Clear();
		m_aFactions.Clear();
		
		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Remove(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Remove(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Remove(OnFactionScoreChanged);
		}
		
		s_OnPlayerListMenu.Invoke(false);
	}

	// Call updates with delay, so name is synced properly
	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int id)
	{
		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 1000, false, true, id);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerList(bool addPlayer, int id)
	{
		if (addPlayer)
		{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			CreateEntry(id, editorDelegateManager);
			
			// Get current sort method and re-apply sorting
			if (!m_Header)
				return;
			
			OnHeaderChanged(m_Header);
		}
		else
		{
			// Delete entry from the list
			SCR_PlayerListEntry playerEntry;
			foreach (SCR_PlayerListEntry entry : m_aEntries)
			{
				if (entry.m_iID != id)
					continue;
				
				playerEntry = entry;
				break;
			}
			
			if (!playerEntry)
				return;
			
			playerEntry.m_wRow.RemoveFromHierarchy();
			m_aEntries.RemoveItem(playerEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFrequencies()
	{		
		SCR_GadgetManagerComponent gadgetManager;
		IEntity localPlayer = SCR_PlayerController.GetLocalMainEntity();
		Faction localFaction;
		set<int> localFrequencies = new set<int>();
		if (localPlayer)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(localPlayer.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, localFrequencies);
			
			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(localPlayer.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				localFaction = factionAffiliation.GetAffiliatedFaction();
		}
		
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Faction == localFaction)
			{
				IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(entry.m_iID);
				if (playerEntity)
				{
					//--- ToDo: Don't extract frequencies locally; do it on server and distribute values to all clients
					gadgetManager = SCR_GadgetManagerComponent.Cast(playerEntity.FindComponent(SCR_GadgetManagerComponent));
					set<int> frequencies = new set<int>();
					SCR_Global.GetFrequencies(gadgetManager, frequencies);
					if (!frequencies.IsEmpty())
					{
						entry.m_iSortFrequency = frequencies[0];
						entry.m_wFreq.SetText(SCR_FormatHelper.FormatFrequencies(frequencies, localFrequencies));
						continue;
					}
				}
			}
			entry.m_iSortFrequency = int.MAX;
			entry.m_wFreq.SetText("-");
		}
		GetGame().GetCallqueue().CallLater(UpdateFrequencies, 1000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateScore()
	{		
		if (!m_aAllPlayersInfo || m_aAllPlayersInfo.Count() == 0 || !m_ScoringSystem)
			return;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{			
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != entry.m_iID)
					continue;
				
				entry.m_Info = info;
				break;
			}

			if (!entry.m_Info)
				continue;
			
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);
				
				entry.m_wScore.SetText(score.ToString());
			}
		}
		
		// Get current sort method and re-apply sorting
		if (!m_Header)
			return;
		
		OnHeaderChanged(m_Header);
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseAllVoting()
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if( entry.m_VotingCombo && entry.m_VotingCombo.IsOpened())
				entry.m_VotingCombo.CloseList();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteDisconnectedEntries()
	{
		for (int i = m_aEntries.Count() - 1; i >= 0; i-- )
		{
			if (GetGame().GetPlayerManager().IsPlayerConnected(m_aEntries[i].m_iID))
				continue;
			m_aEntries[i].m_wRow.RemoveFromHierarchy();
			m_aEntries.RemoveItem(m_aEntries[i]);
		}
	}
};
