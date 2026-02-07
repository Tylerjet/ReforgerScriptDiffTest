/*
Pooled scrollable list that can endless simulate scrolling with only small amount of widgets.
Component should behave same way as standart scrolling, but should pool used widget and change their data.
*/

//------------------------------------------------------------------------------------------------
class SCR_PooledListComponent : ScriptedWidgetComponent
{
	// Entries
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Entry widget that list is filled with.")]
	protected ResourceName m_sEntry;

	[Attribute("4", UIWidgets.EditBox, desc: "Space between each entry")] 
	protected int m_iEntriesBottomPadding;
	
	// Animation effects attributes 
	[Attribute("1", UIWidgets.EditBox, desc: "Time for apearing animation")] 
	protected float m_fAnimationAppearTime;
	
	[Attribute("10", UIWidgets.EditBox, desc: "How many entries should be created for single page")] 
	protected int m_iPageEntriesCount;
	
	// Count of all entries in list 
	protected int m_iAllEntriesCount = 0;

	// Constant sizing 
	const float SIZE_UNMEASURED = -1;
	const int CHECK_ENTRY_SIZE_DELAY = 100;
	
	// Move offset when moving up in scroll layout 
	const float ENTRY_OFFSET_UP = 0.1;
	
	// Constant content z orders 
	const int ZORDER_LIST = 1;
	const int ZORDER_OFFSET_TOP = 0;
	const int ZORDER_OFFSET_BOTTOM = 3;
	
	// Constant idget names 
	const string WIDGET_PAGES = "VPages";
	const string WIDGET_PAGE0 = "VPage0";
	const string WIDGET_PAGE1 = "VPage1";
	
	const string WIDGET_SCROLL_LAYOUT =  "ScrollLayout";
	const string WIDGET_SIZE_OFFSET_TOP = "SizeOffsetTop";
	const string WIDGET_SIZE_OFFSET_BOTTOM = "SizeOffsetBottom";
	
	const string WIDGET_FOCUS_REST = "FocusRest";
	
	// Sizes 
	protected float m_fListPxHeight = SIZE_UNMEASURED;
 	protected float m_fPagePxHeight = SIZE_UNMEASURED;
	protected float m_fViewPxHeight = SIZE_UNMEASURED;
	protected float m_fEntryPxHeight = SIZE_UNMEASURED;
	protected float m_fEntryPaddingPxHeight = SIZE_UNMEASURED;
	
	// Content widgets 
	Widget m_wRoot;
	protected ScrollLayoutWidget m_wScroll;
	
	protected Widget m_wPagesWrap;
	protected Widget m_wPage0;
	protected Widget m_wPage1;
	
	protected Widget m_wPage0FirstEntry;
	protected Widget m_wPage0LastEntry;
	protected Widget m_wPage1FirstEntry;
	protected Widget m_wPage1LastEntry;
	
	protected Widget m_wFocusRest;
	protected ref array<Widget> m_aEntryWidgets = new ref array<Widget>;
	
	protected SizeLayoutWidget m_wSizeOffsetTop;
	protected SizeLayoutWidget m_wSizeOffsetBottom;
	
	// Pages switching 
	protected bool m_bPagesInverted = false;
	protected int m_iCurrentPage = 0;
	
	protected bool m_bIsListFocused = false;
	
	// Focused widgets 
	protected int m_iFocusedEntryId = -1;
	protected Widget m_wLastFocused = null;
	protected bool m_bIsFocusVisible = false;
	
	// Scroll vars 
	protected float m_ScrollLastY = 0;
	
	// Invokers 
	ref ScriptInvoker<int> m_OnSetPage = new ScriptInvoker;
	
	//-------------------------------------
	// ScriptedWidgetComponent override
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Accessing handlers and widgets 
		m_wRoot = w;
		AccessingHandlers();	
		
		CreateEntriesWidgets();
		
		SetupOffsets(0);
		
