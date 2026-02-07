//#define DISABLE_HUD

//------------------------------------------------------------------------------------------------
class SCR_InfoDisplayExtended : SCR_InfoDisplay
{
	[Attribute("1", UIWidgets.CheckBox, "Toggles ON/OFF info display.")]
	protected bool m_bIsEnabled;
	
	//------------------------------------------------------------------------------------------------		
	// Interface methods for the extended InfoDisplay class.
	//------------------------------------------------------------------------------------------------		
	bool DisplayStartDrawInit(IEntity owner)
	{
		return true;
	}	

	void DisplayStartDraw(IEntity owner)
	{
	}	
		
	void DisplayStopDraw(IEntity owner)
	{
	}	
	
	void DisplayInit(IEntity owner)
	{
	}	

	void DisplayUpdate(IEntity owner, float timeSlice)
	{
	}	
	
	//------------------------------------------------------------------------------------------------
	// InfoDisplay events blocked for overriding.
	// The interface methods above should be used instead.		
	//------------------------------------------------------------------------------------------------
	private override event void OnStartDraw(IEntity owner)
	{
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
		
		if (!m_bIsEnabled)
			return;
		
		m_bIsEnabled = DisplayStartDrawInit(owner);
		
		if (!m_bIsEnabled)
			return;
		
		super.OnStartDraw(owner);
		
		if (!m_wRoot)
			return;
		
		DisplayStartDraw(owner);
	}
	
	private override event void OnStopDraw(IEntity owner)
	{
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
	
		if (!m_bIsEnabled)
			return;
		
		super.OnStopDraw(owner);
		
		DisplayStopDraw(owner);		
	}	
	
	override event void OnInit(IEntity owner)
	{
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif

		if (!m_bIsEnabled)
			return;

		super.OnInit(owner);
		
		DisplayInit(owner);
	}
	
	private override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bIsEnabled)
			return;
		
		super.UpdateValues(owner, timeSlice);		
		
		DisplayUpdate(owner, timeSlice);
	}	
};