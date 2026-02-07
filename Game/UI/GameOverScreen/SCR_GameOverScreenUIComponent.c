class SCR_GameOverScreenUIComponent: ScriptedWidgetComponent
{	
	[Attribute("TabView")]
	protected string m_sTabViewName;
	
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
	
	[Attribute("0", desc: "Which tab will show the end screen content eg: Victory, Defeat, etc.")]
	protected float m_iContentTabIndex;
	
	protected Widget m_wRoot;
	protected Widget m_wEndscreenContent;
	protected SCR_TabViewComponent m_TabViewComponent;
	protected SCR_FadeUIComponent m_OverlayBackgroundColorFadeUIComponent;
	protected SCR_FadeUIComponent m_ContentFadeComponent;
	protected SCR_FadeUIComponent m_TabViewFadeComponent;
	protected SCR_FadeUIComponent m_BackButtonFadeComponent;
	protected SCR_FadeUIComponent m_ChatButtonFadeComponent;
	
	//~ On tab changed
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget w, int tabIndex)
	{
		OnEndScreenContentShow(tabIndex == m_iContentTabIndex);
	}
	
	//~ When tabview fade is done
	protected void OnTabViewFadeDone()
	{
		if (m_TabViewComponent)
			m_TabViewComponent.EnableAllTabs(true);
	}
	
	//~ Show end screen content eg: Victory, defeat, etc.
	protected void OnEndScreenContentShow(bool show)
	{
		if (m_wEndscreenContent)
			m_wEndscreenContent.SetVisible(show);
	}
	
	/*!
	Create the gameover screen, spawn the layout and fading it in
	\param endScreenUIContent contains the layout and any neccessary information for the endscreen content widget
	*/
	void InitGameOverScreen(SCR_GameOverScreenUIContentData endScreenUIContent)
	{	
		//~ Already has end screen
		if (m_wEndscreenContent)
			return;
		
		Widget contentHolder = m_wRoot.FindAnyWidget(m_sContentHolderName);
		if (!contentHolder)
			return;
		
		//Spawn gameover layout content
		if (!endScreenUIContent.m_sGameOverLayout.IsEmpty())
		{
			m_wEndscreenContent = GetGame().GetWorkspace().CreateWidgets(endScreenUIContent.m_sGameOverLayout, contentHolder);
			
			if (m_wEndscreenContent)
			{
				SCR_GameOverScreenContentUIComponent gameOverWidgetContent = SCR_GameOverScreenContentUIComponent.Cast(m_wEndscreenContent.FindHandler(SCR_GameOverScreenContentUIComponent));
			
				if (gameOverWidgetContent)
					gameOverWidgetContent.InitContent(endScreenUIContent);
			}
		}
		
		if (endScreenUIContent.m_cVignetteColor)
		{
			ImageWidget colorOverlay =  ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundColorOverlayName));
			if (colorOverlay)
				colorOverlay.SetColor(endScreenUIContent.m_cVignetteColor);
		}
		
		if (m_ContentFadeComponent)
			m_ContentFadeComponent.FadeIn();
		if (m_TabViewFadeComponent)
			m_TabViewFadeComponent.FadeIn();
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
		
		Widget tabView = w.FindAnyWidget(m_sTabViewName);
		if (tabView)
		{
			m_TabViewComponent = SCR_TabViewComponent.Cast(tabView.FindHandler(SCR_TabViewComponent));
			m_TabViewFadeComponent = SCR_FadeUIComponent.Cast(tabView.FindHandler(SCR_FadeUIComponent));
			
			if (m_TabViewFadeComponent)
			{
				m_TabViewFadeComponent.SetFadeInSpeed(m_fContentFadeInSpeed);
				tabView.SetVisible(false);
				m_TabViewFadeComponent.GetOnFadeDone().Insert(OnTabViewFadeDone);
			}
		}
			
		if (m_TabViewComponent)
		{
			if (m_TabViewFadeComponent)
				m_TabViewComponent.EnableAllTabs(false);
			m_TabViewComponent.m_OnChanged.Insert(OnTabChanged);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
		if (gameMode)
			gameMode.GetOnGameEnd().Remove(OnGameEnd);
	}
};





