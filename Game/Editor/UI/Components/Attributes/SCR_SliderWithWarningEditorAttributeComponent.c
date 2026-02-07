class SCR_SliderWithWarningEditorAttributeUIComponent: SCR_SliderEditorAttributeUIComponent
{	
	[Attribute("1", UIWidgets.ComboBox, "Show warning when", "", ParamEnumArray.FromEnum(ESliderValue) )]
	protected ESliderValue m_iSliderWarningWhen;
	
	[Attribute("0")]
	protected float m_fStartWarningValue;
	
	[Attribute("1", desc: "If false it will only show warning if the condition was false before and is now true (meaning if you save the attributes and open it again it will not show the warning), else it will always set the warning if the condition is true")]
	protected bool m_bShowWarningAlways;
	
	[Attribute()]
	protected LocalizedString m_sWarningNote;
	
	protected float m_bStartingValue;
	
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{	
		if (!var)
			return;
		
		m_bStartingValue = var.GetFloat();
		super.SetFromVar(var);
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{		
		float value = m_SliderWidgetComponent.GetValue();
		
		if (m_iSliderWarningWhen == ESliderValue.LESS_THAN)
		{	
			if (m_bShowWarningAlways)
				OverrideDescription(value < m_fStartWarningValue, m_sWarningNote);
			else 
				OverrideDescription((m_bStartingValue >= m_fStartWarningValue && value < m_fStartWarningValue), m_sWarningNote);
		}
		else if (m_iSliderWarningWhen == ESliderValue.LESS_OR_EQUAL_THAN)
		{
			if (m_bShowWarningAlways)
				OverrideDescription(value <= m_fStartWarningValue, m_sWarningNote);
			else 
				OverrideDescription((m_bStartingValue > m_fStartWarningValue && value <= m_fStartWarningValue), m_sWarningNote);
		}
		else if (m_iSliderWarningWhen == ESliderValue.EQUAL_TO)
		{
			if (m_bShowWarningAlways)
				OverrideDescription(value == m_fStartWarningValue, m_sWarningNote);
			else 
				OverrideDescription((value != m_bStartingValue && value == m_fStartWarningValue), m_sWarningNote);
		}
		else if (m_iSliderWarningWhen == ESliderValue.GREATER_THAN)
		{
			if (m_bShowWarningAlways)
				OverrideDescription(value > m_fStartWarningValue, m_sWarningNote);
			else 
				OverrideDescription((m_bStartingValue <= m_fStartWarningValue && value > m_fStartWarningValue), m_sWarningNote);
		}
		else if (m_iSliderWarningWhen == ESliderValue.GREATER_OR_EQUAL_THAN)
		{
			if (m_bShowWarningAlways)
				OverrideDescription(value >= m_fStartWarningValue, m_sWarningNote);
			else 
				OverrideDescription((m_bStartingValue < m_fStartWarningValue && value >= m_fStartWarningValue), m_sWarningNote);
		}
		
		return super.OnChange(w, x, y, finished);
	}
};

enum ESliderValue
{
	LESS_THAN = 0,
	LESS_OR_EQUAL_THAN = 1,
	EQUAL_TO = 2,
	GREATER_THAN = 3,
	GREATER_OR_EQUAL_THAN = 4
};