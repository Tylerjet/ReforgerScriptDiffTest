//#define DEBUG_VEHICLE_UI

enum EVehicleInfoState
{
	DEFAULT,
	WARNING,
	ERROR
};

class SCR_BaseVehicleInfo: SCR_InfoDisplayExtended
{	
	protected ResourceName m_Imageset = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	protected ResourceName m_ImagesetGlow = "{ABC6B36856013403}UI/Textures/Icons/icons_wrapperUI-64-glow.imageset";

	protected ImageWidget m_wIcon;
	protected ImageWidget m_wGlow;
	
	[Attribute("", UIWidgets.EditBox, "Indicator icon to be displayed.")]
	protected string  m_sIcon;
	
	[Attribute("0.8", UIWidgets.Slider, "Indicator scale", "0.25 2 0.05")]
	protected float m_fWidgetScale;	

	[Attribute("1", UIWidgets.CheckBox, "Show the greyed-out indicator, if it is inactive, otherwise it will be hidden.")]
	protected bool m_bShowGhost;
		
	protected bool m_bState; //TODO: Redo to use EVehicleInfoState instead
	
	const float OPACITY_FADED = 0.3;
	const float OPACITY_DEFAULT = 1;
	
	const ref Color COLOR_DEFAULT = Color.FromSRGBA(200, 200, 200, 200);				//WHITE with 30% alpha converted to GREY with 100% alpha
	const ref Color COLOR_DEFAULT_GLOW = Color.FromSRGBA(0, 0, 0, 100);

	const ref Color COLOR_WARNING = Color.FromSRGBA(226, 167, 80, 255);			//ORANGE (standard UI orange)
	const ref Color COLOR_WARNING_GLOW = Color.FromSRGBA(162, 97, 0, 255);		//DARK ORANGE		
		
	//------------------------------------------------------------------------------------------------
	//! Turn the icon to active state, set the opacity
	void SetState(bool state)
	{
		#ifdef DEBUG_VEHICLE_UI
		PrintFormat("%1 SetState: %2", this, state);
		#endif
		
		m_bState = state;
		
		if (!m_wIcon)
			return;
		
		if (m_bState)
		{
			m_wIcon.SetColor(COLOR_WARNING);
			m_wGlow.SetColor(COLOR_WARNING_GLOW);
		}
		else
		{
			m_wIcon.SetColor(COLOR_DEFAULT);
			m_wGlow.SetColor(COLOR_DEFAULT_GLOW);			
			
			m_wRoot.SetVisible(m_bShowGhost);
			m_bShowLocal = m_bShowGhost;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	protected bool GetState()
	{
		return m_bState;
	}

	//------------------------------------------------------------------------------------------------
	private void Scale(ImageWidget widget, float scale)
	{
		if (!widget)
			return;
		
		int imageWidth = 0;
		int imageHeight = 0;
		int image = widget.GetImage();
		
		widget.GetImageSize(image, imageWidth, imageHeight);
		widget.SetSize((float)imageWidth * scale, (float)imageHeight * scale);		
	}	

	//------------------------------------------------------------------------------------------------	
	private void Scale(TextWidget widget, float scale)
	{
		if (!widget)
			return;

		float sizeY = FrameSlot.GetSizeY(widget);
		
		FrameSlot.SetSizeY(widget, sizeY * scale);
	}	
		
	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;		
		
		bool state = GetState();
		if (m_bState != state)
			SetState(state);
	}	

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{				
		// Terminate if widget already exists
		if (m_wRoot)
			return false;

		// Fallback to avoid the need to fill-in always the same layout filename
		if (m_LayoutPath == "")
			m_LayoutPath = "{D2E54F91C85CAB6C}UI/layouts/HUD/VehicleInfo/VehicleInfoIcon.layout";			
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create the UI
	override void DisplayStartDraw(IEntity owner)
	{				
		if (!m_wRoot)
			return;

		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		m_wGlow = ImageWidget.Cast(m_wRoot.FindAnyWidget("Glow"));
		
		if (!m_wIcon || !m_wGlow)
			return;
		
		m_wIcon.LoadImageFromSet(0, m_Imageset, m_sIcon);
		m_wGlow.LoadImageFromSet(0, m_ImagesetGlow, m_sIcon);
		
		Scale(m_wIcon, m_fWidgetScale);
		Scale(m_wGlow, m_fWidgetScale);
		
		SetState(GetState());
	}	
		
	//------------------------------------------------------------------------------------------------
	//! Destroy the UI
	override void DisplayStopDraw(IEntity owner)
	{
	}
		
	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}
};