		// Setup actions 
		GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.DOWN, OnActionUp);
		GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.DOWN, OnActionDown);
	}
	
	//-------------------------------------
	// Input actions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when up action is activated 
	protected void OnActionUp()
	{
		if (!m_bIsListFocused)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (m_wLastFocused)
			workspace.SetFocusedWidget(m_wLastFocused);

		if (m_fPagePxHeight == 0)
			return;
		
		/*float pageViewRatio = m_fViewPxHeight / m_fPagePxHeight; 
		float pageRatio = m_iAllEntriesCount - m_iPageEntriesCount * pageViewRatio;*/
		
		float invisibleEntries = EntriesOutOfView();
		if (invisibleEntries == 0)
			return;
		
		// Check current page 
		float scrollPageRatio = (m_iPageEntriesCount + ENTRY_OFFSET_UP) / invisibleEntries;
		int currentPage = Math.Floor(m_ScrollLastY / scrollPageRatio);
		
		if (m_iCurrentPage == currentPage)
			return;
		
		// Is focus on first entry of one of page 
		Widget focus = workspace.GetFocusedWidget();
		
		if (focus != m_wPage0FirstEntry && focus != m_wPage1FirstEntry)
			return;
		
		// Switch pages 
		SetCurrentPage(m_iCurrentPage - 1);
		
		if (focus == m_wPage0FirstEntry)
			workspace.SetFocusedWidget(m_wPage1LastEntry);
		else if (focus == m_wPage1FirstEntry)
			workspace.SetFocusedWidget(m_wPage0LastEntry);
		
		// Set pages up navigation beahavior 
		bool onFirstEntry = (m_iFocusedEntryId == 0);
		
		if (onFirstEntry)
			m_wPagesWrap.SetNavigation(WidgetNavigationDirection.UP, WidgetNavigationRuleType.STOP);
		else 
			m_wPagesWrap.SetNavigation(WidgetNavigationDirection.UP, WidgetNavigationRuleType.ESCAPE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when down action is activated 
	protected void OnActionDown()
	{
		if (!m_bIsListFocused)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (m_wLastFocused)
			workspace.SetFocusedWidget(m_wLastFocused);
		
		// Set pages down navigation beahavior 
		bool onLastEntry = (m_iFocusedEntryId == m_iAllEntriesCount - 1);
		
		if (onLastEntry)
			m_wPagesWrap.SetNavigation(WidgetNavigationDirection.DOWN, WidgetNavigationRuleType.STOP);
		else 
			m_wPagesWrap.SetNavigation(WidgetNavigationDirection.DOWN, WidgetNavigationRuleType.ESCAPE);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this position of scroll is changed 
	protected void OnScroll(float scrollY)
	{
		if (m_fPagePxHeight == 0)
			return;
		
		// What percent of whole list can't be seen 
		float invisibleEntries = EntriesOutOfView();
		if (invisibleEntries == 0)
			return;
		
		// Ratio between entries in single page and entries out of view
		float scrollPageRatio = (m_iPageEntriesCount) / invisibleEntries;
		
		// Check if page is changed
		int currentPage = Math.Floor(scrollY / scrollPageRatio);

		if (m_iCurrentPage != currentPage)
			SetCurrentPage(currentPage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! How many entries are of view - how many entries can be scrolled 
	protected float EntriesOutOfView()
	{
		if (m_fPagePxHeight == 0)
			return 0;
		
		// What percent of whole list can bee seen 
		float pageViewRatio = m_fViewPxHeight / m_fPagePxHeight; 
		float outOfView = m_iAllEntriesCount - m_iPageEntriesCount * pageViewRatio;
		
		if (outOfView < 0)
			return 0;
		
		return outOfView;
	}

	//-------------------------------------
	// public functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Fill entries with data 
	void UpdateEntries(bool animated = false)
	{
		// Fill list with data 
		foreach (Widget w : m_aEntryWidgets)
		{
			if (w)
				FillEntry(w);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update positions of pages and offsets 
	void UpdateScroll()
	{
 		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		// Get scroll positions 
		float scrollX, scrollY = 0;
		m_wScroll.GetSliderPos(scrollX, scrollY);
		
		// Check if current focus is in list 
		Widget focused = FocusedWidgetFromEntryList();
		
		if (focused)
			m_wLastFocused = focused;
		
		// Is list foucused - is current focus on list fallback widget or list entry?
		Widget workspaceFocused = workspace.GetFocusedWidget();
		m_bIsListFocused = (workspaceFocused == m_wFocusRest || workspaceFocused == m_wLastFocused);
		
		// Check scroll change 
		if (m_ScrollLastY == scrollY)
			return;
		
		OnScroll(scrollY);
		m_ScrollLastY = scrollY;
		
		// Check sizes 
		if (m_fEntryPxHeight == SIZE_UNMEASURED || m_fEntryPxHeight == 0 && m_iAllEntriesCount != 0)
		{
			CheckEntrySize();
			MoveToTop();
		}
		
		// Show entry focus 
		if (m_iFocusedEntryId != -1 && m_fEntryPxHeight != 0)
		{			
			// Is entry currenlty within view 
			m_bIsFocusVisible = IsFocusVisible(m_iFocusedEntryId);
			
			if (!m_bIsFocusVisible)
			{
				// Move focus to resting dummy widget if entry should not be visible
				if (m_wFocusRest)
				{
					workspace.SetFocusedWidget(m_wFocusRest);
				}
			}
			else
			{
				if (m_wLastFocused && focused != m_wLastFocused)
				{
					workspace.SetFocusedWidget(m_wLastFocused);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set which page should be currently displayed
	protected void SetCurrentPage(int page)
	{
		m_iCurrentPage = page;

		// Check if pages are inverted to simulate scrolling 
		bool pagesInverted = (m_iCurrentPage % 2 != 0);
		
		SwitchPages(pagesInverted, m_fPagePxHeight, m_iCurrentPage);
		UpdateEntries();
		
		m_OnSetPage.Invoke(page);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change order of list to fake endless scrolling 
	protected void SwitchPages(bool invert, float size, int page)
	{
		// Check entries count 
		if (m_iAllEntriesCount == 0)
			return;
		
		// Check vertical list widgets 
		if (!m_wPage0 || !m_wPage1)
			return;
		
		// Save invert 
		m_bPagesInverted = invert;

		// Check end of scrolling 
		int maxPages = Math.Floor(m_iAllEntriesCount / m_iPageEntriesCount);
		if (page > maxPages - 1)
			return;

		SetupOffsets(page);
		
		// Standart order
		if (invert)
		{
			m_wPage0.SetZOrder(ZORDER_LIST + 1);
			m_wPage1.SetZOrder(ZORDER_LIST);
			return; 
		}
		
		// Switch pages order to simulate scrolling 
		m_wPage0.SetZOrder(ZORDER_LIST);
		m_wPage1.SetZOrder(ZORDER_LIST + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set sizes of top and bottom offsets 
	protected void SetupOffsets(int page)
	{
		float entryWithOffset = m_fEntryPxHeight + m_iEntriesBottomPadding;
		
		int overflowEntryCount = m_iAllEntriesCount - m_iPageEntriesCount*2;
		float offset = entryWithOffset * overflowEntryCount - m_fPagePxHeight * page;
		
		// testing calculations 
		float pageSizeBaseOnEntry = entryWithOffset * m_iPageEntriesCount;

		m_wSizeOffsetTop.SetHeightOverride(m_fPagePxHeight * page);
		m_wSizeOffsetBottom.SetHeightOverride(offset);
		
		m_wSizeOffsetTop.SetZOrder(ZORDER_OFFSET_TOP);
		m_wSizeOffsetBottom.SetZOrder(ZORDER_OFFSET_BOTTOM);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find first available entry and focus it 
	void FocusFirstAvailableEntry()
	{
		// Get and check widget 
		Widget availableEntry = FirstAvailableEntry();
		if (!availableEntry)
			return;
		
		// Focus 
		GetGame().GetWorkspace().SetFocusedWidget(null);
		GetGame().GetWorkspace().SetFocusedWidget(availableEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Move list scroll to top
	void MoveToTop()
	{
		SetCurrentPage(0);
		OnScroll(0);
		
		m_bPagesInverted = false;
		SwitchPages(m_bPagesInverted, 0, 0);
		
		m_ScrollLastY = 0;
		m_wScroll.SetSliderPos(0, 0);
	}
	
	//-------------------------------------
	// protected functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Get reference to all needed component for handling 
	protected void AccessingHandlers() 
	{
		// Base scroll layout 
		m_wScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCROLL_LAYOUT));
		
		// Find pages 
		m_wPagesWrap = m_wRoot.FindAnyWidget(WIDGET_PAGES);
		m_wPage0 = m_wRoot.FindAnyWidget(WIDGET_PAGE0);
		m_wPage1 = m_wRoot.FindAnyWidget(WIDGET_PAGE1);
		
		m_wFocusRest = m_wRoot.FindAnyWidget(WIDGET_FOCUS_REST);
		
		// Find size offsets 
		m_wSizeOffsetTop = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SIZE_OFFSET_TOP));
		m_wSizeOffsetBottom = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SIZE_OFFSET_BOTTOM));
	}

	//------------------------------------------------------------------------------------------------
	//! Create server entries widget 
	protected void CreateEntriesWidgets()
	{
		if(!m_wRoot || m_sEntry.IsEmpty())
			return; 
		
		CreateEntiesWidgetsInPage(m_wPage0, m_iPageEntriesCount, m_wPage0FirstEntry, m_wPage0LastEntry);
		CreateEntiesWidgetsInPage(m_wPage1, m_iPageEntriesCount, m_wPage1FirstEntry, m_wPage1LastEntry);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Create entries to selected page and assing first and last entries 
	protected void CreateEntiesWidgetsInPage(Widget parent, int count, out Widget firstEntry, out Widget lastEntry)
	{
		for (int i = 0; i < count; i++)
		{
			Widget entry = CreateEntry(parent);
			
			// Assing first entry of page
			if (i == 0)
				firstEntry = entry;
			
			// Assing last entry of page
			if (i == count - 1)
				lastEntry = entry;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create entry to given page and setup it's behavior  
	protected Widget CreateEntry(Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (!m_sEntry || !parent || !workspace)
			return null;
		
		Widget widget = workspace.CreateWidgets(m_sEntry, parent);
		if (!widget)
			return null;
		
		// Setup widget entry behavior 
		SetupEntryBehavior(widget);
		
		// Save in widget list  
		m_aEntryWidgets.Insert(widget);
		
		// Set padding 
		VerticalLayoutSlot.SetPadding(widget, 0, 0, 0, m_iEntriesBottomPadding);
		
		return widget;
	}

	protected bool m_bIsMeasured = false;
	
	//------------------------------------------------------------------------------------------------
	//! Get size of widget entry used in list 
	protected void CheckEntrySize()
	{
		if (m_aEntryWidgets.IsEmpty())
			return;
		
		// Check widgets 
		if (!m_bIsMeasured)
		{
			m_wRoot.SetFlags(WidgetFlags.CLIPCHILDREN);
			m_wRoot.ClearFlags(WidgetFlags.INHERIT_CLIPPING);
			
			// Entry height in px - with bottom padding 
			float x;
			m_aEntryWidgets[0].GetScreenSize(x, m_fEntryPxHeight);
	
			// Page height in px
			m_wPage0.GetScreenSize(x, m_fPagePxHeight);
	
			// Height of view int px - how much can user see in px
			m_wRoot.GetScreenSize(x, m_fViewPxHeight);
			
			// Get real entry bottom padding in px 
			m_fEntryPaddingPxHeight = m_fPagePxHeight - m_fEntryPxHeight * m_iPageEntriesCount;
	  		m_fEntryPaddingPxHeight /= m_iPageEntriesCount;
			
			// Check measures
			if (m_fEntryPxHeight != 0)
			{
				m_bIsMeasured = true;
				m_wRoot.ClearFlags(WidgetFlags.CLIPCHILDREN);
				m_wRoot.ClearFlags(WidgetFlags.INHERIT_CLIPPING);
				SetupOffsets(m_iCurrentPage);
			}
		}
		
		// Whole height size in px - cut last bottom padding 
		m_fListPxHeight = m_fEntryPxHeight * m_iAllEntriesCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Empty functions for setting up widget entry beavhior actions, callbacks, etc.
	//! Override this to assign specific beavhior for each entry 
	protected void SetupEntryBehavior(Widget entry) { }
	
	//------------------------------------------------------------------------------------------------
	//! Empty function for filling entries with data 
	//! Override this to fill entries with data 
	protected void FillEntry(Widget w) {}
	
	//------------------------------------------------------------------------------------------------
	//! Setup opacity animation 
	protected void AnimateEntryOpacity(Widget w, int delay, float animTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			w.SetOpacity(opacityStart);
		
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, w, animTime, opacityEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(Widget w, int time, float opacityEnd) 
	{
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Opacity, opacityEnd, time);
	}

	//------------------------------------------------------------------------------------------------
	//! Display right entries with right position by scroll
	protected void ShowEntries(int dataCount)
	{
		// Show entries 
		for (int i = 0; i < m_aEntryWidgets.Count(); i++)
		{
			bool show = i < dataCount;
			m_aEntryWidgets[i].SetVisible(show);	
		}
		
		// Check if offsets needed
		int overflow = dataCount - m_iPageEntriesCount*2;
		bool addOffsets = overflow > 0;
		
		// Set offstes visible if needed
		m_wSizeOffsetTop.SetVisible(addOffsets);
		m_wSizeOffsetBottom.SetVisible(addOffsets);
		
		if (!addOffsets)
			return; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if is possible to see focused entry by id
 	protected bool IsFocusVisible(int id)
	{
		// Zero division check 
		if (m_iAllEntriesCount == 0 || m_fListPxHeight == 0)
			return false;
		
		// Position of entry in px by given id 
		float entryPosY = id * m_fEntryPxHeight;
		
		// Position in percentage
		float entryPosPerc = entryPosY / m_fListPxHeight;
		
		// View in percentage 
		float viewPerc = m_fViewPxHeight/m_fListPxHeight;
		
		// Top and bottom positions of current view in %
		float top = m_ScrollLastY * (1 - viewPerc);
		float bottom = top + viewPerc;
		
		// Entry in perc corection  
		float entryHeightPerc = m_fEntryPxHeight/m_fListPxHeight;
		bottom += entryHeightPerc;

		// Check if entry is in current view 
		if (top <= entryPosPerc && entryPosPerc <= bottom)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return if and which entry widget is actually focus 
	protected Widget FocusedWidgetFromEntryList()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		
		int focusedEntryId = m_aEntryWidgets.Find(focused);
		
		if (focusedEntryId == -1)
			return null;
		
		return focused;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFocus(Widget widget)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget focused = workspace.GetFocusedWidget();
		
		// Is focus on list widget 
		//if (focused == m_wRoot.FindAnyWidgetById)
		
		//GetGame().GetWorkspace().SetFocusedWidget(availableEntry);
		
		// Is out of view 
		if (!m_bIsFocusVisible)
		{
			// Move focus to resting dummy widget if entry should not be visible
			if (m_wFocusRest)
				workspace.SetFocusedWidget(m_wFocusRest);
			
			return;
		}
		
		
		if (widget && focused != widget)
		{
			workspace.SetFocusedWidget(widget);
		}
	}
	
	//-------------------------------------
	// Get & Set
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	int GetPageEntriesCount() { return m_iPageEntriesCount; }
	
	//------------------------------------------------------------------------------------------------
	array<Widget> GetEntryWidgets() { return m_aEntryWidgets; }
	
	//------------------------------------------------------------------------------------------------\
	//! Return hcount of entries that can be scrolled 
	int ScrollableEntriesCount() 
	{ 
		int dif = m_iAllEntriesCount - m_aEntryWidgets.Count();
		
		// No entries check 
		if (dif < 0)
			return 0;
		
		return dif;
	}
	
	//------------------------------------------------------------------------------------------------
	// Setup entries defaults and total count 
	void SetDataEntries(int entriesCount)
	{
		// Set data 
		m_iAllEntriesCount = entriesCount;
		
		// Show entries 
		ShowEntries(entriesCount);
		
		// Check sizes 
		GetGame().GetCallqueue().CallLater(CheckEntrySize, CHECK_ENTRY_SIZE_DELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return first that is visible and enabled
	Widget FirstAvailableEntry()
	{
		foreach (Widget entry : m_aEntryWidgets)
		{
			if (entry.IsVisible() && entry.IsEnabled())
				return entry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsListFocused(bool focused) { m_bIsListFocused = focused; }
	
	//------------------------------------------------------------------------------------------------
	bool IsListFocused() { return m_bIsListFocused; }
};