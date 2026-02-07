class SCR_GameOverScreenCombatControlPatrolUIComponent: SCR_GameOverScreenContentUIComponent
{
	[Attribute("GameOver_Time")]
	protected string m_sTime;
	
	[Attribute("GameOver_Tasks")]
	protected string m_sTasks;
	
	protected ref array<SCR_BaseTask> 	m_aFinishedTasks = {};
	protected string 					timeElapsed;
	
	override void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.InitContent(endScreenUIContent);
		
		RichTextWidget titleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTileName));
		RichTextWidget subtitleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sSubtitleName));
		RichTextWidget timeWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTime));
		RichTextWidget tasksWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTasks));
		
		if (titleWidget)
			titleWidget.SetTextFormat(endScreenUIContent.m_sTitle, endScreenUIContent.m_sTitleParam);

		if (subtitleWidget)
			subtitleWidget.SetTextFormat(endScreenUIContent.m_sSubtitle, endScreenUIContent.m_sSubtitleParam);
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(baseGameMode.GetElapsedTime(), days, hours, minutes, seconds);
		
		timeElapsed = string.Format(WidgetManager.Translate("#AR-CareerProfile_TimePlayed_TotalValue", ""+days, ""+hours, ""+minutes, ""+seconds));
		
		if (timeWidget)
			timeWidget.SetTextFormat(string.Format("<color rgba=%1>%2</color> <br/> %3", UIColors.FormatColor(UIColors.CONTRAST_COLOR), "#AR-CombatScenario_Total_Operation_Time",  timeElapsed));
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		taskManager.GetFinishedTasks(m_aFinishedTasks);
		
		string tasksToShow = "<br/>";
		foreach (SCR_BaseTask task : m_aFinishedTasks)
		{
			if (tasksToShow == "<br/>")
				tasksToShow = tasksToShow + string.Format(WidgetManager.Translate(task.GetTitle()));
			else 
				tasksToShow = tasksToShow + "<br/>" + string.Format(WidgetManager.Translate(task.GetTitle()));
		}
		
		if (tasksWidget)
		{
			if (tasksToShow != "<br/>")
				tasksWidget.SetText(string.Format(WidgetManager.Translate("<color rgba=%1>%2</color> %3", UIColors.FormatColor(UIColors.CONTRAST_COLOR), "#AR-CombatScenario_Completed_Tasks",  tasksToShow)));
			else
				tasksWidget.SetText("");
		}
	}
};