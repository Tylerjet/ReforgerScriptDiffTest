//------------------------------------------------------------------------------------------------
class SCR_CareerEndScreenUI: ScriptedWidgetComponent 
{
	protected Widget m_wRootWidget;
	protected Widget m_wFirstColumnWidget;
	protected Widget m_wCareerSpecializationsWidget;
	
	protected Widget m_wHud;
	protected SCR_CareerProfileHUD m_HudHandler;
	protected SCR_CareerSpecializationsUI m_CareerSpecializationsHandler;
	
	[Attribute(params: "Stats layout")]
	protected ResourceName m_StatsLayout;
	
	[Attribute(params: "Header of Stats layout")]
	protected ResourceName m_HeaderStatsLayout;
	
	[Attribute(params: "Stats Progression layout")]
	protected ResourceName m_ProgressionStatsLayout;
	
	protected static ref SCR_PlayerData m_PlayerData;
	
	[Attribute("#AR-CareerProfile_MatchEnd_Winner", params: "String winner faction")]
	protected string m_sWinnerFaction;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRootWidget = w;
		
		m_wFirstColumnWidget = m_wRootWidget.FindAnyWidget("FirstColumn");
		m_wCareerSpecializationsWidget = m_wRootWidget.FindAnyWidget("CareerSpecializations0");
		
		if (!m_wFirstColumnWidget || !m_wCareerSpecializationsWidget)
			return;
		
		m_wHud = m_wFirstColumnWidget.FindAnyWidget("CareerProfileHUD0");
		
		if (!m_wHud)
			return;
		
		m_HudHandler = SCR_CareerProfileHUD.Cast(m_wHud.FindHandler(SCR_CareerProfileHUD));
		if (!m_HudHandler)
			return;
		m_HudHandler.PrepareHUD("", "RankTitleText", "", "", "LevelProgress", "PlayerLevel", "ProgressBar0", "");
		
		m_CareerSpecializationsHandler = SCR_CareerSpecializationsUI.Cast(m_wCareerSpecializationsWidget.FindHandler(SCR_CareerSpecializationsUI));
		if (!m_CareerSpecializationsHandler)
			return;
		
		SetWinningFaction();
		
		if (!m_PlayerData)
		{
			SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
			if (!dataCollector)
			{
				Print ("SCR_CareerEndScreenUI: No data collector was found.", LogLevel.ERROR);
				return;
			}
			
			m_PlayerData = dataCollector.GetPlayerData(0, false);
			
			//If there's still no player data, we wait for the invoker on data received to let us now that we got the instance through rpl
			if (!m_PlayerData)
			{
				SCR_DataCollectorCommunicationComponent communicationComponent = SCR_DataCollectorCommunicationComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_DataCollectorCommunicationComponent));
				if (communicationComponent)
					communicationComponent.GetOnDataReceived().Insert(OnDataReceived);
				
				return;
			}
			if (!m_PlayerData.IsDataProgressionReady())
				m_PlayerData.CalculateStatsChange();
		}
		
		FillFields();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Data was sent through RPL and now we can use it
	protected void OnDataReceived(SCR_PlayerData playerData)
	{
		m_PlayerData = playerData;
		m_PlayerData.CalculateStatsChange();
		FillFields();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillFields()
	{
		if (!m_PlayerData || !m_PlayerData.IsDataProgressionReady())
			return;
		
		FillHudAndStats();
		FillSpecializationsFrame();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillHudAndStats()
	{
		int Level = m_PlayerData.GetLevelExperience();
		
		m_HudHandler.SetPlayerLevel(Level / SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetProgressBarValue(Level % SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetPlayerRank(m_PlayerData.GetRank());
		m_HudHandler.SetRandomBackgroundPicture();
		
		array<float> EarntPoints = m_PlayerData.GetArrayEarntPoints();
		
		m_HudHandler.SetLevelProgressGain(EarntPoints[SCR_EDataStats.LEVELEXPERIENCE]);
		
		Widget GeneralStatsWidget = m_wFirstColumnWidget.FindAnyWidget("GeneralStatEntries");
		if (!GeneralStatsWidget)
			return;
		
		float warCrimes = (m_PlayerData.GetWarCrimes()) *  (SCR_PlayerDataConfigs.WARCRIMESPUNISHMENT);
		int minutes = (m_PlayerData.GetSessionDuration() - m_PlayerData.GetSessionDuration(false)) / 60;
		
		SCR_CareerUI.CreateProgressionStatEntry(GeneralStatsWidget, m_ProgressionStatsLayout, "#AR-CareerProfile_TimePlayed", EarntPoints[SCR_EDataStats.SESSIONDURATION] * warCrimes, EarntPoints[SCR_EDataStats.SESSIONDURATION], "#AR-CareerProfile_Minutes", ""+minutes);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillSpecializationsFrame()
	{
		m_CareerSpecializationsHandler.SetShowProgression(true);
		m_CareerSpecializationsHandler.FillSpecializations(m_PlayerData, m_StatsLayout, m_HeaderStatsLayout, m_ProgressionStatsLayout);
		m_CareerSpecializationsHandler.FillWarCrimes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWinningFaction()
	{
		ImageWidget matchFlag = ImageWidget.Cast(m_wFirstColumnWidget.FindAnyWidget("MatchResultFlag"));
		RichTextWidget MatchFactionWinner = RichTextWidget.Cast(m_wFirstColumnWidget.FindAnyWidget("MatchFactionWinner"));
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!matchFlag || !MatchFactionWinner || !gamemode)
			return;
		
		SCR_GameModeEndData endData = gamemode.GetEndGameData();
		if (!endData)
			return;
		
		if (endData.GetEndReason() != SCR_GameModeEndData.ENDREASON_EDITOR_FACTION_VICTORY)
		{
			matchFlag.SetVisible(false);
			MatchFactionWinner.SetVisible(false);
			return;
		}
		
		array<int> winningFactionIds = new array<int>;
		endData.GetFactionWinnerIds(winningFactionIds);
		
		if (winningFactionIds.Count() <= 0)
			return;
		
		int factionId = winningFactionIds[0];
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction factionWinner = factionManager.GetFactionByIndex(factionId);
		if (!factionWinner)
			return;
		
		MatchFactionWinner.SetTextFormat(m_sWinnerFaction, factionWinner.GetFactionName());
		
		ResourceName winnerFlag = factionWinner.GetUIInfo().GetIconPath();
		bool success = matchFlag.LoadImageTexture(0, winnerFlag);
		if (!success)
			return;
		
		int x, y;
		matchFlag.GetImageSize(0, x, y);
		matchFlag.SetSize(x, y);
	}
};