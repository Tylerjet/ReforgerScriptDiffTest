class SCR_InstantWeatherCheckboxAttributeUIComponent: SCR_ButtonBoxAttributeUIComponent
{
	[Attribute("#AR-Editor_Attribute_AutomaticWeather_Warning")]
	protected LocalizedString m_sAutomatedExplanation;
	
	[Attribute("#AR-Editor_Attribute_DayDuration_Name")]
	protected LocalizedString m_sAutomatedExplanationParam1;
	
	[Attribute("#AR-Editor_Attribute_AdvanceTime_Name")]
	protected LocalizedString m_sAutomatedExplanationParam2;
	
	
	protected bool m_bWeatherIsAutomatic = true;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		super.Init(w, attribute);
		
		//Check if weather is looping
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager)
			return;
		
		WeatherStateTransitionManager weatherTransitionManager =  weatherManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return;
		
		WeatherStateTransitionNode currentStateTransition = weatherTransitionManager.GetCurrentStateTransitionNode();
		if (!currentStateTransition)
			return;
		
		m_bWeatherIsAutomatic = !currentStateTransition.IsLooping();
	}
	
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{		
		super.SetFromVar(var);
							
		if (!var)
			return;
		
		if (!m_bWeatherIsAutomatic && var.GetInt() == 0)
			OverrideDescription(true, m_sAutomatedExplanation, m_sAutomatedExplanationParam1, m_sAutomatedExplanationParam2);
		else 
			OverrideDescription(false);
	}
	
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{
		OverrideDescription(false);
		
		super.SetVariableToDefaultValue(var);
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{		
		if (!m_bWeatherIsAutomatic && x == 0)
			OverrideDescription(true, m_sAutomatedExplanation, m_sAutomatedExplanationParam1, m_sAutomatedExplanationParam2);
		else 
			OverrideDescription(false);
		
		return super.OnChange(w, x, y, finished);;
	}
};