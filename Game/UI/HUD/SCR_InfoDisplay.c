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
	
	// Attributes for dynamic opacity feature
	[Attribute("0", UIWidgets.CheckBox, "Adjusts opacity of the widget based on level of ambient light.")]
	protected bool m_bAdaptiveOpacity;		
	
	protected Widget m_wRoot;
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
				Widget wParentSlot = wParentRoot.FindAnyWidget(m_sParentSlot);
				WorkspaceWidget wWorkspace = GetGame().GetWorkspace();
			
				if (wParentSlot && wWorkspace)
					m_wRoot = wWorkspace.CreateWidgets(m_LayoutPath, wParentSlot);
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
		
		// Adaptive opacity initialization
		if (m_wRoot && m_HUDManager && m_bAdaptiveOpacity)
		{
			m_HUDManager.GetSceneBrightnessChangedInvoker().Insert(UpdateOpacity);
			
			UpdateOpacity(m_HUDManager.GetAdaptiveOpacity(), m_HUDManager.GetSceneBrightness(), m_HUDManager.GetSceneBrightnessRaw())
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateOpacity(float opacity, float sceneBrightness, float sceneBrightnessRaw)
	{
		//PrintFormat("<Adaptive Opacity> %1 | Updating opacity %2 -> %3", this, m_wRoot.GetOpacity(), opacity);
		
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