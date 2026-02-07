/*
Filter panel component which is commonly found in content browser, scenarios menu, and other places.

It controls a filter list box and a set of filter buttons which are added to the top bar when filter items are selected.
When we click on these buttons, they are removed and the according filter in the list is disabled.

So, this panel just keeps the list box and buttons synchronized. Use the public API to get selected filters.
*/

class SCR_FilterPanelComponent : ScriptedWidgetComponent
{
	const ResourceName LAYOUT_TOP_BAR_FILTER_BUTTON = "{3B47431EF3FA03F0}UI/layouts/Menus/ContentBrowser/Buttons/ContentBrowser_ButtonFilterTopBar.layout";
	
	[Attribute()]
	ref SCR_FilterSet m_FilterSet;
	
	[Attribute("6", UIWidgets.Auto, "When more than this amount of buttons are added to the top bar, they are hidden.")]
	protected int m_iMaxFilterButtons;
	
	[Attribute("false", UIWidgets.Auto, "Is the filter list or main panel content shown at start?")]
	protected bool m_bFilterShownAtStart;
	
	[Attribute("", UIWidgets.Auto, "Unique tag which will be used for saving this filter set in use settings. If left empty, filter set is not saved and not loaded.")]
	protected string m_sFilterSetStorageTag;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout for mutually exclusive filter elements", params: "layout")]
	protected ResourceName m_sElementLayoutMututallyExclusive;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout for general filter elements", params: "layout")]
	protected ResourceName m_sElementLayoutGeneral;
	
	
	ref SCR_FilterPanelWidgets m_Widgets = new SCR_FilterPanelWidgets;
	Widget m_wRoot;
	
