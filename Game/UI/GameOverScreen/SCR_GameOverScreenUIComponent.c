class SCR_GameOverScreenUIComponent: ScriptedWidgetComponent
{	
	[Attribute("Base_ContentHolder")]
	protected string m_sContentHolderName;
	
	[Attribute("Base_Background")]
	protected string m_sBackgroundName;
	
	[Attribute("Base_ColorOverlay")]
	protected string m_sBackgroundColorOverlayName;
	
	[Attribute("#AR-PauseMenu_ReturnTitle", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMainMenuPopUpTitle;
	
	[Attribute("#AR-PauseMenu_ReturnText", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMainMenuPopUpMessage;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", params: "imageset")]
	protected ResourceName m_sMainMenuPopUpImageSet;
	
	[Attribute("exit")]
	protected string m_sMainMenuPopUpImage;
	
	[Attribute("Back")]
	protected string m_sBackButtonName;
	
	[Attribute("ChatButton")]
	protected string m_sChatButtonName;
	
	[Attribute("0.05")]
	protected float m_fBackgroundColorOverlayOpacity;
	
	[Attribute("2")]
	protected float m_fContentFadeInSpeed;
	
	protected Widget m_wRoot;
	protected SCR_FadeUIComponent m_OverlayBackgroundColorFadeUIComponent;
	protected SCR_FadeUIComponent m_ContentFadeComponent;
	protected SCR_FadeUIComponent m_BackButtonFadeComponent;
	protected SCR_FadeUIComponent m_ChatButtonFadeComponent;
	
	/*!
	Create the gameover screen, spawn the layout and fading it in
	\param gameOverLayout layout that will be spawned which contains the specific endscreen widget
	\param endGameData send to the gameOverLayout to access any additional data
	\param title title of game over screen
	\param subtitle subtitle of game over screen
	\param debriefing debriefing text of game over screen
	\param imageTexture Image shown in game over screen
	\param vignetteColor Color of background overlay. If null will not be set
	\param titleParam title param %1
	\param subtitleParam subtitle param %1
	\param debriefingParam debriefing param %1
	\return Widget endscreen widget
	*/
	void InitGameOverScreen(ResourceName gameOverLayout, SCR_GameModeEndData endGameData, LocalizedString title = string.Empty, LocalizedString subtitle = string.Empty, LocalizedString debriefing = string.Empty, ResourceName imageTexture = ResourceName.Empty, Color vignetteColor = null, string titleParam = string.Empty, string subtitleParam = string.Empty, string debriefingParam = string.Empty)
	{
		Widget contentHolder = m_wRoot.FindAnyWidget(m_sContentHolderName);
		if (!contentHolder)
			return;
		
		//Spawn gameover layout content
		if (!gameOverLayout.IsEmpty())
		{
			Widget gameOverContent = GetGame().GetWorkspace().CreateWidgets(gameOverLayout, contentHolder);
			
			if (gameOverContent)
			{
				SCR_GameOverScreenContentUIComponent gameOverWidgetContent = SCR_GameOverScreenContentUIComponent.Cast(gameOverContent.FindHandler(SCR_GameOverScreenContentUIComponent));
			
				if (gameOverWidgetContent)
					gameOverWidgetContent.InitContent(endGameData, title, subtitle, debriefing, imageTexture, titleParam, subtitleParam, debriefingParam);
			}
		}
		
		if (vignetteColor)
		{
			ImageWidget colorOverlay =  ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundColorOverlayName));
			if (colorOverlay)
				colorOverlay.SetColor(vignetteColor);
		}
		
		if (m_ContentFadeComponent)
			m_ContentFadeComponent.FadeIn();
		if (m_BackButtonFadeComponent)
			m_BackButtonFadeComponent.FadeIn();
		if (m_ChatButtonFadeComponent)
			m_ChatButtonFadeComponent.FadeIn();
		if (m_OverlayBackgroundColorFadeUIComponent)
			m_OverlayBackgroundColorFadeUIComponent.FadeIn();
	}
	
	/*!
	Sets the popup dialog text created in GameOverScreenInput
	\param popup dialog to set the text of
	*/
	void SetMainMenuPopUpTexts(DialogUI popup)
	{
		popup.SetMessage(m_sMainMenuPopUpMessage);
		popup.SetTitle(m_sMainMenuPopUpTitle);
		popup.SetTitleIcon(m_sMainMenuPopUpImageSet, m_sMainMenuPopUpImage);
	}
	
	//~ Removes itself from hierargy on game end just in cause
	protected void OnGameEnd()
	{
		m_wRoot.RemoveFromHierarchy();
	}
	
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_wRoot = w;
		
		ImageWidget colorOverlay =  ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundColorOverlayName));
		if (colorOverlay)
		{
			colorOverlay.SetOpacity(m_fBackgroundColorOverlayOpacity);
			m_OverlayBackgroundColorFadeUIComponent = SCR_FadeUIComponent.Cast(colorOverlay.FindHandler(SCR_FadeUIComponent));
			
			if (m_OverlayBackgroundColorFadeUIComponent)
			{
				m_OverlayBackgroundColorFadeUIComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				colorOverlay.SetVisible(false);
			}
		}
		
		Widget contentHolder = w.FindAnyWidget(m_sContentHolderName);
		if (contentHolder)
		{
			m_ContentFadeComponent = SCR_FadeUIComponent.Cast(contentHolder.FindHandler(SCR_FadeUIComponent));
			
			if (m_ContentFadeComponent)
			{
				m_ContentFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				contentHolder.SetVisible(false);
			}
		}
			
		Widget backButton = w.FindAnyWidget(m_sBackButtonName);
		if (backButton)
		{
			m_BackButtonFadeComponent = SCR_FadeUIComponent.Cast(backButton.FindHandler(SCR_FadeUIComponent));
			if (m_BackButtonFadeComponent)
			{
				m_BackButtonFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				backButton.SetVisible(false);
			}
		}
		
		Widget chatButton = w.FindAnyWidget(m_sChatButtonName);
		if (chatButton)
		{
			m_ChatButtonFadeComponent = SCR_FadeUIComponent.Cast(chatButton.FindHandler(SCR_FadeUIComponent));
			if (m_ChatButtonFadeComponent)
			{
				m_ChatButtonFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				chatButton.SetVisible(false);
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
		if (gameMode)
			gameMode.GetOnGameEnd().Insert(OnGameEnd);
	}
	
	override void HandlerDeattached(Widget w)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
		if (gameMode)
			gameMode.GetOnGameEnd().Remove(OnGameEnd);
	}
};


