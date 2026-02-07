//! Map tool menu entry data class
class SCR_MapToolEntry : Managed
{	
	protected bool m_bToolActive;
	protected bool m_bIsEnabled = true;
	protected bool m_bIsVisible = true;
	
	bool m_bButtonSoundsDisabled;
	int m_iSortPriority;
	ResourceName m_sImageSet;
	string m_sIconQuad;
	SCR_ToolMenuButtonComponent m_ButtonComp;
	SCR_MapToolMenuUI m_OwnerMenu;
	
	ref ScriptInvoker m_OnClick = new ScriptInvoker();
	static protected ref ScriptInvoker<SCR_MapToolEntry> s_OnEntryToggled = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnEntryToggledInvoker()
	{
		return s_OnEntryToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activation behavior, ON/OFF if entry is active (visual distinction)
	void SetActive(bool toolActive)
	{
		m_bToolActive = toolActive;
		UpdateVisual();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsEntryActive()
	{
		return m_bToolActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disabled behavior, ON/OFF determines whether can be activated (clicked)
	void SetEnabled(bool isEnabled)
	{
		m_bIsEnabled = isEnabled;
		UpdateVisual();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Whether entry can be activated
	bool IsEntryEnabled()
	{
		return m_bIsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Visibility, determines whether the entry is visible with the menu
	void SetVisible(bool state)
	{
		m_bIsVisible = state;
		UpdateVisual();
	}
		
	//------------------------------------------------------------------------------------------------
	//! Enable button sounds
	void SetButtonSoundsDisabled(bool state)
	{
		m_bButtonSoundsDisabled = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update visual based on current state
	void UpdateVisual()
	{
		if (!m_ButtonComp)	// handler might not be updated yet when the set is attempted
			return;
		
		if (m_bToolActive) 
			SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
		else 
			SetColor(Color.FromInt(UIColors.DARK_GREY.PackToInt()));
		
		if (m_bIsEnabled)
			SetBorderColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
		else 
			SetBorderColor(Color.FromInt(UIColors.DARK_GREY.PackToInt()));
		
		m_ButtonComp.GetRootWidget().SetVisible(m_bIsVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry image color
	//! \param[in] color is target color
	protected void SetColor(Color color)
	{
		m_ButtonComp.m_wImage.SetColor(color);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set border color
	//! \param[in] color is target color
	protected void SetBorderColor(Color color)
	{
		m_ButtonComp.m_wBorder.SetColor(color);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetImageSet()
	{
		return m_sIconQuad;
	}
	
	//------------------------------------------------------------------------------------------------
	//! On click callback
	protected void OnClick()
	{
		if (!m_bButtonSoundsDisabled)
		{
			if (m_bToolActive)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_HIDE);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_SHOW);
		}
			
		s_OnEntryToggled.Invoke(this);
		
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			m_OwnerMenu.SetToolMenuFocused(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] menu
	//! \param[in] imageset
	//! \param[in] icon
	//! \param[in] sortPriority
	void SCR_MapToolEntry(SCR_MapToolMenuUI menu, ResourceName imageset, string icon, int sortPriority = 0)
	{
		m_OwnerMenu = menu;
		m_sImageSet = imageset;
		m_sIconQuad = icon;
		
		if (sortPriority > 0)
			m_iSortPriority = sortPriority;
		else
			m_iSortPriority = 0;
				
		m_OnClick.Insert(OnClick);
	}
}

//! Map tool menu
class SCR_MapToolMenuUI : SCR_MapUIBaseComponent
{	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, "Menu icons imageset", "imageset")]
	ResourceName m_sToolMenuIcons;
	
	[Attribute("{47C1A2A23B9CAC97}UI/layouts/Map/MapToolButton.layout", UIWidgets.ResourceNamePicker, "Entry button prefab", "layout")]
	ResourceName m_sButtonResource;
	
	[Attribute("ToolMenu", UIWidgets.EditBox, desc: "Root frame widget name")]
	string m_sToolMenuRootName;
	
	[Attribute("ToolMenuHoriz", UIWidgets.EditBox, desc: "Tool menu widget name")]
	string m_sToolBarName;
	
	[Attribute("ToolMenuButton", UIWidgets.EditBox, desc: "Default name for generated button widgets")]
	string m_sButtonDefaultName;
	
	static ResourceName s_sToolMenuIcons;
	
	protected bool m_bIsVisible;
	protected bool m_bIsMenuFocused;
	protected Widget m_wToolMenuRoot;
	protected Widget m_wToolMenuBar;
	SCR_MapCursorModule m_CursorModule;
	
	protected ref array<ref SCR_MapToolEntry> m_aMenuEntries = {};
	
	//------------------------------------------------------------------------------------------------
	//! Returns default button name. Bear in mind that actual buttons have added index number to its end in PopulateToolMenu()
	string GetDefaultButtonName()
	{
		return m_sButtonDefaultName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_MapToolEntry> GetMenuEntries()
	{			
		return m_aMenuEntries;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register menu entry
	//! \param[in] imageset is source imageset
	//! \param[in] icon is quad from the provided imageset
	//! \param[in] sortPriority is disply priority of the icon within the menu, lower value means higher priority
	SCR_MapToolEntry RegisterToolMenuEntry(ResourceName imageset, string icon, int sortPriority)
	{			
		SCR_MapToolEntry entry = new SCR_MapToolEntry(this, imageset, icon, sortPriority);
		m_aMenuEntries.Insert(entry);

		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add custom inherited entry
	//! \param[in] customEntry is the subject
	void RegisterEntryCustom(SCR_MapToolEntry customEntry)
	{
		m_aMenuEntries.Insert(customEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Build entries
	protected void PopulateToolMenu()
	{
		Widget button;
		SCR_ToolMenuButtonComponent buttonComp;
		
		bool sorted = false;
		int count = m_aMenuEntries.Count() - 1;

		if (count < 2)
			return;

		while (!sorted)		// sort by prio
		{
			sorted = true;
			
			for (int i = 0; i < count; i++)
			{
				if (m_aMenuEntries[i + 1].m_iSortPriority < m_aMenuEntries[i].m_iSortPriority)
				{
					m_aMenuEntries.SwapItems(i, i+1);
					sorted = false;
				}
			}
		}
		
		foreach (int i, SCR_MapToolEntry entry : m_aMenuEntries)	// use the cached entry data to create layouts and find button handlers
		{
			button = GetGame().GetWorkspace().CreateWidgets(m_sButtonResource, m_wToolMenuBar);
			button.SetName(m_sButtonDefaultName + i);
			
			buttonComp = SCR_ToolMenuButtonComponent.Cast(button.FindHandler(SCR_ToolMenuButtonComponent));
			if (buttonComp)
			{
				buttonComp.m_OnClicked = entry.m_OnClick;
				buttonComp.SetImage(entry.m_sImageSet, entry.m_sIconQuad);
							
				entry.m_ButtonComp = buttonComp;
				
				entry.UpdateVisual();
			}	
		}
		
		SetUIVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Focus menu event when using controller
	protected void OnFocusToolMenu(float value, EActionTrigger reason)
	{
		SetToolMenuFocused(!m_wToolMenuBar.IsEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFreeCursor(float value, EActionTrigger reason)
	{
		SetToolMenuFocused(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapPan(float x, float y, bool adjusted)
	{
		SetToolMenuFocused(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set tool menu visibility
	//! \param[in] state is target visibility
	protected void SetUIVisible(bool state)
	{
		m_bIsVisible = state;
		m_wToolMenuRoot.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set focused state
	//! \param[in] state
	void SetToolMenuFocused(bool state)
	{
		if (state == m_bIsMenuFocused || m_aMenuEntries.IsEmpty())
			return;
				
		if (state)
		{
			if (!m_CursorModule.HandleSubMenu(true))
				return;
			
			m_wToolMenuBar.SetEnabled(true);
			GetGame().GetWorkspace().SetFocusedWidget(m_aMenuEntries[0].m_ButtonComp.GetRootWidget());
			m_bIsMenuFocused = true;
		}
		else 	//  Disable tool menu besides root, so its buttons arent being focused when unwanted
		{
			m_wToolMenuBar.SetEnabled(false);
			GetGame().GetWorkspace().SetFocusedWidget(null);
			m_CursorModule.HandleSubMenu(false);
			m_bIsMenuFocused = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnMouseEnter(Widget w, int x, int y)
	{		
		if (m_wToolMenuRoot && w == m_wToolMenuRoot && !m_wToolMenuBar.IsEnabled())	 // menu is disabled
		{
			if (m_CursorModule.HandleSubMenu(true))
				m_wToolMenuBar.SetEnabled(true);
		}
		
		GetGame().GetWorkspace().SetFocusedWidget(w);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (RenderTargetWidget.Cast(enterW) && w.IsEnabled())	// disable menu
		{
			m_wToolMenuBar.SetEnabled(false);
			m_CursorModule.HandleSubMenu(false);
			
			GetGame().GetWorkspace().SetFocusedWidget(null);
		}
		
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{		
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		
		m_wToolMenuRoot = m_MapEntity.GetMapMenuRoot().FindAnyWidget(m_sToolMenuRootName);
		if (m_wToolMenuRoot)
			m_wToolMenuBar = m_wToolMenuRoot.FindAnyWidget(m_sToolBarName);
		
		m_wToolMenuRoot.AddHandler(this);
		
		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
		{
			inputMgr.AddActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
			inputMgr.AddActionListener("MapGamepadCursorX", EActionTrigger.DOWN, OnFreeCursor);
			inputMgr.AddActionListener("MapGamepadCursorY", EActionTrigger.DOWN, OnFreeCursor);
		}
		
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);
		
		PopulateToolMenu();
				
		m_wToolMenuBar.SetEnabled(false); // by default not interactable unless enabled by entering with mouse/activating through input
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_wToolMenuRoot)
			m_wToolMenuRoot.RemoveHandler(this);
		
		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
		{
			inputMgr.RemoveActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
			inputMgr.RemoveActionListener("MapGamepadCursorX", EActionTrigger.DOWN, OnFreeCursor);
			inputMgr.RemoveActionListener("MapGamepadCursorY", EActionTrigger.DOWN, OnFreeCursor);
		}
		
		m_MapEntity.GetOnMapPan().Remove(OnMapPan);
	}
	
	//------------------------------------------------------------------------------------------------	
	bool IsFocused()
	{
		return m_bIsMenuFocused;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapToolMenuUI()
	{
		s_sToolMenuIcons = m_sToolMenuIcons;
	}
}
