//------------------------------------------------------------------------------------------------
class SplashScreen: BaseLoadingAnim
{
	const string SPLASH_LAYOUT = "{A1CE9D1EC16DA9BE}UI/layouts/Menus/MainMenu/SplashScreen.layout";
	const string BASIC_LAYOUT = "{C925ADF957A9670A}UI/layouts/Menus/MainMenu/PreloadScreen.layout";
	protected SCR_LoadingSpinner m_Spinner;
	static bool s_SplashShowed = false;
	
	//------------------------------------------------------------------------------------------------
	override void Load()
	{
		m_wRoot = m_WorkspaceWidget.CreateWidgets(BASIC_LAYOUT,m_WorkspaceWidget);
		if (BaseLoadingAnim.s_NumReloads > 0 || s_SplashShowed)
		{
			// Show basic loading screen
			Widget w = m_wRoot.FindAnyWidget("Spinner");
			if (w)
				m_Spinner = SCR_LoadingSpinner.Cast(w.FindHandler(SCR_LoadingSpinner));
		}
		else
		{
			// Show splash screen
			SplashScreenSequence.s_wLayout = m_WorkspaceWidget.CreateWidgets(SPLASH_LAYOUT);
			SplashScreenSequence.s_Sequence = new SplashScreenSequence(SplashScreenSequence.s_wLayout);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		super.Update(timeSlice, progress, minDurationRatio);
		if (SplashScreenSequence.s_Sequence)
			SplashScreenSequence.s_Sequence.Update(timeSlice, progress);
		else if (m_Spinner)
			m_Spinner.Update(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsFirstLoadingScreen()
	{
		return BaseLoadingAnim.s_NumReloads == 0 && !s_SplashShowed;
	}
	
	static void SplashShowed()
	{
		s_SplashShowed = true;
	}
};

//---------------------------------------------------------------------------------------------
class SplashScreenSequence
{
	const float UPDATE_TIME = 100;
	static float m_fWorldTime;
	static ref Widget s_wLayout;
	static ref SplashScreenSequence s_Sequence;
	static bool m_bShown = false;

	protected BaseWorld m_World;
	protected float m_fTime;
	protected int m_iPhase;
	protected Widget m_wLogoBI;
	protected Widget m_wFadeImage;
	protected Widget m_wMap;
	protected Widget m_wDust1;
	protected Widget m_wDust2;
	
	protected const float m_fFadeoutTime = 0.25;
	protected const float m_fPhase1Time = 3;
	protected const float m_fPhase2Time = 3.5;
	protected const float m_fPhase3Time = 4;
	protected const float m_fMapWidth = 3840;
	protected const float m_fMapHeight = 2160;
	protected const float m_fInvokeCloseOpacity = 0.5;
	
	Widget m_wRoot;
	protected SCR_LoadingSpinner m_Spinner;
	protected ref AnimateWidget m_Animator;
	
	protected ref ScriptInvoker Event_OnClose;
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnClose()
	{
		if (Event_OnClose)
			Event_OnClose.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnClose()
	{
		if (!Event_OnClose)
			Event_OnClose = new ScriptInvoker();

		return Event_OnClose;
	}
	
	//---------------------------------------------------------------------------------------------
	void SplashScreenSequence(Widget root)
	{
		m_wRoot = root;
		
		m_fTime = 0;
		m_wRoot.SetZOrder(100000);
		
		m_wMap = m_wRoot.FindAnyWidget("Map");
		m_wLogoBI = m_wRoot.FindAnyWidget("BILogo");
		m_wFadeImage = m_wRoot.FindAnyWidget("FadeImage");
		m_wDust1 = m_wRoot.FindAnyWidget("Dust1");
		m_wDust2 = m_wRoot.FindAnyWidget("Dust2");
		
		m_Animator = AnimateWidget.GetInstance();
		if (!m_Animator)
			m_Animator = new AnimateWidget();
		
		Widget spinner = m_wRoot.FindAnyWidget("Spinner");
		if (spinner)
			m_Spinner = SCR_LoadingSpinner.Cast(spinner.FindHandler(SCR_LoadingSpinner));
		
		// Setup first phase
		m_wLogoBI.SetVisible(false);
		m_wMap.SetVisible(false);
		
		m_wFadeImage.SetOpacity(1);
		m_wFadeImage.SetVisible(true);
		
 		Fade(m_wFadeImage, false, 0.35);
		ShowBohemia();
		
		// Skip splash screen if custom world is present, or nosplash parameter
		string world;
		if (System.IsCLIParam("nosplash") || System.GetCLIParam("world", world) /*|| Game().IsDev()*/)
			m_iPhase = 1;
	}
	
	// Update from loading anim
	//---------------------------------------------------------------------------------------------
	void Update(float timeSlice, float progress)
	{
		m_fTime += timeSlice;
		m_Animator.UpdateAnimations(timeSlice);
		
		if (m_Spinner)
			m_Spinner.Update(timeSlice);
	}

	// Update from callqueue
	//---------------------------------------------------------------------------------------------
	bool Update(float timeSlice)
	{
		m_fTime += timeSlice;
		m_Animator.UpdateAnimations(timeSlice);
		
		if (m_Spinner)
			m_Spinner.Update(timeSlice);
		
		// Check for starting a new sequence
		switch (m_iPhase)
		{
			case 0:
				if (m_fTime > m_fPhase1Time)
				{
					// Fade out the logo screen
					m_iPhase++;
					Fade(m_wFadeImage, true, m_fFadeoutTime);
				}
			break;
			case 1:
				if (m_fTime > m_fPhase2Time)
				{
					m_iPhase++;
					FadeOutSplashScreen();
				}
			break;
			case 2:
			
				// Invoke close during closing fadout before closing anim is fully done 
				if (m_wRoot.GetOpacity() <= m_fInvokeCloseOpacity)
					InvokeEventOnClose();
			
				if (m_fTime > m_fPhase3Time)
				{
					InvokeEventOnClose();
					DeleteSequence();
					return true;
				}
			break;
		}
		
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	void DeleteSequence()
	{
		if (s_wLayout)
		{
			s_wLayout.SetVisible(false);
			s_wLayout.RemoveFromHierarchy();
			s_wLayout = null;
		}
		
		SplashScreenSequence.s_Sequence = null;
	}
	
	//---------------------------------------------------------------------------------------------
	protected void Fade(Widget w, bool show, float time)
	{
		// Prevent null division
		if (time <= 0)
			time = 0.0001;
		
		AnimateWidget.Opacity(w, show, 1 / time);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void FadeOutSplashScreen()
	{
		m_wLogoBI.SetVisible(false);
		m_wMap.SetVisible(false);
		m_wDust2.SetVisible(false);
		m_wDust1.SetVisible(false);
		AnimateWidget.Opacity(m_wRoot, 0, 1 / m_fFadeoutTime);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ShowBohemia()
	{
		ResetMapAnimation();
		ResetDustAnimation();

		// Show animated logo
		m_wLogoBI.SetVisible(true);
		m_wLogoBI.SetOpacity(0);
		AnimateWidget.Opacity(m_wLogoBI, 1, 1);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ResetMapAnimation()
	{
		m_wMap.SetVisible(true);
		m_wMap.SetOpacity(1);
		float size[2] = {m_fMapWidth * 0.45, m_fMapHeight * 0.45};
		AnimateWidget.Size(m_wMap, size, 0.025);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void ResetDustAnimation()
	{
		m_wDust1.SetVisible(true);
		m_wDust1.SetOpacity(1);
		m_wDust2.SetVisible(true);
		m_wDust2.SetOpacity(1);

		float pos[2] = {10000, 0};
		AnimateWidget.Position(m_wDust1, pos, 0.0025);
		AnimateWidget.Position(m_wDust2, pos, 0.0005);
	}
};