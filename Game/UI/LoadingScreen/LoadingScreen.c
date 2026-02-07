class ArmaReforgerLoadingAnim: BaseLoadingAnim
{
	// TODO: Use .gproj definition of default loading screen attributes
	const string PERCENT_STRING = "#AR-ValueUnit_Percentage";
	const string LAYOUT = "{67DDD8A66C5FC5D3}UI/layouts/Menus/loadingScreen.layout";
	protected const float PERCENT_COUNTING_TIMEOUT = 1;
	protected const string AUTHOR_FORMAT = "#AR-AuthorLoadingScreen" + " ";
	protected const string AUTHOR_UNKNOWN = "#AR-Author_Unknown";

	protected Widget m_wBasicScreen;
	protected Widget m_wProgressBar;
	protected Widget m_wProgressSpace;
	protected TextWidget m_wProgressText;
	
	protected Widget m_wLoadingFrame;
	protected ImageWidget m_wMissionImage;
	protected TextWidget m_wTitle;
	protected TextWidget m_wAuthor;
	protected TextWidget m_wDescription;
	protected ImageWidget m_wLogo;
	
	protected SCR_LoadingSpinner m_Spinner1;
	protected SCR_LoadingSpinner m_Spinner2;
	protected SCR_LoadingHintComponent m_Hint;
	
	protected Widget m_wCrossplayWarning;
	protected Widget m_wModdedWarning;

	protected bool m_bMissionSet;
	protected float m_fProgress;
	protected float m_fTotalLoadingTime;

	static protected bool s_bOpened;
	static ref ScriptInvoker s_OnEnterLoadingScreen = new ScriptInvoker();
	static ref ScriptInvoker m_onExitLoadingScreen = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void Load()
	{
		super.Load();
		
		if (!m_wRoot)
			return;
		
		m_wRoot.SetZOrder(UIConstants.LOADING_SCREEN_Z_ORDER);
		m_wBasicScreen = m_wRoot.FindAnyWidget("BasicScreen");
		m_wLoadingFrame = m_wRoot.FindAnyWidget("LoadingScreenFull");
		m_wTitle = TextWidget.Cast(m_wRoot.FindAnyWidget("Title"));
		m_wDescription = TextWidget.Cast(m_wRoot.FindAnyWidget("Description"));
		m_wAuthor = TextWidget.Cast(m_wRoot.FindAnyWidget("Author"));
		m_wMissionImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("LoadingImage"));
		m_wLogo = ImageWidget.Cast(m_wRoot.FindAnyWidget("Logo"));
		m_wProgressBar = m_wRoot.FindAnyWidget("LoadingProgress");
		m_wProgressSpace = m_wRoot.FindAnyWidget("LoadingSpacer");
		m_wProgressText = TextWidget.Cast(m_wRoot.FindAnyWidget("ProgressText"));
		m_wCrossplayWarning = m_wRoot.FindAnyWidget("LoadingNote_CrossPlay");
		m_wModdedWarning = m_wRoot.FindAnyWidget("LoadingNote_ModdedGame");
		
		Widget spinner1 = m_wRoot.FindAnyWidget("Spinner1").FindAnyWidget("Spinner");
		if (spinner1)
			m_Spinner1 = SCR_LoadingSpinner.Cast(spinner1.FindHandler(SCR_LoadingSpinner));
		
		Widget spinner2 = m_wRoot.FindAnyWidget("Spinner2").FindAnyWidget("Spinner");
		if (spinner2)
			m_Spinner2 = SCR_LoadingSpinner.Cast(spinner2.FindHandler(SCR_LoadingSpinner));
		
		Widget hint = m_wRoot.FindAnyWidget("HintText");
		if (hint)
			m_Hint = SCR_LoadingHintComponent.Cast(hint.FindHandler(SCR_LoadingHintComponent));

		if (m_wBasicScreen)
			m_wBasicScreen.SetVisible(true);
		if (m_wLogo)
			m_wLogo.SetVisible(true);
		if (m_wLoadingFrame)
			m_wLoadingFrame.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void ArmaReforgerLoadingAnim(WorkspaceWidget workspaceWidget)
	{
		GetGame().m_OnMissionSetInvoker.Insert(ShowFullLoadingScreen);
	}

	//------------------------------------------------------------------------------------------------
	override Widget CreateLayout(WorkspaceWidget workspaceWidget)
	{
		return workspaceWidget.CreateWidgets(LAYOUT);
	}
	
	// Show initial black screen
	//------------------------------------------------------------------------------------------------
	override void Show()
	{
		// HOTFIX: This prevents consecutive loading screen to show properly
		/*
		// If there is a splash screen, link it to the current workspace
		if (!SplashScreenSequence.m_bShown && SplashScreenSequence.s_Sequence && SplashScreenSequence.s_wLayout)
			m_WorkspaceWidget.AddChild(SplashScreenSequence.s_wLayout);
		*/
		
		super.Show();
		s_bOpened = true;
		
		EnableSounds(false);

		// Exit if full loading screen was shown already
		if (m_bMissionSet)
			return;
		
		if (m_wRoot)
			m_wRoot.SetVisible(true);
		
		if (m_wBasicScreen)
			m_wBasicScreen.SetVisible(true);

		if (m_wLoadingFrame)
			m_wLoadingFrame.SetVisible(false);
		
		GetGame().GetInputManager().SetLoading(true);
		s_OnEnterLoadingScreen.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void EnableSounds(bool enable)
	{
		AudioSystem.SetMasterVolume(AudioSystem.SFX, enable);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, enable);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, enable);
		AudioSystem.SetMasterVolume(AudioSystem.Music, enable);
		AudioSystem.SetMasterVolume(AudioSystem.UI, enable);
	}
	
	// Show full loading screen
	//------------------------------------------------------------------------------------------------
	void ShowFullLoadingScreen(SCR_MissionHeader header)
	{
		Show();
		
		if (!m_wRoot || !m_wLoadingFrame)
			return;
		
		m_bMissionSet = true;
		if (!header)
		{
			// Show only the basic loading screen
			m_wLoadingFrame.SetVisible(false);
			return;
		}
		
		string description = header.m_sDetails;
		if (description == string.Empty)
			description = header.m_sDescription;
		
		string image = header.m_sLoadingScreen;
		if (image == string.Empty)
			image = header.m_sIcon;
	
		if (m_wTitle)
			m_wTitle.SetText(header.m_sName);
		
		if (m_wDescription)
			m_wDescription.SetText(description);
		
		if (m_wAuthor)
		{
			if (header.m_sAuthor.IsEmpty())
				m_wAuthor.SetText(AUTHOR_UNKNOWN);
			else
				m_wAuthor.SetTextFormat(AUTHOR_FORMAT +  header.m_sAuthor);
		}
		
		if (m_wMissionImage)
			SCR_WLibComponentBase.SetTexture(m_wMissionImage, image);

		m_wRoot.SetVisible(true);
		m_wLoadingFrame.SetVisible(true);
		
		if (m_wBasicScreen)
			m_wBasicScreen.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		super.Update(timeSlice, progress, minDurationRatio);
		
		// If there is a splash sequence, update it
		if (SplashScreenSequence.s_Sequence)
			SplashScreenSequence.s_Sequence.Update(timeSlice, progress);
		
		m_fTotalLoadingTime += timeSlice;

		if (m_Spinner1)
			m_Spinner1.Update(timeSlice);
		
		if (m_Spinner2)
			m_Spinner2.Update(timeSlice);
		
		if (m_Hint)
			m_Hint.Update(timeSlice);
		
		if (m_fTotalLoadingTime < PERCENT_COUNTING_TIMEOUT)
			return;
		
		if (!m_wProgressBar || !m_wProgressText || !m_wProgressImage)
			return;
		
		// Make sure progression never goes back
		if (progress < m_fProgress)
			progress = m_fProgress;
		else if (progress > m_fProgress)
			m_fProgress = progress;
		
		HorizontalLayoutSlot.SetFillWeight(m_wProgressBar, progress);
		HorizontalLayoutSlot.SetFillWeight(m_wProgressSpace, 1 - progress);
		m_wProgressText.SetTextFormat(PERCENT_STRING, Math.Floor(progress * 100));
	}

	//------------------------------------------------------------------------------------------------
	override void Hide()
	{
		if (SplashScreenSequence.s_Sequence && SplashScreenSequence.s_wLayout)
		{
			GetGame().GetWorkspace().AddChild(SplashScreenSequence.s_wLayout);
			SCR_UICore.UpdateSplashScreen();
		}

		s_bOpened = false;
		m_bMissionSet = false;
		m_fTotalLoadingTime = 0;
		m_fProgress = 0;
		
		if (m_Hint)
			m_Hint.OnLoadingFinished();
		
		EnableSounds(true);
		GetGame().GetInputManager().SetLoading(false);
		m_onExitLoadingScreen.Invoke();
		super.Hide();
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsOpen()
	{
		return s_bOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetJoiningCrossPlay(bool isCrossPlay)
	{
		m_wCrossplayWarning.SetVisible(isCrossPlay);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLoadingModded(bool isModded)
	{
		m_wModdedWarning.SetVisible(isModded);
	}	
};
