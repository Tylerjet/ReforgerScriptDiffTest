class SCR_VotingStickyNotificationUIComponent: SCR_StickyNotificationUIComponent
{
	[Attribute("1")]
	protected bool m_bListenToPlayerSpecificVotes;
	
	[Attribute("#AR-Notification_Sticky_MultiplePlayerVotes", desc: "When there is more then 1 vote in progress", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMultipleVotesText;
	
	[Attribute("#AR-Notification_Sticky_VoteNow", desc: "Text shown on the second line", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sSingleVoteOptionalLineText;
	
	[Attribute("InstantVote", desc: "Action name icon shown on %1")]
	protected string m_sSingleVoteOptionalAction;
	
	protected SCR_VotingManagerComponent m_VotingManagerComponent;
	
	protected int m_iVotingCount;
	protected bool m_bUsingGamePad;
	
	protected void OnVotingChanged(bool isInit = false)
	{		
		array<EVotingType> validActiveVotingTypes = new array<EVotingType>;
		m_iVotingCount = m_VotingManagerComponent.GetAllVotingsAboutPlayer(m_bListenToPlayerSpecificVotes, validActiveVotingTypes, false, true);
		
		//If no votes active
		if (m_iVotingCount <= 0)
		{
			//Hide notification
			SetStickyActive(false, !isInit);
			return;
		}
		//Multiple votes active
		else if (m_iVotingCount > 1)
		{
			m_Text.SetTextFormat(m_sMultipleVotesText, m_iVotingCount.ToString());
		}
		//One vote active
		else 
		{			
			array<int> voteValues = new array<int>;
			m_VotingManagerComponent.GetAllVotingValues(validActiveVotingTypes[0], voteValues, false, true);
			string playerName = string.Empty;
			
			//Show player name
			if (voteValues[0] != SCR_VotingBase.DEFAULT_VALUE)
			{
				PlayerManager playermanager = GetGame().GetPlayerManager();
				if (playermanager)
					playerName = playermanager.GetPlayerName(voteValues[0]);
			}
			
			m_Text.SetTextFormat(m_VotingManagerComponent.GetVotingInfo(validActiveVotingTypes[0]).GetStickyNotificationText(), playerName);
		}
		
		//~ Set second line visable if only one message (If not gamepad)
		if (m_OptionalMessageLayout)
			m_OptionalMessageLayout.SetVisible(m_iVotingCount == 1 && !m_bUsingGamePad);
		
		//Show Notification
		SetStickyActive(true, !isInit);
	}
	
	protected void OnVotingStart(EVotingType type, int value)
	{
		OnVotingChanged();
	}
	
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		OnVotingChanged();
	}
	
	//~ Hide optional message if using gamepad
	protected void OnInputDeviceChangedToGamepad(bool isGamepad)
	{
		m_bUsingGamePad = isGamepad;
		
		if (m_OptionalMessageLayout)
			m_OptionalMessageLayout.SetVisible(m_iVotingCount == 1 && !m_bUsingGamePad);
	}
	
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{			
		super.OnInit(notificationLog);

		if (m_OptionalMessageLayout)
		{
			m_OptionalMessageLayout.SetVisible(false);
			TextWidget text = TextWidget.Cast(m_OptionalMessageLayout.FindAnyWidget(m_sTextName));
			
			if (text)
			{
				string actionKey = string.Format("<color rgba='226, 167, 79, 255'><action name='%2' scale='1.5/></color>", param2: m_sSingleVoteOptionalAction);
				text.SetTextFormat(m_sSingleVoteOptionalLineText, actionKey);
			}
		}
			
		m_VotingManagerComponent = SCR_VotingManagerComponent.GetInstance();
		if (m_VotingManagerComponent)
		{
			//~ Changed to gamepad
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceChangedToGamepad);
			InputManager inputManager = GetGame().GetInputManager();
			if (inputManager)
				OnInputDeviceChangedToGamepad(!inputManager.IsUsingMouseAndKeyboard());
			
			
			m_VotingManagerComponent.GetOnVotingStart().Insert(OnVotingStart);
			m_VotingManagerComponent.GetOnVotingEnd().Insert(OnVotingEnd);
			OnVotingChanged(true);
		}
	}
	
	protected override void OnButton()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
				
		if (menuManager)
			menuManager.OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	protected override void OnDestroy()
	{
		if (m_VotingManagerComponent)
		{
			m_VotingManagerComponent.GetOnVotingStart().Remove(OnVotingStart);
			m_VotingManagerComponent.GetOnVotingEnd().Remove(OnVotingEnd);
		}
	}	
};
