//! Interactive non-focus button showing hint, and triggering actions

//------------------------------------------------------------------------------------------------
class SCR_NavigationButtonComponent : SCR_ButtonBaseComponent
{
	[Attribute()]
	protected string m_sLabel;

	[Attribute()]
	protected string m_sActionName;
	
	[Attribute(desc: "Instead of using the action above for hint display, use the one set directly in the Action RichTextWidget")]
	protected bool m_bUseLayoutActionHint;

	[Attribute("0", "If true action icon should fadeout if context of given action is not active")]
	protected bool m_bFadeAction;

	protected TextWidget m_wLabel;
	protected RichTextWidget m_wActionRichText;

	protected bool m_bIsInvokerSet = false;
	protected bool m_bIsActionActive = false;

	ref ScriptInvoker m_OnActivated = new ref ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wActionRichText = RichTextWidget.Cast(w.FindAnyWidget("Action"));
		m_wLabel = TextWidget.Cast(w.FindAnyWidget("Text"));

		SetAction(m_sActionName);
		SetLabel(m_sLabel);
		
		
		// Fading action icon 
		if (!m_bFadeAction)
			return;

		// Listen to menu edit context change invoke
		ChimeraMenuBase menu = ChimeraMenuBase.CurrentChimeraMenu();
		if (menu)
		{
			menu.m_OnTextEditContextChange.Insert(OnTextEditContextChange);
			m_bIsInvokerSet = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0 || !m_wRoot.IsVisible() || !m_wRoot.IsEnabled())
			return false;

		m_OnActivated.Invoke(this, m_sActionName);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Call this on text edit context change from menu
	//! Hide buton hint if action is on active
	protected void OnTextEditContextChange(bool ctxActive)
	{
		// Check button action active
		bool actionActive = GetGame().GetInputManager().IsActionActive(m_sActionName);

		m_bIsActionActive = actionActive;

		// Set visuals
		if (actionActive)
			AnimateWidget.Opacity(m_wActionRichText, UIConstants.ENABLED_WIDGET_OPACITY, UIConstants.FADE_RATE_FAST, true);
		else
			AnimateWidget.Opacity(m_wActionRichText, UIConstants.DISABLED_WIDGET_OPACITY, UIConstants.FADE_RATE_FAST, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInput()
	{
		if (!m_wRoot)
			return;

		if (!m_wRoot.IsVisibleInHierarchy() || !m_wRoot.IsEnabledInHierarchy())
			return;

		// Bail if attached to menu but menu is not focused
		if (!IsParentMenuFocused())
			return;

		m_OnActivated.Invoke(this, m_sActionName);

		if (m_bCanBeToggled)
			SetToggled(!IsToggled());
		else
			SCR_UISoundEntity.SoundEvent(m_sSoundClicked);
	}

	//------------------------------------------------------------------------------------------------
	string GetLabel()
	{
		return m_sLabel;
	}

	//------------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		m_sLabel = label;
		if (!m_wLabel)
			return;

		m_wLabel.SetVisible(m_sLabel != string.Empty);
		m_wLabel.SetText(m_sLabel);
	}

	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}

	//------------------------------------------------------------------------------------------------
	void SetAction(string action)
	{
		if (!m_wActionRichText)
			return;

		// Remove old listener and add a new one
		GetGame().GetInputManager().RemoveActionListener(m_sActionName, EActionTrigger.DOWN, OnInput);

		m_sActionName = action;
		if (!m_bUseLayoutActionHint)
			m_wActionRichText.SetText(string.Format("<action name='%1' scale='1.25'/>", m_sActionName));
		
		GetGame().GetInputManager().AddActionListener(m_sActionName, EActionTrigger.DOWN, OnInput);
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_NavigationButtonComponent GetNavigationButtonComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_NavigationButtonComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_NavigationButtonComponent, name, parent, searchAllChildren)
			);
		return comp;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFadeAction()
	{
		return m_bFadeAction;
	}

	//------------------------------------------------------------------------------------------------
	void SetFadeAction(bool fade)
	{
		m_bFadeAction = fade;
	}

};