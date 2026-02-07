//#define DEBUG_ADAPTIVE_OPACITY
//#define DEBUG_INFO_DISPLAY

enum EWidgetAnchor
{
	TOPLEFT,
	TOP,
	TOPRIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOMLEFT,
	BOTTOM,
	BOTTOMRIGHT
};

//------------------------------------------------------------------------------------------------
class SCR_InfoDisplay : GroupInfoDisplay
{
	// Attributes
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	ResourceName m_LayoutPath;
	[Attribute("2", UIWidgets.ComboBox, "HUD Layer for the UI element to be placed in. Ignored when InfoDisplay is nested under another InfoDisplay.", "", ParamEnumArray.FromEnum(EHudLayers) )]
	EHudLayers m_eLayer;
	[Attribute("0", UIWidgets.EditBox, "Override the hierarchy to show display in front or behind other displays.")]
	int m_iOverrideZOrder;
	[Attribute("", UIWidgets.EditBox, "Name of slot in parent widget, the UI element is going to be placed in. Used when InfoDisplay is nested under another InfoDisplay.")]
	protected string  m_sParentSlot;
	
	// Dimensions and safezone
	//m_WeaponInfoPanel
	[Attribute("", UIWidgets.EditBox, "Name of widget containing the GUI element content. Uses the root widget, if empty.")]
	protected string  m_sContentWidget;	
	[Attribute("0", UIWidgets.Slider, "Adjustment to the content widget width. Can be used to provide a widget-specific padding.", "-200 200 1")]
	protected int m_iContentWidthAdjustment;
	[Attribute("0", UIWidgets.Slider, "Adjustment to the content height width. Can be used to provide a widget-specific padding.", "-200 200 1")]
	protected int m_iContentHeightAdjustment;
		
	// Attributes for dynamic opacity feature
	[Attribute("0", UIWidgets.CheckBox, "Adjusts opacity of the widget based on level of ambient light.")]
	protected bool m_bAdaptiveOpacity;		
	
	protected Widget m_wRoot;
	protected Widget m_wContent;
	protected Widget m_wSlot;
	protected bool m_bShown = false;
	protected bool m_bForceShowState = false;
	protected SCR_HUDManagerComponent m_HUDManager;
	protected int m_iForcedShowBehavior = 0;
	
	protected int m_iChildDisplays = 0;
	protected ref array<BaseInfoDisplay> m_aChildDisplays = new ref array<BaseInfoDisplay>;
	protected SCR_InfoDisplay m_pParentDisplay = null;
	protected bool m_bRegistered = false;
	
	#ifdef WORKBENCH
	protected float m_fAdaptiveOpacityValue = -1;
	#endif
	
	//------------------------------------------------------------------------------------------------
	void RemoveForcedShow(bool removeAll = false)
	{
		if (m_iForcedShowBehavior <= 0)
			return;
		
		if (removeAll)
			m_iForcedShowBehavior = 0;
		else if (m_iForcedShowBehavior > 0)
			m_iForcedShowBehavior--;
	}
	
