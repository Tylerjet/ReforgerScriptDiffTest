//#define DEBUG_VEHICLE_UI

enum EVehicleInfoState
{
	DISABLED,
	ENABLED,
	WARNING,
	ERROR
};

class SCR_BaseVehicleInfo : SCR_InfoDisplayExtended
{
	protected ResourceName m_Imageset = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	protected ResourceName m_ImagesetGlow = "{ABC6B36856013403}UI/Textures/Icons/icons_wrapperUI-64-glow.imageset";

	protected ImageWidget m_wIcon;
	protected ImageWidget m_wGlow;

	[Attribute("", UIWidgets.EditBox, "Indicator icon to be displayed.")]
	protected string m_sIcon;

	[Attribute("0.8", UIWidgets.Slider, "Indicator scale", "0.25 2 0.05")]
	protected float m_fWidgetScale;

	[Attribute("1", UIWidgets.CheckBox, "Show the greyed-out indicator, if it is inactive, otherwise it will be hidden.")]
	protected bool m_bShowGhost;

	protected EVehicleInfoState m_eState = EVehicleInfoState.ERROR;
	protected bool m_bIsBlinking;
	protected int m_iBlinkingOffset;

	const float OPACITY_FADED = 0.3;
	const float OPACITY_DEFAULT = 1;

	const ref Color COLOR_DISABLED = Color.FromSRGBA(200, 200, 200, 100);				//WHITE with 30% alpha converted to GREY with 100% alpha
	const ref Color COLOR_DISABLED_GLOW = Color.FromSRGBA(0, 0, 0, 100);

	const ref Color COLOR_ENABLED = Color.FromSRGBA(255, 255, 255, 255);				//WHITE
	const ref Color COLOR_ENABLED_GLOW = Color.FromSRGBA(162, 162, 162, 255);

	const ref Color COLOR_WARNING = Color.FromSRGBA(226, 167, 80, 255);			//ORANGE (standard UI orange)
	const ref Color COLOR_WARNING_GLOW = Color.FromSRGBA(162, 97, 0, 255);		//DARK ORANGE

	const ref Color COLOR_ERROR = Color.FromSRGBA(226, 80, 80, 255);			//RED
	const ref Color COLOR_ERROR_GLOW = Color.FromSRGBA(162, 0, 0, 255);		//DARK RED

	//------------------------------------------------------------------------------------------------
	//! Turn the icon to active state, set the opacity
	void SetState(EVehicleInfoState state)
	{
		#ifdef DEBUG_VEHICLE_UI
		PrintFormat("%1 SetState: %2", this, state);
		#endif

		if (!m_wIcon)
			return;

		if (!m_wGlow)
			return;

		m_eState = state;

		if (state == EVehicleInfoState.DISABLED)
		{
			m_wIcon.SetColor(COLOR_DISABLED);
			m_wGlow.SetColor(COLOR_DISABLED_GLOW);
		}
		else if (state == EVehicleInfoState.ENABLED)
		{
			m_wIcon.SetColor(COLOR_ENABLED);
			m_wGlow.SetColor(COLOR_ENABLED_GLOW);
		}
		else if (state == EVehicleInfoState.WARNING)
		{
			m_wIcon.SetColor(COLOR_WARNING);
			m_wGlow.SetColor(COLOR_WARNING_GLOW);
		}
		else if (state == EVehicleInfoState.ERROR)
		{
			m_wIcon.SetColor(COLOR_ERROR);
			m_wGlow.SetColor(COLOR_ERROR_GLOW);
		}

		if (!m_bShowGhost)
		{
			m_bShowLocal = state != EVehicleInfoState.DISABLED;
			SetVisible(state != EVehicleInfoState.DISABLED);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	protected EVehicleInfoState GetState()
	{
		return m_eState;
	}

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	protected bool IsBlinking()
	{
		return m_bIsBlinking;
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

		EVehicleInfoState state = GetState();
		int time = GetGame().GetWorld().GetWorldTime();
		if (IsBlinking())
		{
			if (Math.Mod(time - m_iBlinkingOffset, 1000) < 500)
				state = Math.Max(state - 1, EVehicleInfoState.DISABLED);
		}
		else
		{
			m_iBlinkingOffset = Math.Mod(time, 1000);
		}

		if (m_eState != state)
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
