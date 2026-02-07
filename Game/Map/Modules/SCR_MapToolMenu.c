//------------------------------------------------------------------------------------------------
class SCR_MapToolEntry : Managed
{	
	protected bool m_bVisible;
	protected int m_iSortPriority;
	ResourceName m_sImageSet;
	string m_sIconQuad;
	SCR_ButtonImageComponent m_ButtonComp;
	
	ref ScriptInvoker m_OnClick = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! OnClick event
	protected void HandleClick()
	{
		m_bVisible = !m_bVisible;
		if (m_bVisible) 
			SetColor(UIColors.CONTRAST_COLOR);
		else 
			SetColor(Color.Gray25);
		
		GetGame().GetWorkspace().SetFocusedWidget(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry image color
	//! \param color is target color
	void SetColor(Color color)
	{
		m_ButtonComp.m_wImage.SetColor(color);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapToolEntry(ResourceName imageset, string icon, bool defaultVisible, int sortPriority)
	{
		m_sImageSet = imageset;
		m_sIconQuad = icon;
		
		m_bVisible = defaultVisible;
		if (m_bVisible) 
			SetColor(UIColors.CONTRAST_COLOR);
		
		m_iSortPriority = sortPriority;
		
		m_OnClick.Insert(HandleClick);
	}
}

//------------------------------------------------------------------------------------------------
//!
[BaseContainerProps()]
class SCR_MapToolMenuModule : SCR_MapModuleBase
{
	const ResourceName ICONS_IMAGESET = "{12BA7AE37121F768}UI/Textures/Icons/icons_wrapperUI-96.imageset";
	const ResourceName BUTTON_LAYOUT = "{9F48F5037C02D961}UI/layouts/Map/MapToolMenuButton.layout";
	const string TOOL_MENU_ROOT = "ToolMenu";
	const string TOOL_MENU_BAR = "ToolMenuHoriz";
	
	protected bool m_bIsVisible;
	protected Widget m_wToolMenuRoot;
	protected Widget m_wToolMenuBar;
	protected ref array<ref SCR_MapToolEntry> m_aMenuEntries = {};
	
	//------------------------------------------------------------------------------------------------
	//! Register menu entry
	//! \param imageset is source imageset
	//! \param icon is quad from the provided imageset
	//! \param defaultVisible determines the initial state of the icon
	//! \param sortPriority is disply priority of the icon within the menu, lower value means higher priority
	SCR_MapToolEntry RegisterToolMenuEntry(ResourceName imageset, string icon, bool defaultVisible, int sortPriority)
	{				
		SCR_MapToolEntry entry = new SCR_MapToolEntry(imageset, icon, defaultVisible, sortPriority);
		m_aMenuEntries.Insert(entry);

		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Build entries
	protected void PopulateToolMenu()
	{
		Widget button;
		SCR_ButtonImageComponent buttonComp;
		
		foreach (SCR_MapToolEntry entry : m_aMenuEntries)
		{
			button = GetGame().GetWorkspace().CreateWidgets(BUTTON_LAYOUT, m_wToolMenuBar);
		
			buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			if (buttonComp)
			{
				buttonComp.m_OnClicked = entry.m_OnClick;
				buttonComp.SetImage(entry.m_sImageSet, entry.m_sIconQuad);	
							
				entry.m_ButtonComp = buttonComp;
			}
			
		}
		
		SetUIVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Focus menu event when using controller
	protected void OnFocusToolMenu()
	{
		if (m_aMenuEntries.IsEmpty())
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aMenuEntries[0].m_ButtonComp.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set tool menu visibility
	//! \param state is target visibility
	protected void SetUIVisible(bool state)
	{
		m_bIsVisible = state;
		m_wToolMenuRoot.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		m_wToolMenuRoot = m_MapEntity.GetMapMenuRoot().FindAnyWidget(TOOL_MENU_ROOT);
		if (m_wToolMenuRoot)
			m_wToolMenuBar = m_wToolMenuRoot.FindAnyWidget(TOOL_MENU_BAR);
		
		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
			inputMgr.AddActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
		
		SetUIVisible(false);
		PopulateToolMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
			inputMgr.RemoveActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
	}
}
