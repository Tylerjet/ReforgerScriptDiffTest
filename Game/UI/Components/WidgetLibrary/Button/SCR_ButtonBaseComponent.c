//! Base class for any button, regardless its own content

//------------------------------------------------------------------------------------------------
class SCR_ButtonBaseComponent : SCR_WLibComponentBase 
{
	[Attribute("", UIWidgets.CheckBox, "Can the button be only clicked, or also toggled?")]
	bool m_bCanBeToggled;
	
	[Attribute()]
	protected bool m_bIsToggled;
	
	[Attribute("true", UIWidgets.CheckBox, "Use in built button colorization or handle it on your own")]
	bool m_bUseColorization;

	[Attribute("1 1 1 0.1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundDefault;
	
	[Attribute("1 1 1 0.3", UIWidgets.ColorPicker)]
	ref Color m_BackgroundHovered;
	
	[Attribute("0.760 0.392 0.08 0.3", UIWidgets.ColorPicker)]
	ref Color m_BackgroundSelected;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundSelectedHovered;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundClicked;
	
	[Attribute("true")]
	bool m_bShowBorderOnFocus;
	
	[Attribute()]
	bool m_bShowBorderOnHover;

	Widget m_wBackground;
	Widget m_wBorder;
	
	// Arguments passed: SCR_ButtonBaseComponent
	ref ScriptInvoker m_OnClicked = new ref ScriptInvoker();
	// Arguments passed: SCR_ButtonBaseComponent, bool (toggled)
	ref ScriptInvoker m_OnToggled = new ref ScriptInvoker();
	
	// TODO: Rewrite - do not pass widget 
	ref ScriptInvoker m_OnFocus = new ref ScriptInvoker();
	
	// Arguments passed: SCR_ButtonBaseComponent, bool border shown
	ref ScriptInvoker m_OnShowBorder = new ref ScriptInvoker();
	
	protected MenuBase m_ParentMenu; // Pointer back to menu which owns this button, initialized at FindParentMenu
	protected SCR_SubMenuBase m_ParentSubMenu;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wBackground = w.FindAnyWidget("Background");
		m_wBorder = w.FindAnyWidget("Border");
		if (m_wBorder && m_bUseColorization)
			m_wBorder.SetOpacity(0);
		
		ColorizeBackground(false);
		
		if (!m_wRoot.IsEnabled())
			OnDisabled(false);
		
		// Find parent menu. Due to init order, menu component is not attached to menu root widget right now,
		// so it must be done through call queue.
		GetGame().GetCallqueue().CallLater(FindParentMenu, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		if (m_bCanBeToggled)
			SetToggled(!m_bIsToggled);
		m_OnClicked.Invoke(this);
		
		return false;
	}

	// This coloring is unreliable. Disabled until events are fixed
	/*
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (!m_bCanBeToggled)
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundClicked, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (!m_bCanBeToggled)
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundHovered, m_fAnimationRate);
		
		return false;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		if (m_bMouseOverToFocus)
			return false;
		
		if (m_bShowBorderOnHover)
			WidgetAnimator.PlayAnimation(m_wBorder, WidgetAnimationType.Opacity, 1, m_fAnimationRate);

		if (!m_bUseColorization)
			return false;
				
		if (m_bIsToggled)
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundSelectedHovered, m_fAnimationRate);
		else
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundHovered, m_fAnimationRate);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		if (m_bMouseOverToFocus)
			return false;
		
		if (m_bUseColorization)
		{
			if (m_bIsToggled)
				WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundSelected, m_fAnimationRate);
			else
				WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundDefault, m_fAnimationRate);
		}
		
		if (!m_bShowBorderOnHover || (m_bShowBorderOnFocus && GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot))
			return false;
		
		WidgetAnimator.PlayAnimation(m_wBorder, WidgetAnimationType.Opacity, 0, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_bShowBorderOnFocus)
			WidgetAnimator.PlayAnimation(m_wBorder, WidgetAnimationType.Opacity, 1, m_fAnimationRate);
		
		m_OnFocus.Invoke(m_wRoot);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		if (!m_bShowBorderOnFocus || (m_bShowBorderOnHover && WidgetManager.GetWidgetUnderCursor() == m_wRoot))
			return false;
		
		WidgetAnimator.PlayAnimation(m_wBorder, WidgetAnimationType.Opacity, 0, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuSelect()
	{
		if (!m_wRoot.IsEnabled())
			return;
		
		if (m_bCanBeToggled)
		{
			SetToggled(!m_bIsToggled);
		}
		else if (m_bUseColorization)
		{
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, m_BackgroundClicked, m_fAnimationRate);
			GetGame().GetCallqueue().CallLater(ColorizeBackground, m_fAnimationTime * 500 + 1, false, true);
		}
		m_OnClicked.Invoke(this);
	}
	
	// Public API
	//------------------------------------------------------------------------------------------------
	void SetToggled(bool toggled, bool animate = true, bool invokeChange = true)
	{
		if (!m_bCanBeToggled)
			return;
		
		m_bIsToggled = toggled;
		PlaySound(m_sSoundClicked);
		ColorizeBackground(animate);
		if (invokeChange)
			m_OnToggled.Invoke(this, m_bIsToggled);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsToggled()
	{
		return m_bIsToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetToggleable(bool togglable)
	{
		m_bCanBeToggled = togglable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetToggleable()
	{
		return m_bCanBeToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowBorder(bool show, bool animate = true)
	{		
		if (animate)
			WidgetAnimator.PlayAnimation(m_wBorder, WidgetAnimationType.Opacity, show, m_fAnimationRate);
		else if (m_wBorder)
			m_wBorder.SetOpacity(show);
		
		m_OnShowBorder.Invoke(this, show);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBorderShown()
	{
		if (!m_wBorder)
			return false;
		return m_wBorder.GetOpacity() != 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorizeBackground(bool animate = true)
	{
		if (!m_bUseColorization || !m_wBackground)
			return;
		
		Color color;
		bool isHovered = WidgetManager.GetWidgetUnderCursor() == m_wRoot;
		if (m_bIsToggled && m_bCanBeToggled)
		{
			if (isHovered)
				color = m_BackgroundSelectedHovered;
			else
				color = m_BackgroundSelected;
		}
		else
		{
			if (isHovered)
				color = m_BackgroundHovered;
			else
				color = m_BackgroundDefault;
		}
		
		if (animate)
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, color, m_fAnimationRate);
		else
		{
			WidgetAnimator.StopAnimation(m_wBackground, WidgetAnimationType.Color);
			m_wBackground.SetColor(color);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonBaseComponent GetButtonBase(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonBaseComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_ButtonBaseComponent, name, parent, searchAllChildren)
			);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds the parent menu of this component
	protected void FindParentMenu()
	{
		// Find parent menu
		MenuBase parentMenu = GetGame().GetMenuManager().GetOwnerMenu(m_wRoot);
		m_ParentMenu = parentMenu;
		
		// Find parent submenu
		Widget wParent = m_wRoot.GetParent();
		while (wParent)
		{
			ScriptedWidgetEventHandler subMenuBase = wParent.FindHandler(SCR_SubMenuBase);
			if (subMenuBase)
			{
				m_ParentSubMenu = SCR_SubMenuBase.Cast(subMenuBase);
				break;
			}
			wParent = wParent.GetParent();
		}
		
		// Warning disabled since we have quite many places where a button is not in a menu
		//if (!parentMenu)
		//	Print(string.Format("Failed to find parent menu of SCR_ButtonBaseComponent %1 on widget %2", this, this.GetRootWidget().GetName()), LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if parent menu is focused, or if there is no parent menu.
	protected bool IsParentMenuFocused()
	{
		if (m_ParentSubMenu)
			return m_ParentSubMenu.GetShown();
		else if(m_ParentMenu)
			return m_ParentMenu.IsFocused();
		else
			return true;
	}
};