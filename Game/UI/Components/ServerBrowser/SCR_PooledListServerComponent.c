/*
Pooled scrollable list with server entries handling.
*/

//------------------------------------------------------------------------------------------------
class SCR_PooledServerListComponent : SCR_PooledListComponent
{
	// Server arrays 
	protected ref array<SCR_ServerBrowserEntryComponent> m_aRoomEntries = new ref array<SCR_ServerBrowserEntryComponent>;
	protected ref array<Room> m_aRooms = new ref array<Room>;
	
	// Invokers 
	ref ScriptInvoker m_OnServerFocusEnter = new ScriptInvoker;
	ref ScriptInvoker m_OnServerFavorite = new ScriptInvoker; 
	ref ScriptInvoker m_OnServerDoubleClick = new ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	// Override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override protected Widget CreateEntry(Widget parent)
	{
		Widget entry = super.CreateEntry(parent);
		
		// Check entry 
		if (!entry)
			return null;
		
		// Access server entry component 
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry.FindHandler(SCR_ServerBrowserEntryComponent));
		
		if (!serverEntry)
			return null;
		
		// Setup server entry 
		serverEntry.Event_OnFocusEnter.Insert(OnServerEntryFocusEnter);
		serverEntry.m_OnFavorite.Insert(OnServerEntryFavorite);
		
		// Save server entry 
		m_aRoomEntries.Insert(serverEntry);
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set server entry it's room data to display server info 
	override protected void FillEntry(Widget w)
	{
		int id = -1;
		
		SCR_ServerBrowserEntryComponent serverEntry = ServerEntryByWidget(w, id);
		if (!serverEntry)
			return;
		
		//id += m_iPageEntriesCount * m_iCurrentPage;
		
		if (m_bPagesInverted)
		{
			// Modify count if pages are inverted 
			if (w.GetParent() == m_wPage0)
				id += m_iPageEntriesCount;
			if (w.GetParent() == m_wPage1)
				id -= m_iPageEntriesCount;
		}
		
		// Get addons manager 
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		// Fill room
		if (0 <= id && id < m_aRooms.Count())
		{
			serverEntry.SetRoomInfo(m_aRooms[id]);
			
			// Disable moded if mods are not allowed	
			bool enable = serverEntry.GetIsModed() && !mgr.GetUgcPrivilege();
			//serverEntry.EnableEntry(!enable);
		}
		else 
		{
			serverEntry.SetRoomInfo(null);
		}
	}
	
	bool m_bMovedToRest = true;
	
	//------------------------------------------------------------------------------------------------
	//! Update positions of pages and offsets 
	override void UpdateScroll()
	{
		super.UpdateScroll();
		
		// Hide focus  
		if (m_bIsFocusVisible && !m_bMovedToRest)
		{
			RestartBorder(m_wLastFocused, 1);
			m_bMovedToRest = true;
		}
		
		if (!m_bIsFocusVisible && m_bMovedToRest)
		{
			RestartBorder(m_wLastFocused, 0);
			m_bMovedToRest = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RestartBorder(Widget entry, float opacity)
	{
		if (!entry)
			return;
		
		Widget border = entry.FindAnyWidget("Border");
		if (!border)
			return;
		
		border.SetOpacity(opacity);
		WidgetAnimator.StopAnimation(border, WidgetAnimationType.Opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	// Invoker actions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when focusing on server entry 
	//! Show data about server 
	protected void OnServerEntryFocusEnter(SCR_ServerBrowserEntryComponent serverEntry)
	{
		//m_OnServerFocusEnter.Invoke(serverEntry);
		
		// Set current focused id 
		int entryId = m_aRoomEntries.Find(serverEntry);
		entryId += m_iCurrentPage * m_iPageEntriesCount;
		
		if (m_bPagesInverted)
		{
			// Modify count if pages are inverted 
			if (serverEntry.GetRootWidget().GetParent() == m_wPage0)
				entryId += m_iPageEntriesCount;
			else if (serverEntry.GetRootWidget().GetParent() == m_wPage1)
				entryId -= m_iPageEntriesCount;
		}
		
		// Save focused id
		m_iFocusedEntryId = entryId;
		
		m_bMovedToRest = true;
		m_bIsFocusVisible = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryFavorite(SCR_ServerBrowserEntryComponent serverEntry, bool favorited)
	{
		//m_OnServerFavorite.Invoke(serverEntry, favorited);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Protectted functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Returns server entry owned by given entry widget root 
	protected SCR_ServerBrowserEntryComponent ServerEntryByWidget(Widget entryWidget, out int id)
	{
		// Check widget 
		if (!entryWidget)	
			return null;
		
		// Find server entry by id 
		id = m_aEntryWidgets.Find(entryWidget);
		
		if (0 <= id && id < m_aRoomEntries.Count())
		{
			return m_aRoomEntries[id]; 
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Public functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetRooms(array<Room> rooms, int allRoomsCount = -1, bool animate = false) 
	{ 
		m_wRoot.SetVisible(true);
		
		m_aRooms = rooms;
		
		int count = 0;
		foreach (Room room : m_aRooms)
		{
			int page = Math.Floor(count / m_iPageEntriesCount);
			count++;
		}
		
		// Update data
		int dataCount = allRoomsCount;
		if (allRoomsCount == -1)
			dataCount = m_aRooms.Count();
		
		SetDataEntries(dataCount);
		
		UpdateEntries(animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowEmptyRooms()
	{
		m_aRooms.Clear();
		UpdateEntries(false);
		
		// Hide entries 
		int count = m_aEntryWidgets.Count();
		for (int i = 0; i < count; i++)
 			m_aEntryWidgets[i].SetVisible(false);
		
		if (m_bIsListFocused)
			SetFocus(m_wFocusRest);
		
		if (m_bIsMeasured)
		{
			m_wRoot.ClearFlags(WidgetFlags.CLIPCHILDREN);
			m_wRoot.ClearFlags(WidgetFlags.INHERIT_CLIPPING);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display only entries that are fully visible in single view (without scrolling)
	protected void ShowOnlyVisibleEntries()
	{	
		if (m_fEntryPxHeight == SIZE_UNMEASURED)
			CheckEntrySize();
		
		// Full entry check 
		int entryPadded = m_fEntryPxHeight + m_fEntryPaddingPxHeight;
		if (entryPadded == 0) 
			return;
		
		// Visible entries  
		int visibleEntries = m_fViewPxHeight / entryPadded;
		if (m_iAllEntriesCount > visibleEntries)
			return;
		
		// Display only visible entries 
		for (int i = 0; i < m_iPageEntriesCount * 2; i++)
		{
			bool show = m_iPageEntriesCount * m_iCurrentPage + i < visibleEntries;
 			m_aRoomEntries[i].GetRootWidget().SetVisible(show);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ServerBrowserEntryComponent> GetRoomEntries() { return m_aRoomEntries; }
};