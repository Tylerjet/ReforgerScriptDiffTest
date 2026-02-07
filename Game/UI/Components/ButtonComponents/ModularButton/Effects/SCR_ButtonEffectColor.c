/*
Effect which colorizes a widget with given name.
*/

[BaseContainerProps(), SCR_ButtonEffectTitleAttribute("Color", "m_sWidgetName")]
class SCR_ButtonEffectColor : SCR_ButtonEffectWidgetBase
{
	// Attributes
	
	[Attribute(defvalue: "0.2", UIWidgets.EditBox, "How fast each animation proceeds")]
	protected float m_fAnimationTime;
	
	[Attribute()]
	ref Color m_cDefault;
	
	[Attribute()]
	ref Color m_cHovered;
	
	[Attribute()]
	ref Color m_cActivated;
	
	[Attribute()]
	ref Color m_cActivatedHovered;
	
	[Attribute()]
	ref Color m_cDisabled;
	
	[Attribute()]
	ref Color m_cDisabledActivated;
	
	[Attribute()]
	ref Color m_cClicked;
	
	[Attribute()]
	ref Color m_cFocusGained;
	
	[Attribute()]
	ref Color m_cFocusLost;
	
	[Attribute()]
	ref Color m_cToggledOn;
	
	[Attribute()]
	ref Color m_cToggledOff;

	
	override void OnStateDefault(bool instant)
	{
		Apply(m_cDefault, instant);		
	}
	
	override void OnStateHovered(bool instant)
	{
		Apply(m_cHovered, instant);
	}
	
	override void OnStateActivated(bool instant)
	{
		Apply(m_cActivated, instant);
	}
	
	override void OnStateDisabled(bool instant)
	{
		Apply(m_cDisabled, instant);
	}
	
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_cDisabledActivated, instant);
	}
	
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_cActivatedHovered, instant);
	}
	
	override void OnClicked(bool instant)
	{
		Apply(m_cClicked, instant);
	}
	
	override void OnFocusGained(bool instant)
	{
		Apply(m_cFocusGained, instant);
	}	
	
	override void OnFocusLost(bool instant)
	{
		Apply(m_cFocusLost, instant);
	}
	
	override void OnToggledOn(bool instant)
	{
		Apply(m_cToggledOn, instant);
	}
	
	override void OnToggledOff(bool instant)
	{
		Apply(m_cToggledOff, instant);
	}
	
	// Called when effect is disabled. Here you should stop all running effects.
	override void OnDisabled()
	{
		AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationColor);
	}
	
	protected void Apply(Color color, bool instant)
	{
		if (color && m_wTarget)
		{
			if (!instant && m_fAnimationTime != 0)
			{
				AnimateWidget.Color(m_wTarget, color, 1 / m_fAnimationTime);
			}
			else
			{
				AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationColor);
				m_wTarget.SetColor(color);
			}
		}
	}
};
