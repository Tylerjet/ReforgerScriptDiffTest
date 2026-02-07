//#define DEBUG_STATS_PANELS

enum EStatsPanelState
{
	INIT,
	DEFAULT,
	WARNING,
	ERROR
};

class SCR_StatsPanelBase : SCR_InfoDisplayExtended
{
	[Attribute("#AR-ValueUnit_Milliseconds", UIWidgets.EditBox, "Pattern defining how the value and units are displayed (e.g. '%1 ms'.")]
	protected string m_sFormattingPattern;

	[Attribute("50", UIWidgets.SpinBox, "Threshold value for 'warning visual' style. Set it to same or higher value than 'm_iValueError' to disable 'warning visual' style.")]
	protected int m_iValueWarning;	

	[Attribute("100", UIWidgets.SpinBox, "Threshold value for 'error visual' style.")]
	protected int m_iValueError;
	
	[Attribute("999", UIWidgets.SpinBox, "Max. value that can be displayed in the widget. Used to cap the value and also to reserve the space for the value text, to prevent visual glitches.")]
	protected int m_iValueMax;	

	[Attribute("ping-high", UIWidgets.EditBox, "OK icon, used when value is bellow 'm_iValueWarning' threshold.")]
	protected string m_sIconOK;	
	
	[Attribute("ping-high", UIWidgets.EditBox, "Warning icon, used when value is above 'm_iValueWarning', but under 'm_iValueError' threshold.")]
	protected string m_sIconWarning;		

	[Attribute("ping-high", UIWidgets.EditBox, "Error icon, used when value is above 'm_iValueError' threshold.")]
	protected string m_sIconError;
		
	[Attribute("1", UIWidgets.SpinBox, "How often the display is updated (in seconds).", "1 10")]
	protected int m_iUpdateInterval;	

	protected string m_Imageset = "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	protected string m_ImagesetGlow = "{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset";
	
	RplIdentity m_RplIdentity;
	ref SCR_StatsPanelWidgets m_Widgets;
	protected EStatsPanelState m_eState = EStatsPanelState.INIT;
	
	protected bool m_bShowInDefaultState;			// Based on SCR_InfoDisplay 'm_bShowWhenCreated' flag

	protected int m_iSecondsElapsed;
	protected float m_fTimeElapsed;
	
	protected float m_fValueAvg;
	protected float m_fValueAggregated;
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		if (m_iUpdateInterval <= 0)
			return false;
		
		#ifndef DEBUG_STATS_PANELS
		if (!Replication.IsRunning())
			return false;
		#endif
		
		m_RplIdentity = RplIdentity.Local();
		
		return true;
	}	
		
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_Widgets = new SCR_StatsPanelWidgets();
		m_Widgets.Init(m_wRoot);
		m_Widgets.m_wTextPlaceholder.SetText(WidgetManager.Translate(m_sFormattingPattern, m_iValueMax));
		
		m_bShowInDefaultState = m_bShown;
		
		float value = GetValue();
		Update(value);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;
		
		m_fTimeElapsed += timeSlice;
		
		if (m_fTimeElapsed < 1)
			return;

		m_fTimeElapsed = m_fTimeElapsed - 1;
		m_iSecondsElapsed++;

		float value = GetValue();
		
		m_fValueAggregated = m_fValueAggregated + value;
		
		if (m_iSecondsElapsed < m_iUpdateInterval)
			return;
		
		m_iSecondsElapsed = 0;
	
		// Calculate avg stat value
		m_fValueAvg = m_fValueAggregated/m_iUpdateInterval;
		m_fValueAggregated = 0;
		
		Update(m_fValueAvg);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected float GetValue()
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Update(float value)
	{
		#ifdef DEBUG_STATS_PANELS
		if (value == 0)
			value = Math.RandomFloat(0.01,0.1);
		
		value = value * 1000;
		#endif
		
		value = Math.Clamp(Math.Round(value), 0, m_iValueMax);		

		// Update text
		string text = WidgetManager.Translate(m_sFormattingPattern, value);		
		m_Widgets.m_wText.SetText(text);		
			
		// Evaluate the state based on current value	
		EStatsPanelState state = EStatsPanelState.DEFAULT;
		
		if (value > m_iValueError)
			state = EStatsPanelState.ERROR; 
		else if (value > m_iValueWarning)
			state = EStatsPanelState.WARNING;
		
		// State didn't change, no need to update visuals (text already updated)
		if (m_eState == state)
			return;

		m_eState = state;
		
		// Get visialization attributes based on the new state
		float opacity;
		Color color;
		Color colorGlow;
		string icon;		
				
		switch (state)
		{
			case EStatsPanelState.WARNING:
				opacity = 1;	
				color = GUIColors.ORANGE_BRIGHT2;
				colorGlow = GUIColors.ORANGE;
				icon	 = m_sIconWarning;
			break;
			
			case EStatsPanelState.ERROR:
				opacity = 1;	
				color = GUIColors.RED_BRIGHT2;
				colorGlow = GUIColors.RED;
				icon	 = m_sIconError;
			break;			
			
			default:
				opacity = 0.5;	
				color = GUIColors.DEFAULT;
				colorGlow = GUIColors.DEFAULT_GLOW;
				icon	 = m_sIconOK;
			break;
		}		
		
		// Set global color & opacity
		m_Widgets.m_wColorOpacity.SetOpacity(opacity);
		m_Widgets.m_wColorOpacity.SetColor(color);

		// Set icon & glow		
		m_Widgets.m_wIcon.LoadImageFromSet(0, m_Imageset, icon);
		m_Widgets.m_wIconGlow.LoadImageFromSet(0, m_ImagesetGlow, icon);
		m_Widgets.m_wIconGlow.SetColor(colorGlow);
		
		// Update text shadow
		m_Widgets.m_wText.SetShadow(20, colorGlow.PackToInt(), 0.35); 
		
		// Set visibility
		Show((state == EStatsPanelState.DEFAULT && m_bShowInDefaultState) || state != EStatsPanelState.DEFAULT);
	}	
}