class SCR_GameOverScreenContentUIComponent: ScriptedWidgetComponent
{
	[Attribute("GameOver_Image")]
	protected string m_sImageName;
	
	[Attribute("GameOver_State")]
	protected string m_sTileName;
	
	[Attribute("GameOver_Condition")]
	protected string m_sSubtitleName;
	
	[Attribute("GameOver_Description")]
	protected string m_sDebriefingName;
	
	protected Widget m_wRoot;
	
	/*!
	Fills the widgets of the gameover screen content
	\param gameOverLayout layout that will be spawned which contains the specific endscreen widget
	\param endGameData send to the gameOverLayout to access any additional data
	\param title title of game over screen
	\param subtitle subtitle of game over screen
	\param debriefing debriefing text of game over screen
	\param imageTexture Image shown in game over screen
	\param titleParam title param %1
	\param subtitleParam subtitle param %1
	\param debriefingParam debriefing param %1
	\return Widget endscreen widget
	*/
	void InitContent(SCR_GameModeEndData endGameData, LocalizedString title = string.Empty, LocalizedString subtitle = string.Empty, LocalizedString debriefing = string.Empty, ResourceName imageTexture = ResourceName.Empty, string titleParam = string.Empty, string subtitleParam = string.Empty, string debriefingParam = string.Empty)
	{
		TextWidget titleWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTileName));
		TextWidget subtitleWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sSubtitleName));
		TextWidget debriefingWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sDebriefingName));
		ImageWidget image = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sImageName));
		
		if (titleWidget)
			titleWidget.SetTextFormat(title, titleParam);

		if (subtitleWidget)
			subtitleWidget.SetTextFormat(subtitle, subtitleParam);
		
		if (debriefingWidget)
			debriefingWidget.SetTextFormat(debriefing, debriefingParam);
		
		
		if (image && !imageTexture.IsEmpty())
		{
			image.LoadImageTexture(0, imageTexture);
		}
	}
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
	override void HandlerDeattached(Widget w)
	{
	}
};
