class SCR_SplashScreenComponent : ScriptedWidgetComponent
{
	Widget m_wRoot;
	Widget m_wEaRoot;
	Widget m_wFadeImage;
	ref ScriptInvoker m_OnFinished = new ScriptInvoker();
	
	[Attribute("0.5")]
	float m_fTransitionTime;

	//---------------------------------------------------------------------------------------------
	override private void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wFadeImage = w.FindAnyWidget("Fade");
		m_wEaRoot = w.FindAnyWidget("EARoot");
		w.SetZOrder(10); // To be in front of the main menu
		
		TextWidget action = TextWidget.Cast(w.FindAnyWidget("Action"));
		if (action)
			action.SetTextFormat("#AR-SplashScreen_Continue", "<color rgba='226, 167, 79, 255'><action name='CloseWarning'/></color>");
	}

	//---------------------------------------------------------------------------------------------
	void ShowEAScreen()
	{
		if (!m_wEaRoot)
			return;
		
		Widget dust1 = m_wEaRoot.FindAnyWidget("Dust1");
		Widget dust2 = m_wEaRoot.FindAnyWidget("Dust2");
		dust1.SetOpacity(0);
		dust2.SetOpacity(0);
		
		float pos[2] = {15000, 0};
		WidgetAnimator.PlayAnimation(dust1, WidgetAnimationType.Position, 0.0005, pos[0], pos[1]);
		WidgetAnimator.PlayAnimation(dust1, WidgetAnimationType.Opacity, 1, 0.5);
		
		float pos2[2] = {35000, 0};
		WidgetAnimator.PlayAnimation(dust2, WidgetAnimationType.Position, 0.0005, pos2[0], pos2[1]);
		WidgetAnimator.PlayAnimation(dust2, WidgetAnimationType.Opacity, 1, 0.5);

		// Chat context has menuSelect action. TODO: Make sure that menu context is here
		ActivateContext();
		GetGame().GetInputManager().AddActionListener("CloseWarning", EActionTrigger.DOWN, OnInput);
	}
	
	//---------------------------------------------------------------------------------------------
	void Fade(Widget w, bool show, float time)
	{
		// Prevent null division
		if (time <= 0)
			time = 0.0001;
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Opacity, show, 1 / time);
	}
	
	//---------------------------------------------------------------------------------------------
	void FinishSequence()
	{
		m_wEaRoot.SetVisible(false);
		Fade(m_wRoot, false, m_fTransitionTime * 0.5);
		m_OnFinished.Invoke();
		GetGame().GetCallqueue().CallLater(Close, m_fTransitionTime * 1000 * 0.5);
	}
	
	//---------------------------------------------------------------------------------------------
	void ActivateContext()
	{
		GetGame().GetInputManager().ActivateContext("BetaWarningContext", 1);
		GetGame().GetCallqueue().CallLater(ActivateContext, 0);
	}
	
	//---------------------------------------------------------------------------------------------
	void Close()
	{
		GetGame().GetCallqueue().Remove(ActivateContext);
		m_wRoot.RemoveFromHierarchy();
	}
	
	//---------------------------------------------------------------------------------------------
	void OnInput()
	{
		GetGame().GetInputManager().SetLoading(false);
		WidgetAnimator.PlayAnimation(m_wRoot, WidgetAnimationType.Opacity, 1, 1);
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Fade(m_wFadeImage, true, m_fTransitionTime * 0.5);
		GetGame().GetCallqueue().CallLater(FinishSequence, m_fTransitionTime * 1000 * 0.5);
		GetGame().GetInputManager().RemoveActionListener("CloseWarning", EActionTrigger.DOWN, OnInput);
	}
};