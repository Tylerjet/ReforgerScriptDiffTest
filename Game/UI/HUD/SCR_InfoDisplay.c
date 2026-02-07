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
	
	protected Widget m_wRoot;
	protected bool m_bShown = false;
	protected bool m_bForceShowState = false;
	protected SCR_HUDManagerComponent m_HUDManager;
	protected int m_iForcedShowBehavior = 0;
	
	protected int m_iChildDisplays = 0;
	protected ref array<BaseInfoDisplay> m_aChildDisplays = new ref array<BaseInfoDisplay>;
	protected SCR_InfoDisplay m_pParentDisplay = null;
	protected bool m_bRegistered = false;
	
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
	void Show(bool show, float speed = WidgetAnimator.FADE_RATE_DEFAULT, bool force = false)
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
			WidgetAnimator.PlayAnimation(m_wRoot,WidgetAnimationType.Opacity,show,speed);
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
			
			m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();
			if (!m_HUDManager)
				return;
	
			m_bShown = false;
			m_HUDManager.RegisterHUDElement(this);
			m_bRegistered = true;
			m_wRoot = m_HUDManager.CreateLayout(m_LayoutPath, m_eLayer, m_iOverrideZOrder);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
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