// Placeholder component for highlighting highlighted elements, used in settingsMenu

class ElementHighlightComponent : ScriptedWidgetComponent
{
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	private ref Color m_ColorDefault;
	
	[Attribute("0.85 0.7 0 1", UIWidgets.ColorPicker, "")]
	private ref Color m_ColorFocused;
	
	[Attribute(UISounds.FOCUS, UIWidgets.EditBox, "")]
	protected string m_sSoundFocused;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		w.SetColor(m_ColorDefault);
		
		if (!w.IsEnabled())
			w.SetOpacity(0.5);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundFocused);
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Color, m_ColorFocused, WidgetAnimator.FADE_RATE_DEFAULT);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Color, m_ColorDefault, WidgetAnimator.FADE_RATE_DEFAULT);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		return false;
	}
};