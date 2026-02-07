//------------------------------------------------------------------------------------------------
//! Fulscreen map radial menu UI
class SCR_MapContextualMenuUI: SCR_MapUIBaseComponent
{
	// TODO rename class & rewrite ctx based entries into radial menu ones
	
	protected Widget 				m_wRadialMenuRoot;
	protected SCR_MapCursorModule 	m_CursorModule;
	protected ref SCR_RadialMenuMap m_RadialMenuMap;
	protected ref SCR_RadialHandlerMap m_RadialHandler;
	
	protected bool m_bRefresh;
	protected bool m_bEntriesUpdate = false;		// entries updated instead of entry selected

	protected ref map<int, ref SCR_MapMenuEntry>	m_aRadialEntries = new map<int, ref SCR_MapMenuEntry>;			// lookup of entries to avoid duplicates
	protected ref map<int, ref SCR_MapMenuCategory>	m_aRadialCategories = new map<int, ref SCR_MapMenuCategory>;	// lookup of entries to avoid duplicates

	protected vector m_vMenuWorldPos;
	
	protected ref ScriptInvoker<> m_OnMenuOpen = new ScriptInvoker();
	protected ref ScriptInvoker<BaseSelectionMenuEntry, float[]> m_OnEntryPerformed = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMenuOpenInvoker() { return m_OnMenuOpen; }
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntryPerformedInvoker() { return m_OnEntryPerformed; }
	
	//------------------------------------------------------------------------------------------------
	static SCR_MapContextualMenuUI GetInstance()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return null;
		
		return SCR_MapContextualMenuUI.Cast(mapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
	}
	
