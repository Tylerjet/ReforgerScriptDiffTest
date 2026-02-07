class SCR_VotingStickyNotificationUIComponent: SCR_StickyNotificationUIComponent
{
	[Attribute("1")]
	protected bool m_bListenToPlayerSpecificVotes;
	
	[Attribute("#AR-Notification_Sticky_MultiplePlayerVotes", desc: "When there is more then 1 vote in progress")]
	protected LocalizedString m_sMultipleVotesText;
	
	protected SCR_VotingManagerComponent m_VotingManagerComponent;
	
	protected void OnVotingChanged(bool isInit = false)
	{		
		array<EVotingType> validActiveVotingTypes = new array<EVotingType>;
		int count;
		
		if (m_bListenToPlayerSpecificVotes)		
			count = m_VotingManagerComponent.GetVotingsAboutPlayer(validActiveVotingTypes);
		else 
			count = m_VotingManagerComponent.GetVotingsNotAboutPlayer(validActiveVotingTypes);
		
		//If no votes active
		if (count <= 0)
		{
			//Hide notification
			SetStickyActive(false, !isInit);
			return;
		}
		//Multiple votes active
		else if (count > 1)
		{
			m_Text.SetTextFormat(m_sMultipleVotesText, count.ToString());
		}
		//One vote active
		else 
		{
			array<int> voteValues = new array<int>;
			m_VotingManagerComponent.GetAllVotingValues(validActiveVotingTypes[0], voteValues);
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
	
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{			
		super.OnInit(notificationLog);
			
		m_VotingManagerComponent = SCR_VotingManagerComponent.GetInstance();
		if (m_VotingManagerComponent)
		{
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
