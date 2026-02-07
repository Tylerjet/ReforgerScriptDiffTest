class SCR_SliderBloodAttributeComponent : SCR_SliderWithWarningEditorAttributeUIComponent
{
	[Attribute("BloodUnconsciousBar", desc: "Image that shows when Character becomes conscious")]
	protected string m_sUnconsciousBarName;
	
	[Attribute("BloodUnconsciousBarTop", desc: "Set at the end of the bar")]
	protected string m_sUnconsciousBarTopName;
	
	[Attribute("BloodUnconsciousFillRight", desc: "Filler make sure the bar is set the correct size")]
	protected string m_sUnconsciousFillerRightName;
	
	[Attribute("1", desc: "Size of the BarTop")]
	protected float m_iUnconsciousBarTopSize;
	
	protected SCR_GameModeHealthSettings m_GameModeHealthSettings;
	
	//[Attribute("Character goes Unconcious", desc: "Text shown if character(s) will become unconcious")]
	//protected LocalizedString m_sUnconciousWarning;
	
	protected float m_fUnconsciousLevel;
	protected bool m_bEntityUnconsciousnessPermitted;
	
	protected Widget m_UnconsciousBar;
	protected Widget m_UnconsciousBarTop;
	protected Widget m_UnconsciousFillerRight;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{	
		m_UnconsciousBar = w.FindAnyWidget(m_sUnconsciousBarName);
		m_UnconsciousBarTop = w.FindAnyWidget(m_sUnconsciousBarTopName);
		m_UnconsciousFillerRight = w.FindAnyWidget(m_sUnconsciousFillerRightName);
		
		LayoutSlot.SetFillWeight(m_UnconsciousBarTop, m_iUnconsciousBarTopSize);
		
		if (!m_UnconsciousBar || !m_UnconsciousFillerRight || !m_UnconsciousBarTop)
			return;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		vector bloodData = var.GetVector();
		m_bEntityUnconsciousnessPermitted = bloodData[2] != 0;
		
		m_GameModeHealthSettings = SCR_GameModeHealthSettings.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeHealthSettings));
		
		m_fUnconsciousLevel = bloodData[1] * 100;
		
		super.Init(w, attribute);
	}
	
	
	override void SetSliderSettings(SCR_BaseEditorAttributeVar var, SCR_BaseEditorAttributeEntrySlider sliderData)
	{
		super.SetSliderSettings(var, sliderData);
		UpdateBloodSlider();
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{	
		super.SetVariableToDefaultValue(var);
		UpdateBloodSlider();
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{						
		super.SetFromVar(var);
		UpdateBloodSlider();
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{		
		super.OnChange(w, x, y, finished);
		UpdateBloodSlider();
		return false;
	}
	
	protected void UpdateBloodSlider()
	{
		if (m_SliderWidgetComponent.GetValue() >= m_fUnconsciousLevel)
		{
			m_UnconsciousBarTop.SetVisible(true);
			LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(m_fUnconsciousLevel - m_iUnconsciousBarTopSize, 0, 100));
			LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - m_fUnconsciousLevel, 0, 100));
		}
		else 
		{
			if (m_SliderWidgetComponent.GetValue() >= m_iUnconsciousBarTopSize)
			{
				m_UnconsciousBarTop.SetVisible(true);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(m_SliderWidgetComponent.GetValue() - m_iUnconsciousBarTopSize, 0 , 100));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - m_SliderWidgetComponent.GetValue(), 0, 100));
			}
			else 
			{
				m_UnconsciousBarTop.SetVisible(false);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(m_SliderWidgetComponent.GetValue(), 0 , 100));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - m_SliderWidgetComponent.GetValue(), 0, 100));
			}
		}
	}
};
