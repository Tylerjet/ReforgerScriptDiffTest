//! Interactive non-focus button showing hint, and triggering actions

//------------------------------------------------------------------------------------------------
class SCR_NavigationButtonComponent : SCR_ButtonBaseComponent
{
	[Attribute()]
	protected string m_sLabel;

	[Attribute()]
	protected string m_sActionName;

	[Attribute("true", UIWidgets.CheckBox, "force padding 0, 0, m_iPaddingRight, 0")]
	bool m_bForcePadding;

	[Attribute()]
	protected int m_iPaddingRight;

	[Attribute(desc: "Instead of using the action above for hint display, use the one set directly in the Action RichTextWidget")]
	protected bool m_bUseLayoutActionHint;

	[Attribute("0", "If true action icon should fadeout if context of given action is not active")]
	protected bool m_bFadeAction;

	[Attribute("false", UIWidgets.CheckBox, "Use in built button action colorization or handle it on your own")]
	bool m_bUseActionColorization;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActionDefault;

	[Attribute("1.000000 0.631006 0.246006 1.000000", UIWidgets.ColorPicker)]
	protected ref Color m_ActionHovered;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActionSelected;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActionSelectedHovered;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActionClicked;

	[Attribute("0.074 0.074 0.074 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActionDisabled;

	protected TextWidget m_wLabel;
	protected RichTextWidget m_wActionRichText;

	protected bool m_bIsInvokerSet = false;
	protected bool m_bIsActionActive = false;

	ref ScriptInvoker m_OnActivated = new ref ScriptInvoker();

	protected bool m_bForceDisabled;
	protected bool m_bShouldBeEnabled;
	//protected bool m_bIsInputEnabled;
	protected bool m_bIsInteractionActive;

	protected bool m_bIsHovered;

	//Actions
	protected const string CONFIRM_HOLD_ACTION = "MenuSelectHold";
	protected const string CONFIRM_ACTION = "DialogConfirm";
	protected const string CANCEL_ACTION = "MenuBack";

	protected ref array<DialogUI> m_aDialogs;


	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wActionRichText = RichTextWidget.Cast(w.FindAnyWidget("Action"));
		m_wLabel = TextWidget.Cast(w.FindAnyWidget("Text"));

		SetAction(m_sActionName);
		SetLabel(m_sLabel);

		//m_bIsInputEnabled = IsEnabled();
		ColorizeAction(false);

		if (w && m_bForcePadding)
			AlignableSlot.SetPadding(w, 0, 0, m_iPaddingRight, 0);

		if (GetGame().InPlayMode())
		{
			m_aDialogs = new array<DialogUI>;

			SCR_NavigationButtonHelper.m_OnActiveWidgetInteraction.Insert(OnActiveWidgetInteraction);
			SCR_NavigationButtonHelper.m_OnDialogOpen.Insert(OnDialogOpen);
			SCR_NavigationButtonHelper.m_OnDialogClose.Insert(OnDialogClose);
		}

		m_bShouldBeEnabled = IsEnabled();

		// Fading action icon
		if (!m_bFadeAction)
			return;
		/*
		// Listen to menu edit context change invoke
		ChimeraMenuBase menu = ChimeraMenuBase.CurrentChimeraMenu();
		if (menu)
		{
			menu.m_OnTextEditContextChange.Insert(OnTextEditContextChange);
			m_bIsInvokerSet = true;
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	override void SetEnabled(bool enabled, bool animate = true)
	{
		m_bShouldBeEnabled = enabled;
		SetEnabled_Internal(enabled, animate);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEnabled_Internal(bool enabled, bool animate = true)
	{
		if (!m_bForceDisabled)
			super.SetEnabled(enabled, animate);
		else
			super.SetEnabled(false, animate);
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
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		m_bIsHovered = true;

		if (m_bUseActionColorization && m_wActionRichText)
		{
			ColorizeAction();
		}

		return false;
	}


	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		m_bIsHovered = false;

		if (m_bUseActionColorization && m_wActionRichText)
		{
			ColorizeAction();
		}

		return false;
	}


	//------------------------------------------------------------------------------------------------
	override void OnMenuSelect()
	{

		super.OnMenuSelect();

		if (!m_bCanBeToggled)
		{
			if (m_bUseActionColorization && m_wActionRichText)
			{
				AnimateWidget.Color(m_wActionRichText, m_ActionClicked, m_fAnimationRate);
				GetGame().GetCallqueue().CallLater(ColorizeAction, m_fAnimationTime * 500 + 1, false, true);
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	override void SetToggled(bool toggled, bool animate = true, bool invokeChange = true)
	{
		super.SetToggled(toggled, animate, invokeChange);

		ColorizeAction(animate);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDisabled(bool animate)
	{
		super.OnDisabled(animate);

		if (!m_wActionRichText)
			return;

		ColorizeAction(animate);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnabled(bool animate)
	{
		super.OnEnabled(animate);

		ColorizeAction(animate);
	}

	//------------------------------------------------------------------------------------------------
	void ColorizeAction(bool animate = true, int delay = 0)
	{
		if (delay == 0)
		{
			UpdateActionColor(animate);
			return;
		}

		GetGame().GetCallqueue().Remove(UpdateActionColor);
		GetGame().GetCallqueue().CallLater(UpdateActionColor, delay, false, animate);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateActionColor(bool animate = true)
	{
		if (!m_bUseActionColorization || !m_wActionRichText)
			return;

		Color color;
		bool isHovered = m_bIsHovered;
		if (m_bIsToggled && m_bCanBeToggled)
		{
			if (isHovered)
				color = m_ActionSelectedHovered;
			else
				color = m_ActionSelected;
		}
		else
		{
			if (isHovered)
				color = m_ActionHovered;
			else
				color = m_ActionDefault;
		}
		/*
		if (!m_bIsInputEnabled)
		{
			if (isHovered)
				color = m_ActionHovered;
			else
				color = m_ActionDisabled;
		}
		*/
		if (!IsEnabled())
			color = m_ActionDisabled;

		if (animate)
			AnimateWidget.Color(m_wActionRichText, color, m_fAnimationRate);
		else
		{
			AnimateWidget.StopAnimation(m_wActionRichText, WidgetAnimationColor);
			m_wActionRichText.SetColor(color);
		}
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
		/*
		if (!m_bIsInputEnabled)
			return;
		*/
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


	/*
	//------------------------------------------------------------------------------------------------
	//!Disables the button press -> OnInput() but leaves the button itself enabled to respond to mouse events
	void SetInputEnabled(bool newEnabled, int delay = 0)
	{
		m_bIsInputEnabled = newEnabled;
		ColorizeAction(true, delay);
	}
	*/

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabled(bool forceDisabled, bool animate = true)
	{
		GetGame().GetCallqueue().Remove(SetForceDisabled); //Stop SetForceDisabledWithDelay

		m_bForceDisabled = forceDisabled;

		if (forceDisabled)
			SetEnabled_Internal(false);
		else
			SetEnabled_Internal(m_bShouldBeEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabledWithDelay(bool forceDisabled, bool animate = true, int delay = 0)
	{
		m_bForceDisabled = forceDisabled;

		if (delay == 0)
		{
			SetForceDisabled(forceDisabled, animate);
			return;
		}

		GetGame().GetCallqueue().Remove(SetForceDisabled);
		GetGame().GetCallqueue().CallLater(SetForceDisabled, delay, false, forceDisabled, animate);
	}

	//Handling Input through SCR_NavigationButtonHelper, ei: edit and combo boxes call this to disable buttons during interaction
	//------------------------------------------------------------------------------------------------
	void OnActiveWidgetInteraction(bool isInteractionActive, int delay)
	{
		m_bIsInteractionActive = isInteractionActive;

		if (IsInTopMenu())
			//SetInputEnabled(!isInteractionActive, delay);
			SetForceDisabledWithDelay(isInteractionActive, true, delay);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogOpen(DialogUI dialog)
	{
		m_aDialogs.Insert(dialog);

		//SetInputEnabled(IsInTopMenu());
		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogClose(DialogUI dialog)
	{
		m_aDialogs.RemoveItem(dialog);

		//SetInputEnabled(IsInTopMenu());
		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there are no dailogs or if this button is in the last dialog opened
	bool IsInTopMenu()
	{
		CleanDialogsArray();

		return m_aDialogs.IsEmpty() || m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there if this button is in the last dialog opened
	bool IsInTopDialog()
	{
		CleanDialogsArray();

		return !m_aDialogs.IsEmpty() && m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	//! Remove NULL entries
	void CleanDialogsArray()
	{
		foreach (DialogUI d : m_aDialogs)
		{
			if (!d) m_aDialogs.RemoveItem(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool GetForceDisabled()
	{
		return m_bForceDisabled;
	}

	void SetColorActionDisabled(Color newColor)
	{
		m_ActionDisabled = newColor;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_NavigationButtonComponent FindComponent(notnull Widget w)
	{
		return SCR_NavigationButtonComponent.Cast(w.FindHandler(SCR_NavigationButtonComponent));
	}

	//------------------------------------------------------------------------------------------------
	//DEBUG
	/*
	override bool OnSelect(Widget w, int x, int y)
	{
		Print(string.Format("OnSelect"), LogLevel.WARNING);
		return super.OnSelect(w, x, y);
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		Print(string.Format("OnItemSelected"), LogLevel.WARNING);
		return super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		Print(string.Format("OnFocus"), LogLevel.WARNING);
		return super.OnFocus(w, x, y);
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{

		Print(string.Format("OnFocusLost"), LogLevel.WARNING);
		return super.OnFocusLost(w, x, y);
	}

	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{

		Print(string.Format("OnMouseWheel"), LogLevel.WARNING);
		return super.OnMouseWheel(w, x, y, wheel);
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{

		Print(string.Format("OnMouseButtonDown"), LogLevel.WARNING);
		return super.OnMouseButtonDown(w, x, y, button);
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{

		Print(string.Format("OnMouseButtonUp"), LogLevel.WARNING);
		return super.OnMouseButtonUp(w, x, y, button);
	}

	//! control is one of ControlID
	override bool OnController(Widget w, int control, int value)
	{

		Print(string.Format("OnController"), LogLevel.WARNING);
		return super.OnController(w, control, value);
	}

	override bool OnKeyDown(Widget w, int x, int y, int key)
	{

		Print(string.Format("OnKeyDown"), LogLevel.WARNING);
		return super.OnKeyDown(w, x, y, key);
	}

	override bool OnKeyUp(Widget w, int x, int y, int key)
	{

		Print(string.Format("OnKeyUp"), LogLevel.WARNING);
		return 	super.OnKeyUp(w, x, y, key);
	}

	override bool OnKeyPress(Widget w, int x, int y, int key)
	{

		Print(string.Format("OnKeyPress"), LogLevel.WARNING);
		return super.OnKeyPress(w, x, y, key);
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{

		Print(string.Format("OnChange"), LogLevel.WARNING);
		return super.OnChange(w, x, y, finished);
	}

	override bool OnNeedScroll(Widget w, int x, int y, float prevScrollPosX, float prevScrollPosY, float newScrollPosX, float newScrollPosY)
	{

		Print(string.Format("OnNeedScroll"), LogLevel.WARNING);
		return super.OnNeedScroll(w, x, y, prevScrollPosX, prevScrollPosY, newScrollPosX, newScrollPosY);
	}

	override bool OnModalResult(Widget w, int x, int y, int code, int result)
	{
		Print(string.Format("OnModalResult widget %1, x %2,  y %3,  code %4, result %5", w, x, y, code, result), LogLevel.ERROR);
		return super.OnModalResult(w, x, y, code, result);
	}
	*/
};
