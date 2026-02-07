//------------------------------------------------------------------------------------------------
class SCR_CareerProfileOverviewUI: SCR_SubMenuBase
{
	protected Widget m_wRootWidget;
	protected Widget m_wFirstColumnWidget;
	protected Widget m_wSecondColumnWidget;
	protected Widget m_wThirdColumnWidget;
	
	protected Widget m_wHud, m_wSpiderNet;
	protected SCR_CareerProfileHUD m_HudHandler;
	protected SCR_SpiderNet m_SpiderNetHandler;
	protected SCR_LoadoutPreviewComponent m_LoadoutPreviewHandler;
		
	[Attribute("", UIWidgets.Object, category: "Loadout Manager")]
	protected ref SCR_BasePlayerLoadout m_PlayerLoadout;
	
	[Attribute(params: "Stats layout")]
	protected ResourceName m_StatsLayout;
	
	[Attribute(params: "Header of Stats layout")]
	protected ResourceName m_HeaderStatsLayout;
	
	protected Widget m_wCharacterLoadout;
	
	protected ref SCR_PlayerData m_PlayerData;
	
	protected ref array <Widget> m_aSpecializationsStatsWidgets;
	
	static const int XP_NEEDED_FOR_LEVEL = 10000;
	
	protected int m_iAttemptsCounter = 0; //To remove
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_wRootWidget = GetRootWidget();
		if (!m_wRootWidget)
			return;
		
		m_wFirstColumnWidget = m_wRootWidget.FindAnyWidget("FirstColumn");
		m_wSecondColumnWidget = m_wRootWidget.FindAnyWidget("SecondColumn");
		m_wThirdColumnWidget = m_wRootWidget.FindAnyWidget("ThirdColumn");
		if (!m_wFirstColumnWidget || !m_wSecondColumnWidget || !m_wThirdColumnWidget)
			return;
		
		m_wHud = m_wFirstColumnWidget.FindAnyWidget("CareerProfileHUD0");
		if (!m_wHud)
			return;
		
		m_HudHandler = SCR_CareerProfileHUD.Cast(m_wHud.FindHandler(SCR_CareerProfileHUD));
		if (!m_HudHandler)
			return;
		
		m_wSpiderNet = m_wSecondColumnWidget.FindAnyWidget("SpiderNet0");
		if (!m_wSpiderNet)
			return;
		
		m_SpiderNetHandler = SCR_SpiderNet.Cast(m_wSpiderNet.FindHandler(SCR_SpiderNet));
		if (!m_SpiderNetHandler)
			return;
		
		if (!m_PlayerData)
		{
			m_PlayerData = new SCR_PlayerData(0); //In the main menu we have no player ID, we use 0 to indicate that because in local id doesn't matter
		}
		
		fillFields();
		
		m_wCharacterLoadout = m_wHud.FindAnyWidget("CharacterLoadout");
		if (!m_wCharacterLoadout)
			return;
		