	//------------------------------------------------------------------------------------------------
	void Show(bool show, float speed = UIConstants.FADE_RATE_DEFAULT, bool force = false)
	{
		if (!m_wRoot)
			return;
		
		if (force)
		{
			m_iForcedShowBehavior++;
		}
		else if (m_iForcedShowBehavior > 0)
		{
			return;
		}
		
		if (show != m_bShown)
		{
			m_bShown = show;
			AnimateWidget.Opacity(m_wRoot, show, speed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bShown;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsForceShown()
	{
		return m_bForceShowState > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible = true)
	{
		if (m_wRoot)
			m_wRoot.SetVisible(visible);
		
		if (m_wSlot)
			m_wSlot.SetVisible(visible);
	}	

	//------------------------------------------------------------------------------------------------
	//! Return width and height of the InfoDisplay element, optionally with safezones adjustments
	bool GetDimensions(out float width, out float height, bool addSafezones = true)
	{
		if (!m_wContent)
		{
			width = 0;
			height = 0;
			return false;
		}
			
		m_wContent.GetScreenSize(width, height);

		WorkspaceWidget workspace = m_wContent.GetWorkspace();
		width = workspace.DPIUnscale(width);
		height = workspace.DPIUnscale(height);		

		if (addSafezones)
		{
			width += m_iContentWidthAdjustment;
			height += m_iContentHeightAdjustment;
		}
		
		return true;
	}		

	//------------------------------------------------------------------------------------------------
	//! Return width and height of the InfoDisplay element, optionally with safezones adjustments
	bool GetAnchorPosition(out float x, out float y, EWidgetAnchor anchor = EWidgetAnchor.TOPLEFT, bool addSafezones = true)
	{
		if (!m_wContent)
		{
			x = 0;
			y = 0;
			return false;
		}
		
		float width, height;
		
		GetDimensions(width, height, addSafezones);
		
		m_wContent.GetScreenPos(x, y);
		
		WorkspaceWidget workspace = m_wContent.GetWorkspace();
		x = workspace.DPIUnscale(x) - m_iContentWidthAdjustment * 0.5 * addSafezones;
		y = workspace.DPIUnscale(y) - m_iContentHeightAdjustment * 0.5 * addSafezones;			

	    switch(anchor)
	    {
	        case EWidgetAnchor.TOPLEFT:

	            break;
	  
	        case EWidgetAnchor.TOP:

				x += width / 2;
	            break;
	  
	        case EWidgetAnchor.TOPRIGHT:
	            
				x += width;
	            break;
			
	        case EWidgetAnchor.LEFT:

				y += height / 2;
				break;
	  
	        case EWidgetAnchor.CENTER:

				y += height / 2;
				x += width / 2;
	            break;
	  
	        case EWidgetAnchor.RIGHT:
	            
				y += height / 2;
				x += width;
	            break;
			
	        case EWidgetAnchor.BOTTOMLEFT:

				y += height;			
	            break;
	  
	        case EWidgetAnchor.BOTTOM:

				y += height;
				x += width / 2;
	            break;
	  
	        case EWidgetAnchor.BOTTOMRIGHT:
	            
				y += height;
				x += width;
	            break;
			
			default:
				
				break;									
	    }	
		
		return true;	
	}		
			
	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		if (m_wRoot)
			return;
		
		// Nested placement; used when parent InfoDisplay is properly defined		
		if (m_pParentDisplay)
		{
			Widget wParentRoot = m_pParentDisplay.m_wRoot;
			
			if (wParentRoot)
			{
				m_wSlot = wParentRoot.FindAnyWidget(m_sParentSlot);
				WorkspaceWidget wWorkspace = GetGame().GetWorkspace();
			
				if (m_wSlot && wWorkspace)
					m_wRoot = wWorkspace.CreateWidgets(m_LayoutPath, m_wSlot);
			}
		}
		
		// Default placement; used when there is no parent InfoDisplay or it's slot cannot be detected		
		if (!m_wRoot)
		{
			m_pParentDisplay = null;
			m_sParentSlot = "";
			
			if (!m_HUDManager)
				m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();
			
			if (!m_HUDManager)
				return;
	
			m_bShown = false;
			m_HUDManager.RegisterHUDElement(this);
			m_bRegistered = true;
			m_wRoot = m_HUDManager.CreateLayout(m_LayoutPath, m_eLayer, m_iOverrideZOrder);
		}

		// Detect 'content widget'
		if (m_sContentWidget != string.Empty)
			m_wContent = m_wRoot.FindAnyWidget(m_sContentWidget);
		
		if (!m_wContent)
			m_wContent = m_wRoot;		
				
		// Adaptive opacity initialization
		if (m_wRoot && m_HUDManager && m_bAdaptiveOpacity)
		{
			m_HUDManager.GetSceneBrightnessChangedInvoker().Insert(UpdateOpacity);
			
			UpdateOpacity(m_HUDManager.GetAdaptiveOpacity(), m_HUDManager.GetSceneBrightness(), m_HUDManager.GetSceneBrightnessRaw())
		}
		
		#ifdef DEBUG_INFO_DISPLAY
		PrintFormat("%1 [OnStartDraw] m_wRoot: %2", this, m_wRoot);
		#endif		
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateOpacity(float opacity, float sceneBrightness, float sceneBrightnessRaw)
	{
		#ifdef DEBUG_ADAPTIVE_OPACITY
		PrintFormat("%1 [UpdateOpacity] %2 -> %3", this, m_wRoot.GetOpacity(), opacity);
		#endif
		
		#ifdef WORKBENCH
		float currentOpacity = m_wRoot.GetOpacity();
		
		if (m_fAdaptiveOpacityValue != -1 && m_fAdaptiveOpacityValue != currentOpacity)
			Print(string.Format("%1 adaptive opacity value was overriden from %2 to %3 by some ill means!", this, m_fAdaptiveOpacityValue, currentOpacity), LogLevel.WARNING);
		
		m_fAdaptiveOpacityValue = opacity;
		#endif
		
		m_wRoot.SetOpacity(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
		// Adaptive opacity initialization
		if (m_wRoot && m_HUDManager && m_bAdaptiveOpacity)
			m_HUDManager.GetSceneBrightnessChangedInvoker().Remove(UpdateOpacity);
		
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();

		if (m_HUDManager && m_bRegistered)
			m_HUDManager.UnregisterHUDElement(this);
		
		#ifdef DEBUG_INFO_DISPLAY
		PrintFormat("%1 [OnStopDraw] m_wRoot: %2", this, m_wRoot);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
	}	
	
	//------------------------------------------------------------------------------------------------
	override event void OnInit(IEntity owner)
	{
		// Get slotted children info
		m_iChildDisplays = GetInfoDisplays(m_aChildDisplays);
		
		foreach (BaseInfoDisplay pDisplay : m_aChildDisplays)
		{
			SCR_InfoDisplay pInfoDisplay = SCR_InfoDisplay.Cast(pDisplay);
			
			if (pInfoDisplay)
				pInfoDisplay.m_pParentDisplay = this;
		}
	}	
};