	// Event handlers
	ref ScriptInvoker m_OnFilterChanged = new ScriptInvoker;
	ref ScriptInvoker m_OnFilterPanelToggled = new ScriptInvoker; // (bool newState) - true when new state is filter panel opened
	
	
	// --------------- Public API -----------------------
	
	
	//----------------------------------------------------------------------------------------
	SCR_FilterSet GetFilter()
	{
		return m_FilterSet;
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns the component of the search edit box
	SCR_EditBoxSearchComponent GetEditBoxSearch()
	{
		return m_Widgets.m_FilterSearchComponent;
	}
	
	
	//----------------------------------------------------------------------------------------
	//! Shows either the main content of the panel or the filter
	void ShowFilterListBox(bool show)
	{
		m_Widgets.m_FilterButtonComponent.SetToggled(show, false);
		_ShowFilterListBox(show);
	}
	
	//----------------------------------------------------------------------------------------
	bool GetFilterListBoxShown()
	{
		return m_Widgets.m_FilterButtonComponent.GetToggled();
	}
	
	
	//----------------------------------------------------------------------------------------
	//! Selects or deselects the filter visually.
	//! !!! Doesn't update the internal state of the filter. You still need to call filter.SetSelected();
	void SelectFilter(SCR_FilterEntry filter, bool select, bool invokeOnChanged = true)
	{
		Filter_ListBox_EnableFilter(filter, select);
		
		if (select)
		{
			// Add button at the top bar. But don't add it for a default filter entry in a mutually exclusive category.
			if (! (filter.m_bSelectedAtStart && filter.GetCategory().m_bMutuallyExclusive))
				Filter_TopBar_AddButton(filter);
		}
		else
			Filter_TopBar_DeleteButton(filter);
		
		if (invokeOnChanged)
			m_OnFilterChanged.Invoke(filter);
	}
	
	//----------------------------------------------------------------------------------------
	//! Saves the filter configuration in user settings.
	//! tag - optional parameter, a tag used for saving the filter.
	//! if tag is not provided, the internal m_sFilterSetStorageTag is used from the attribute.
	void Save(string tag = string.Empty)
	{
		if (tag.IsEmpty())
			tag = m_sFilterSetStorageTag;
		
		if (tag.IsEmpty())
			return;
		
		if (m_sFilterSetStorageTag && m_FilterSet)
		{
			SCR_AllFilterSetsStorage.SaveFilterSet(tag, m_FilterSet);
		}
	}
	
	
	//----------------------------------------------------------------------------------------
	void ResetToDefaultValues()
	{
		m_FilterSet.ResetToDefaultValues();
		SyncInternalToUi();
	}
	
	//----------------------------------------------------------------------------------------
	void EnableFilterButton(bool enable)
	{
		m_Widgets.m_FilterButtonComponent.SetEnabled(enable);
	}
	
	//----------------------------------------------------------------------------------------
	bool GetFilterButtonEnabled()
	{
		return m_Widgets.m_FilterButtonComponent.GetEnabled();
	}
	
	//----------------------------------------------------------------------------------------
	//! Tries to load filter values. If fails, it restores the filter to default state.
	//! tag - optional parameter, a tag used for saving the filter.
	//! if tag is not provided, the internal m_sFilterSetStorageTag is used from the attribute.
	bool TryLoad(string tag = string.Empty)
	{
		if (tag.IsEmpty())
			tag = m_sFilterSetStorageTag;
		
		if (tag.IsEmpty())
			return false;
		
		bool loadSuccess = SCR_AllFilterSetsStorage.TryLoadFilterSet(tag, m_FilterSet);
		
		if (!loadSuccess)
			m_FilterSet.ResetToDefaultValues();
		
		SyncInternalToUi();
		
		return loadSuccess;
	}
	
	
	//----------------------------------------------------------------------------------------
	string GetStorageTag()
	{
		return m_sFilterSetStorageTag;
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns true when anything inside the filter panel is focused
	bool GetFocused()
	{
		// We check if focused widget is within hierarchy of our root widget
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (!focused)
			return false;
		
		Widget parent = focused.GetParent();
		
		while (parent)
		{
			if (parent == m_wRoot)
				return true;
			
			parent = parent.GetParent();
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	void SetFocusOnFirstItem()
	{
		m_Widgets.m_FilterListBoxComponent.SetFocusOnFirstItem();
	}
	
	//----------------------------------------------------------------------------------------
	
	
	// ---------------- Protected / Private --------------
	
	
	//------------------------------------------------------------------------------------------------
	//! Enables/disables this filter in the list box
	protected void Filter_ListBox_EnableFilter(SCR_FilterEntry filter, bool enabled)
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;
		
		int item = comp.FindItemWithData(filter);
		
		if (item == -1)
			return;
		
		comp.SetItemSelected(item, enabled, false);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Resets filters in category, all except for one filter, if it's not null
	protected void Filter_ListBox_ResetFiltersInCategory(SCR_FilterCategory category, SCR_FilterEntry filterExclude)
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;
		
		for (int i = 0; i < comp.GetItemCount(); i++)
		{
			auto filter = SCR_FilterEntry.Cast(comp.GetItemData(i));
			if ((filter.GetCategory() == category) &&  (filter != filterExclude) ) // If same category as the changed item, and not this item, reset it
			{
				comp.SetItemSelected(i, false, false);
			}
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Deletes a button which matches given category and filter
	protected void Filter_TopBar_DeleteButton(SCR_FilterEntry filter)
	{
		Widget w = m_Widgets.m_FilterButtonsLayout.GetChildren();
		Widget wPrev = null;
		Widget nextFocus = null;
		int deletedButtonId = 0;
		while(w)
		{
			auto comp = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
			if (comp)
			{
				auto buttonFilter = SCR_FilterEntry.Cast(comp.GetData());
				if (buttonFilter == filter)
				{
					// If we deleted a button which was focused, we need to find a new one
					if (GetGame().GetWorkspace().GetFocusedWidget() == w)
					{
						if (w.GetSibling())
							nextFocus = w.GetSibling();
						else if (wPrev)
							nextFocus = wPrev;
						else
							nextFocus = m_Widgets.m_FilterSearch;
					}
					m_Widgets.m_FilterButtonsLayout.RemoveChild(w);
				} 
			}
			wPrev = w;
			w = w.GetSibling();
			deletedButtonId++;
		}
		
		Filter_TopBar_UpdateButtonsVisibility();
		
		if (nextFocus)
			GetGame().GetWorkspace().SetFocusedWidget(nextFocus);

		/*		
		// If we deleted a button which was focused, we need to find a new one
		if (findNewFocus)
		{
			array<Widget> buttons = new array<Widget>;
			w = m_Widgets.m_FilterButtonsLayout.GetChildren();
			while(w)
			{
				buttons.Insert(w);
				w = w.GetSibling();
			}
			
			if (deletedButtonId < buttons.Count())
				GetGame().GetWorkspace().SetFocusedWidget(buttons[deletedButtonId]);		// Focus on next button on the list, which now has same position
			else if (buttons.Count() > 0)
				GetGame().GetWorkspace().SetFocusedWidget(buttons[deletedButtonId - 1]);	// Focus on prev button if there is a prev button
			else
				GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_FilterSearch);			// Focus on search bar if there's nothing left
		}
		*/
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Deletes all buttons which are linked to filters of given category
	protected void Filter_TopBar_DeleteButtonsInCategory(SCR_FilterCategory category)
	{
		Widget w = m_Widgets.m_FilterButtonsLayout.GetChildren();
		auto widgetsToDelete = new array<Widget>;
		while(w)
		{
			auto comp = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
			if (comp)
			{
				auto buttonFilter = SCR_FilterEntry.Cast(comp.GetData());
				if (buttonFilter.GetCategory() == category)
					widgetsToDelete.Insert(w);
			}
			w = w.GetSibling();
		}
		
		foreach (Widget wdelete : widgetsToDelete)
			m_Widgets.m_FilterButtonsLayout.RemoveChild(wdelete);
		
		Filter_TopBar_UpdateButtonsVisibility();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Adds a top bar filter button, links it with this filter category and filter
	protected void Filter_TopBar_AddButton(SCR_FilterEntry filter)
	{
		// Bail if another button with same filter already exists
		Widget layoutContent = m_Widgets.m_FilterButtonsLayout.GetChildren();
		while (layoutContent)
		{
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.Cast(layoutContent.FindHandler(SCR_ModularButtonComponent));
			if (comp)
			{
				if (comp.GetData() == filter)
					return;
			}
			layoutContent = layoutContent.GetSibling();
		}
		
		
		Widget w = GetGame().GetWorkspace().CreateWidgets(LAYOUT_TOP_BAR_FILTER_BUTTON, m_Widgets.m_FilterButtonsLayout);
		SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
			
		ResourceName iconImageSet = filter.m_sImageTexture;
		if (iconImageSet.IsEmpty())
			iconImageSet = filter.GetCategory().m_sFilterImageSet;
		
		ImageWidget wImage = ImageWidget.Cast(comp.GetRootWidget().FindAnyWidget("Image"));
		wImage.LoadImageFromSet(0, iconImageSet, filter.m_sImageName);
		
		auto buttonData = filter;
		comp.SetData(buttonData);
		
		comp.m_OnClicked.Insert(OnTopBarFilterButton);
		
		Filter_TopBar_UpdateButtonsVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hides latest buttons if there are above m_iMaxFilterButtons buttons.
	//! Also updates the text of extra button count.
	protected void Filter_TopBar_UpdateButtonsVisibility()
	{
		int nButtons = 0;
		Widget b = m_Widgets.m_FilterButtonsLayout.GetChildren();
		Widget firstButton = b;
		
		// Count how many buttons there are...
		while (b)
		{
			nButtons++;
			b = b.GetSibling();
		}
		
		// Hide buttons above threshold, show others
		// It's easier to redo it again for all buttons in case some where removed from the middle
		b = firstButton;
		for (int i = 0; i < nButtons; i++)
		{
			bool visible = i < m_iMaxFilterButtons;
			b.SetVisible(visible);
			b = b.GetSibling();
		}
		
		// Show the +123 text
		if (nButtons > m_iMaxFilterButtons)
		{
			int nExtra = nButtons - m_iMaxFilterButtons;
			string s = string.Format("+%1", nExtra);
			m_Widgets.m_FilterButtonsCountText.SetText(s);
			m_Widgets.m_FilterButtonsCountText.SetVisible(true);
		}
		else
		{
			m_Widgets.m_FilterButtonsCountText.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fills the listbox with categories and filters
	protected void Filter_InitFilterPanel()
	{
		if (m_FilterSet)
		{
			// Create filter list entries according to configuration
			
			foreach (SCR_FilterCategory category : m_FilterSet.GetFilterCategories())
			{
				m_Widgets.m_FilterListBoxComponent.AddSeparator(category.m_sDisplayName);
				
				// Resolve filter element layout
				// Mutually exclusive and generic filter category have different layouts and visuals
				ResourceName elementLayout = m_sElementLayoutGeneral;
				if (category.m_bMutuallyExclusive)
					elementLayout = m_sElementLayoutMututallyExclusive;
				
				foreach (SCR_FilterEntry filter : category.GetFilters())
				{	
					// Image set can be from filter or from category
					ResourceName iconImageSet = filter.m_sImageTexture;
					if (iconImageSet.IsEmpty())
						iconImageSet = category.m_sFilterImageSet;
					
					int id = m_Widgets.m_FilterListBoxComponent.AddItemAndIcon(filter.m_sDisplayName, iconImageSet,
						filter.m_sImageName, filter, elementLayout);
				}
			}			
		}
	}
	
	
	//----------------------------------------------------------------------------------------
	//! Updates internal values from UI
	protected void SyncUiToInternal()
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;
		int nElements = comp.GetItemCount();
		for (int i = 0; i < nElements; i++)
		{
			SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetItemData(i));
			if (filter)
			{
				filter.SetSelected(comp.IsItemSelected(i));
			}
		}
	}
	
	
	//----------------------------------------------------------------------------------------
	//! Updates UI from internal values
	protected void SyncInternalToUi()
	{
		// Select filters which are selected in the filter set.
		auto categories = m_FilterSet.GetFilterCategories();
		foreach (SCR_FilterCategory category : categories)
		{
			auto filters = category.GetFilters();
			foreach (SCR_FilterEntry filter : filters)
			{	
				SelectFilter(filter, filter.GetSelected(), false);
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	//! Shows either the main content of the panel or the filter
	protected void _ShowFilterListBox(bool show)
	{
		m_Widgets.m_MainContent.SetVisible(!show);
		m_Widgets.m_FilterListBoxOverlay.SetVisible(show);
		
		m_OnFilterPanelToggled.Invoke(show);
	}
	
	
	
	//---------------------------------- EVENT HANDLERS ------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Called when top bar filter button is clicked
	//! Button is deleted, according filter in the listbox is unchecked
	protected void OnTopBarFilterButton(SCR_ModularButtonComponent comp)
	{	
		SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetData());	
		
		// If it's last selected entry and we don't allow to have nothing selected here ...
		if (!filter.GetCategory().m_bAllowNothingSelected && filter.GetCategory().GetSelectedCount() == 1)
		{
			// Try to select first entry which is enabled at start
			SCR_FilterEntry defaultFilter;
			foreach (SCR_FilterEntry f : filter.GetCategory().GetFilters())
			{
				if (f.m_bSelectedAtStart)
				{
					defaultFilter = f;
					break;
				}
			}
			
			// Bail if default filter is not found, this is not a defined behaviour
			if (!defaultFilter)
				return;
			
			Filter_ListBox_EnableFilter(defaultFilter, true);
		}
		
		Filter_ListBox_EnableFilter(filter, false);
		
		// Delete this button
		Filter_TopBar_DeleteButton(filter);
		
		SyncUiToInternal();
		m_OnFilterChanged.Invoke(filter);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called when filter listbox selection changes
	protected void OnFilterListBoxChanged(SCR_ListBoxComponent comp, int item, bool newSelected)
	{
		// If filters in this category are mutually exclusive
		// disable all other filters in same category
		SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetItemData(item));
		
		bool preventDeselection = !newSelected && !filter.GetCategory().m_bAllowNothingSelected && filter.GetCategory().GetSelectedCount() == 1;
		
		if (filter.GetCategory().m_bMutuallyExclusive && !preventDeselection)
		{
			Filter_ListBox_ResetFiltersInCategory(filter.GetCategory(), filter);
			Filter_TopBar_DeleteButtonsInCategory(filter.GetCategory()); 
		}
			
		// Add button to the top row, if this item was toggled on
		if (newSelected)
		{
			if (! (filter.m_bSelectedAtStart && filter.GetCategory().m_bMutuallyExclusive))
				Filter_TopBar_AddButton(filter);
		}
		else
		{
			// If it's last item deselected, and it's not allowed to have nothing selected, switch it back.
			if (preventDeselection)
			{
				comp.SetItemSelected(item, true, false);
			}
			else
				Filter_TopBar_DeleteButton(filter);
		}
			
		SyncUiToInternal();
		
		if (!preventDeselection)
			m_OnFilterChanged.Invoke(filter);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnToggleFilterPanelButton(SCR_ModularButtonComponent comp, bool newToggled)
	{
		_ShowFilterListBox(newToggled);
	}
	
	
	
	//----------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		
		Filter_InitFilterPanel();
		
		_ShowFilterListBox(m_bFilterShownAtStart);
		m_Widgets.m_FilterButtonComponent.SetToggled(m_bFilterShownAtStart, false);
		
		Filter_TopBar_UpdateButtonsVisibility();
		
		m_Widgets.m_FilterButtonComponent.m_OnToggled.Insert(OnToggleFilterPanelButton);
		m_Widgets.m_FilterListBoxComponent.m_OnChanged.Insert(OnFilterListBoxChanged);
	}
};