		m_LoadoutPreviewHandler = SCR_LoadoutPreviewComponent.Cast(m_wCharacterLoadout.FindHandler(SCR_LoadoutPreviewComponent));
		m_LoadoutPreviewHandler.SetPreviewedLoadout(m_PlayerLoadout);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void fillFields()
	{
		if (m_PlayerData)
		{			
			//We use these evaluations as a placeholder, the proper process is calling the methods after the playerStats have been gathered from backend
			//We are waiting for an evaluation from backend on whether we need a new event or if there's a way for us to know when playerStats are available
			if (m_PlayerData.IsPlayerDataAvailable())
			{
				fillHudAndRank();
				fillSpecializations();
			}
			else
			{
				m_iAttemptsCounter++;
				//Print("Loading player profile from Main Menu. Attempt: "+m_iAttemptsCounter, LogLevel.DEBUG); //To remove
				if (m_iAttemptsCounter > 100)
				{
					Print("SCR:CareerProfileOverviewUi: m_PlayerData is not available. Using an empty profile instead", LogLevel.DEBUG);
					m_PlayerData.LoadEmptyProfile();
					fillFields();
				}
				else
				{
					GetGame().GetCallqueue().CallLater(fillFields, 10); //To remove. The method should listen to an invoker from the callback instead
				}
			}
		}
		else
		{
			Print ("SCR:CareerProfileOverviewUI:FillFields: No player data available", LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void fillHudAndRank()
	{
		if (!m_PlayerData || !m_HudHandler || !m_wFirstColumnWidget)
			return;
		
		int RankExperience = m_PlayerData.GetRankExperience();
		
		m_HudHandler.SetPlayerLevel(RankExperience/XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetProgressBarValue(RankExperience%XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetPlayerRank(m_PlayerData.GetRank());
		m_HudHandler.SetRandomBackgroundPicture();
		
		Widget RankStatsWidget = m_wFirstColumnWidget.FindAnyWidget("RankStatEntries");
		if (!RankStatsWidget)
			return;
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(m_PlayerData.GetSessionDuration(), days, hours, minutes, seconds);
		
		CreateStatEntry(RankStatsWidget, "#AR-CareerProfile_TimePlayed", ""+ days + " "+"#AR-CareerProfile_TimePlayed_Days"+" " + hours + " "+"#AR-CareerProfile_TimePlayed_Hours"+" " + minutes + " "+"#AR-CareerProfile_TimePlayed_Minutes");
		CreateStatEntry(RankStatsWidget, "#AR-CareerProfile_Deaths", ""+m_PlayerData.GetDeaths()+" "+"#AR-CareerProfile_Times");
		CreateHeaderStatEntry(RankStatsWidget, "#AR-CareerProfile_Distance");
		CreateStatEntry(RankStatsWidget, "#AR-CareerProfile_DistanceTravelled_ByFoot"+"...", ""+m_PlayerData.GetDistanceWalked()+" "+"#AR-CareerProfile_KMs");
		CreateStatEntry(RankStatsWidget, "#AR-CareerProfile_DistanceTravelled_AsDriver"+"...", ""+m_PlayerData.GetDistanceDriven()+" "+"#AR-CareerProfile_KMs");
		CreateStatEntry(RankStatsWidget, "#AR-CareerProfile_DistanceTravelled_AsPassenger"+"...", ""+m_PlayerData.GetDistanceAsOccupant()+" "+"#AR-CareerProfile_KMs");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void fillSpecializations()
	{
		if (!m_PlayerData)
			return;
		
		m_SpiderNetHandler.SetSpPoints(m_PlayerData.GetSpecializationPoints());
		m_SpiderNetHandler.DrawSpiderNet();
		m_SpiderNetHandler.RegisterCareerProfileHandler(this);
		
		m_aSpecializationsStatsWidgets = {};
		SetUpSpecializationDisplay();
		UpdateSpecializationStats(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateHeaderStatEntry(Widget container, string text = "")
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget HeaderEntry = Widget.Cast(workspace.CreateWidgets(m_HeaderStatsLayout, container));
		if (!HeaderEntry)
			return null;
		
		TextWidget textWidget;
		
		textWidget = TextWidget.Cast(HeaderEntry.FindAnyWidget("HeaderStatText"));
		if (!textWidget)
			return null;
		
		textWidget.SetText(text);
		
		return HeaderEntry;
	}
	
	protected Widget CreateStatEntry(Widget container, string text = "", string value = "")
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget StatEntry = Widget.Cast(workspace.CreateWidgets(m_StatsLayout, container));
		if (!StatEntry)
			return null;
		
		TextWidget statName, statValue;
		
		statName = TextWidget.Cast(StatEntry.FindAnyWidget("StatName"));
		statValue = TextWidget.Cast(StatEntry.FindAnyWidget("StatValue"));
		if (!statName || !statValue)
			return null;
		
		statName.SetText(text);
		statValue.SetText(value);
		
		return StatEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateStatEntry(string name, string value, Widget StatEntry)
	{
		if (!StatEntry)
			return;
		
		TextWidget statName, statValue;
		
		statName = TextWidget.Cast(StatEntry.FindAnyWidget("StatName"));
		statValue = TextWidget.Cast(StatEntry.FindAnyWidget("StatValue"));
		if (!statName || !statValue)
			return;
		
		statName.SetText(name);
		statValue.SetText(value);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetUpSpecializationDisplay()
	{
		if (!m_PlayerData)
			return;
		
		Widget specializationsWidget = m_wThirdColumnWidget.FindAnyWidget("SpecializationsPanel");
		if (!specializationsWidget)
			return;
		
		int maxCount = m_PlayerData.GetSpecializationCount(0);
		if (m_PlayerData.GetSpecializationCount(1) > maxCount)
			maxCount = m_PlayerData.GetSpecializationCount(1);
		if (m_PlayerData.GetSpecializationCount(2) > maxCount)
			maxCount = m_PlayerData.GetSpecializationCount(2);
		
		for (int i = 0; i < maxCount; i++)
		{
			Widget tempW = CreateStatEntry(specializationsWidget);
			
			if (tempW)
				m_aSpecializationsStatsWidgets.Insert(tempW);
		}
	}
	
	//Legends are responsible for adding a call to this method OnButtonClick
	//------------------------------------------------------------------------------------------------
	void UpdateSpecializationStats(int specializationId)
	{
		if (!m_aSpecializationsStatsWidgets || m_aSpecializationsStatsWidgets.IsEmpty())
			return;
		
		UpdateSpecialization(specializationId);
		
		switch (specializationId)
		{
			case 0:
				UpdateStatEntry("#AR-CareerProfile_DistanceTravelled_ByFoot", ""+m_PlayerData.GetDistanceWalked()+" "+"#AR-CareerProfile_KMs", m_aSpecializationsStatsWidgets[0]);
				UpdateStatEntry("#AR-CareerProfile_PlayersKilled", ""+m_PlayerData.GetPlayerKills()+" "+"#AR-CareerProfile_Kills", m_aSpecializationsStatsWidgets[1]);
				UpdateStatEntry("#AR-CareerProfile_AIKilled", ""+m_PlayerData.GetAIKills()+" "+"#AR-CareerProfile_Kills", m_aSpecializationsStatsWidgets[2]);
				UpdateStatEntry("#AR-CareerProfile_BulletsShot", ""+m_PlayerData.GetBulletsShot()+" "+"#AR-CareerProfile_Rounds", m_aSpecializationsStatsWidgets[3]);
				break;
			case 1:
				SpecializationStatsSetVisible(m_PlayerData.GetSpecializationCount(1));
				UpdateStatEntry("#AR-CareerProfile_DistanceSupplyVehicle", ""+m_PlayerData.GetTraveledDistanceSupplyVehicle() +" "+"#AR-CareerProfile_KMs", m_aSpecializationsStatsWidgets[0]);
				UpdateStatEntry("#AR-CareerProfile_TimeSupplyVehicle", ""+m_PlayerData.GetTraveledTimeSupplyVehicle()/60 +" "+"#AR-CareerProfile_Minutes", m_aSpecializationsStatsWidgets[1]);
				UpdateStatEntry("#AR-CareerProfile_PointsDriverAllies", ""+m_PlayerData.GetPointsAsDriverOfPlayers() +" "+"#AR-CareerProfile_Points", m_aSpecializationsStatsWidgets[2]);
				break;
			case 2:
				SpecializationStatsSetVisible(m_PlayerData.GetSpecializationCount(2));
				UpdateStatEntry("#AR-CareerProfile_BandagesSelf", "WIP"+ " "+"#AR-CareerProfile_Times", m_aSpecializationsStatsWidgets[0]);
				UpdateStatEntry("#AR-CareerProfile_BandagesAllies", "WIP"+ " "+"#AR-CareerProfile_Times", m_aSpecializationsStatsWidgets[1]);
				break;
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSpecialization(int n)
	{
		if (n < 0 || n > 2)
			return;
		
		RichTextWidget SpecializationTitle = RichTextWidget.Cast(m_wThirdColumnWidget.FindAnyWidget("SpecializationTitleText"));
		RichTextWidget SpecializationProgress = RichTextWidget.Cast(m_wThirdColumnWidget.FindAnyWidget("SpecializationProgressText"));
		if (!SpecializationTitle || !SpecializationProgress)
			return;
		
		string title, value;
		switch (n)
		{
			case 0: 
				title = "1. "+"#AR-CareerProfile_Specialization1";
				value = "25.21%";
				break;
			case 1: 
				title = "2. "+"#AR-CareerProfile_Specialization2";
				value = "14.00%";
				break;
			case 2: 
				title = "3. "+"#AR-CareerProfile_Specialization3";
				value = "12.32%";
				break;
		}
		
		SpecializationTitle.SetText(title);
		SpecializationProgress.SetText(value);
		
		SpecializationStatsSetVisible(m_PlayerData.GetSpecializationCount(n));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpecializationStatsSetVisible(int n)
	{
		int statWidgetsCount = m_aSpecializationsStatsWidgets.Count();
		
		if (n < 0 || n > statWidgetsCount)
			return;
		
		int i;
		
		for (i = 0; i < n; i++)
			m_aSpecializationsStatsWidgets[i].SetVisible(true);
		
		for (i = n; i < statWidgetsCount; i++)
			m_aSpecializationsStatsWidgets[i].SetVisible(false);
	}
};