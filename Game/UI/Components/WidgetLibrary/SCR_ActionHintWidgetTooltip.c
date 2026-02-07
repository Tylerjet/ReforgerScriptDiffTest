//------------------------------------------------------------------------------------------------
class SCR_ActionHintWidgetTooltip : SCR_ScriptedWidgetTooltip
{
	protected SCR_ActionHintWidgetTooltipPreset m_ActionPreset;
	protected SCR_InputButtonComponent m_ActionDisplay;

	protected const string WIDGET_ACTION = "Action";

	//------------------------------------------------------------------------------------------------
	override void InitContents()
	{
		super.InitContents();

		m_ActionPreset = SCR_ActionHintWidgetTooltipPreset.Cast(m_Preset);
		if (!m_ActionPreset)
			return;

		Widget action = m_wTooltipProxy.FindAnyWidget(WIDGET_ACTION);
		if (action)
			m_ActionDisplay = SCR_InputButtonComponent.FindComponent(action);

		if (!m_ActionDisplay)
			return;

		SetAction(m_ActionPreset.m_sAction);
		SetActionColor(m_ActionPreset.m_ActionColor);
	}

	//------------------------------------------------------------------------------------------------
	bool SetAction(string action)
	{
		if (!m_ActionDisplay || action.IsEmpty())
			return false;

		m_ActionDisplay.SetAction(action);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SetActionColor(Color color)
	{
		if (!m_ActionDisplay)
			return false;

		m_ActionDisplay.SetColorActionDisabled(color);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool ResetAction()
	{
		return SetAction(GetDefaultAction());
	}

	//------------------------------------------------------------------------------------------------
	bool ResetActionColor()
	{
		return SetActionColor(GetDefaultActionColor());
	}

	//------------------------------------------------------------------------------------------------
	string GetDefaultAction()
	{
		if (!m_ActionPreset)
			return string.Empty;

		return m_ActionPreset.m_sDefaultAction;
	}

	//------------------------------------------------------------------------------------------------
	Color GetDefaultActionColor()
	{
		if (!m_ActionPreset)
			return Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());

		return Color.FromInt(m_ActionPreset.m_DefaultActionColor.PackToInt());
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_ActionHintWidgetTooltipPreset : SCR_ScriptedWidgetTooltipPreset
{
	[Attribute("", desc: "action to display")]
	string m_sAction;

	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_ACTIVE))]
	ref Color m_ActionColor;

	string m_sDefaultAction;
	Color m_DefaultActionColor;

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		m_sDefaultAction = m_sAction;
		m_DefaultActionColor = m_ActionColor;
	}
}