	//------------------------------------------------------------------------------------------------
	int GetEntryCount()
	{
		return m_aRadialEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMenuWorldPosition()
	{				
		m_vMenuWorldPos[1] = GetGame().GetWorld().GetSurfaceY(m_vMenuWorldPos[0], m_vMenuWorldPos[2]);
		
		return m_vMenuWorldPos;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Toggles menu 
	void ToggleMenu(IEntity owner, bool isOpen)
	{
		if (isOpen)
			OpenMenu();
		else 
			CloseMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillMenuEntries()
	{
		m_OnMenuOpen.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Opens radial menu
	//! \return false if cannot open due to not having any entries
	bool OpenMenu()
	{		
		float wX, wY, sX, sY;
		m_MapEntity.GetMapCursorWorldPosition(wX, wY);
		m_MapEntity.WorldToScreen(wX, wY, sX, sY);
		m_MapEntity.PanSmooth(sX, sY);
		
		m_vMenuWorldPos[0] = wX;
		m_vMenuWorldPos[2] = wY;
		
		m_CursorModule.HandleContextualMenu();
		m_RadialMenuMap.DisplayStartDraw(GetGame().GetPlayerController());

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Close radial menu
	void CloseMenu()
	{
		m_RadialHandler.ClearEntries(0);
		m_aRadialEntries.Clear();
		m_aRadialCategories.Clear();
				
		m_RadialMenuMap.DisplayStopDraw(GetGame().GetPlayerController());
		m_CursorModule.HandleContextualMenu(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Insert own entry into the menu
	void InsertCustomRadialEntry(SCR_MapMenuEntry entry, string category = "")
	{
		if (m_aRadialEntries.Contains(entry.m_iHash))
			return;
		
		m_aRadialEntries.Set(entry.m_iHash, entry);
		
		int categoryHash;
		if (!category.IsEmpty())
		{
			categoryHash = category.Hash();
			if (!m_aRadialCategories.Contains(categoryHash))
				AddRadialCategory(category);
		}
		
		if (!category.IsEmpty() && m_aRadialCategories.Contains(categoryHash))
			m_RadialHandler.AddElementToMenuMap(entry, m_aRadialCategories.Get(categoryHash));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Insert own category into the menu
	void InsertCustomRadialCategory(SCR_MapMenuCategory entry, string category = "")
	{
		if (m_aRadialCategories.Contains(entry.m_iHash))
			return;
		
		m_aRadialCategories.Set(entry.m_iHash, entry);
		
		int categoryHash;
		if (!category.IsEmpty())
		{
			categoryHash = category.Hash();
			if (!m_aRadialCategories.Contains(categoryHash))
				AddRadialCategory(category);
		}
		
		if (category)
			m_RadialHandler.AddElementToMenuMap(entry, m_aRadialCategories.Get(categoryHash));
		else 
			m_RadialHandler.AddElementToMenuMap(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! Add simple entry
	SCR_MapMenuEntry AddRadialEntry(string name, string category = "")
	{
		SCR_MapMenuEntry entry = new SCR_MapMenuEntry(name, category);

		if (m_aRadialEntries.Contains(entry.m_iHash))
			return m_aRadialEntries.Get(entry.m_iHash);
		
		m_aRadialEntries.Set(entry.m_iHash, entry);
		
		int categoryHash;
		if (!category.IsEmpty())
		{
			categoryHash = category.Hash();
			if (!m_aRadialCategories.Contains(categoryHash))
				AddRadialCategory(category);
		}
		
		if (!category.IsEmpty() && m_aRadialCategories.Contains(categoryHash))
			m_RadialHandler.AddElementToMenuMap(entry, m_aRadialCategories.Get(categoryHash));
		
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add simple category
	SCR_MapMenuCategory AddRadialCategory(string name, string category = "")
	{
		SCR_MapMenuCategory entry = new SCR_MapMenuCategory(name);
		
		if (m_aRadialCategories.Contains(entry.m_iHash))
			return m_aRadialCategories.Get(entry.m_iHash);
		
		m_aRadialCategories.Set(entry.m_iHash, entry);
		
		int categoryHash;
		if (!category.IsEmpty())
		{
			categoryHash = category.Hash();
			if (!m_aRadialCategories.Contains(categoryHash))
				AddRadialCategory(category);
		}
				
		if (category)
			m_RadialHandler.AddElementToMenuMap(entry, m_aRadialCategories.Get(categoryHash));
		else 
			m_RadialHandler.AddElementToMenuMap(entry);

		return entry;
	}
								
	//------------------------------------------------------------------------------------------------
	//! Entry performed event
	void OnEntryPerformed(BaseSelectionMenuEntry entry, int index)
	{
		float wX, wY;
		float worldPos[2];
		m_MapEntity.GetMapCenterWorldPosition(wX, wY);
		worldPos[0] = wX;
		worldPos[1] = wY;
		
		m_OnEntryPerformed.Invoke(entry, worldPos);
	}
		
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_wRadialMenuRoot = m_RootWidget.FindAnyWidget("RadialMenuMap");
		
		m_RadialHandler = new SCR_RadialHandlerMap();
		m_RadialHandler.InitMapHandler();
		m_RadialHandler.onMenuToggleInvoker.Insert(ToggleMenu);
		m_RadialHandler.m_OnActionPerformed.Insert(OnEntryPerformed);
		m_RadialHandler.GetRadialMenuInteraction().onAttemptMenuOpenInvoker.Insert(FillMenuEntries);

		
		m_RadialMenuMap = new SCR_RadialMenuMap(GetGame().GetPlayerController());
		m_RadialMenuMap.InitMenuVisuals(m_RadialHandler, m_wRadialMenuRoot);
				
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{		
		m_RadialHandler.onMenuToggleInvoker.Remove(ToggleMenu);
		m_RadialHandler.m_OnActionPerformed.Remove(OnEntryPerformed);
		m_RadialHandler.GetRadialMenuInteraction().onAttemptMenuOpenInvoker.Remove(FillMenuEntries);
		
		if (m_RadialHandler && m_RadialHandler.IsOpen())
			CloseMenu();

		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_RadialHandler && m_RadialHandler.IsOpen())
		{				
			m_RadialHandler.Update(GetGame().GetPlayerController(), timeSlice);
			
			if (m_RadialMenuMap)
				m_RadialMenuMap.DisplayUpdate(GetGame().GetPlayerController(), timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapContextualMenuUI()
	{
		m_bHookToRoot = true;
	}
};