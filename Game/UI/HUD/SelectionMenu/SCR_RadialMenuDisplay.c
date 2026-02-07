//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RadialMenuDisplay : SCR_BaseSelectionMenuDisplay
{
	protected static const string RADIALMENU_LAYOUT_DEFAULT = "{8DE7D0C6EB809121}UI/layouts/Common/RadialMenu/radialMenu.layout";
	protected static const string RADIALMENU_ELEMENT_LAYOUT_DEFAULT = "{E5AEDB99A9A14C25}UI/layouts/Common/RadialMenu/radialElement.layout";
	
	
	//! Layout used for the radial menu
	[Attribute(RADIALMENU_LAYOUT_DEFAULT, UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_LayoutPath;
	
	//! Layout used for each element
	[Attribute(RADIALMENU_ELEMENT_LAYOUT_DEFAULT, UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_rElementLayoutPath;
	
	//! Layout used for each element
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rEmptyElementIconPath;
	
	//! The selection arrow widget found in the created layout
	protected ImageWidget m_wSelectionArrow;
	
	//! Widget that rolls on the inside ring of the radial menu showing current input
	protected ImageWidget m_wSelector;
	
	//! Title of selected element widget
	protected TextWidget m_wSelectionTitle;
	
	//! Description of selected element widget
	protected TextWidget m_wSelectionDescription;
	
	//! Hint shown below the menu widget
	protected TextWidget m_wSelectionHint;
	
	//! Root widget of the radial menu display
	protected Widget m_wRoot;
	
	//! Container for entry-widget linking
	protected ref SCR_RadialMenuWidgetPairList m_pWidgetPairs = new ref SCR_RadialMenuWidgetPairList();	
	
	//! Currently selected entry or null if none
	protected BaseSelectionMenuEntry m_pSelectedEntry;
	
	//! Last input for selection, X = horizontal, Y = vertical
	protected vector m_vSelectionInput;
	
	//! Last input in angles around the "radial" circle
	protected float m_fSelectionAngles;
	
	//! Minimum magnitude of input for selection to be valid
	protected float m_fMinInputMagnitude;
	
	//! Create instance of display with default values
	override void SetDefault()
	{
		m_LayoutPath = RADIALMENU_LAYOUT_DEFAULT;
		m_rElementLayoutPath = RADIALMENU_ELEMENT_LAYOUT_DEFAULT;
		m_rEmptyElementIconPath = SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set element's frame slot in desired way, called when element is created
	protected void PrepareElementSlot(Widget widget)
	{
		if (!widget)
			return;
		
		const float elementSize = 140.0;
		FrameSlot.SetSize(widget, elementSize , elementSize);
		FrameSlot.SetAlignment(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMin(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMax(widget, 0.5, 0.5);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set provided elements position to the provided positon in screenspace	
	protected void SetElementPosition(Widget widget, vector position)
	{
		if (!widget)
			return;
		
		FrameSlot.SetPos(widget, position[0], position[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateElementWidget(Widget root, string elementLayout)
	{
		if (!root)
			return null;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		auto wWidget = workspace.CreateWidgets(elementLayout, root);
		if (!wWidget)
			return null;
		
		PrepareElementSlot(wWidget);		
		return wWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO: Move functions to "global func"
	protected vector GetPointOnCircle(float radius, float degrees)
	{
		// -mathpi because 90 deg offset left
		float halfpi = 0.5 * Math.PI;
		float x = radius * Math.Cos(degrees - halfpi);
		float y = radius * Math.Sin(degrees - halfpi);

		return Vector(x, y, 0.0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param index Current element (one we're setting) index
	//! \param totalCount Count of all elements (so we can set uniform pos)
	protected vector GetElementPosition(int index, int totalCount, float radius)
	{
		float degs = (float)360 / (float)totalCount;
		degs *= Math.DEG2RAD;
		vector point = GetPointOnCircle(radius, degs * index);
		return point;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Applies providede element data (from entry) to the widget (value)
	protected void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed)
	{
		if (!element)
			return;
		
		//if (element.m_pEntry.CanBeShown(null, null))
		//	return;
		
		if (element.IsEmpty())
			return;
		
		ImageWidget icon = ImageWidget.Cast(element.m_pWidget.FindAnyWidget("Icon"));
		
		if (!icon)
			return;
		
		string iconPath = element.m_pEntry.GetEntryIconPath();
		
		Widget parentWidget = icon.GetParent();
		if (parentWidget)
		{
			if (canBePerformed)
				parentWidget.SetColorInt(ARGB(255,255,255,255));
			else
				parentWidget.SetColorInt(ARGB(220,128,128,128));
		}	
		
		// Set default icon
		if (iconPath == string.Empty)
		{
			string defaultIcon = SCR_RadialMenuIcons.GetDefaultIconPath(element.m_pEntry);			
			icon.LoadImageTexture(0, defaultIcon);
			icon.SetImage(0);
			icon.SetVisible(true);
			return;
		}
		
		icon.LoadImageTexture(0, iconPath);
		icon.SetImage(0);
		icon.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSelectionData(BaseSelectionMenuEntry selection, vector input, float selectionAngle)
	{
		// If "selector" should be shown,
		// show it and set rotation to current input
		bool showSelector = input.Length() > m_fMinInputMagnitude;
		if (showSelector)
		{
			if (SetVisibleSafe(m_wSelector, true))
			{
				float inputAngle = SCR_RadialMenu.GetAngle(input, vector.Up);
				m_wSelector.SetRotation(inputAngle * Math.RAD2DEG);
			}
		}
		else
		{
			SetVisibleSafe(m_wSelector, false);
		}
		
		// If no selection is active, disable all other elements
		if (!selection)
		{
			SetVisibleSafe(m_wSelectionTitle, false);
			SetVisibleSafe(m_wSelectionHint, false);
			SetVisibleSafe(m_wSelectionDescription, false);
			SetVisibleSafe(m_wSelectionArrow, false);
			return;
		}
		
		// Update selection arrow rotation and visibility
		if (SetVisibleSafe(m_wSelectionArrow, true))
		{
			m_wSelectionArrow.SetRotation(selectionAngle * Math.RAD2DEG);
		}
		
		
		// Finally updated title, hint and description of selected elemenet
		string title = selection.GetEntryName();
		string hint = string.Empty; // NYI
		string description = selection.GetEntryDescription();
		
		// Title
		if (m_wSelectionTitle) 
		{
			bool showTitle = title != string.Empty;
			m_wSelectionTitle.SetVisible(showTitle);
			if (showTitle)
				m_wSelectionTitle.SetText(title);
		}
			
		// Hint
		if (m_wSelectionHint)
		{
			bool showHint = hint != string.Empty;
			m_wSelectionHint.SetVisible(showHint);
			if (showHint)
				m_wSelectionHint.SetText(hint);
			
		}
		
		
		// Description
		if (m_wSelectionDescription)
		{
			bool showDesc = description != string.Empty;
			m_wSelectionDescription.SetVisible(showDesc);
			if (showDesc)
				m_wSelectionDescription.SetText(description);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to set selected entry. Can be null as for no selection.
	override void SetSelection(BaseSelectionMenuEntry selectedEntry, vector selectionInput, float selectionAngle, float minInputMagnitude)
	{
		m_pSelectedEntry = selectedEntry;
		m_vSelectionInput = selectionInput;
		m_fSelectionAngles = selectionAngle;
		m_fMinInputMagnitude = minInputMagnitude;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetContent(array<BaseSelectionMenuEntry> allEntries, array<BaseSelectionMenuEntry> disabledEntries)
	{
		int originalCount = m_pWidgetPairs.Count();
		int incomingCount = allEntries.Count();
		
		// TODO: Move
		const float radius = 256.0;
		
		// If we need to shrink the content, do so
		if (incomingCount < originalCount)
		{
			for (int i = originalCount-1; i >= incomingCount; i--)
			{
				DestroyElementWidgetAt(i);
			}
		}
		
		for (int i = 0; i < incomingCount; i++)
		{
			// TODO: Change way how this is passed in, to prevent
			// looking up an array every time
			auto currentEntry = allEntries[i];
			bool canBePerformed = disabledEntries.Find(currentEntry) == -1;
			
			// For widgets which already exist, 
			// just update entries
			if (i < originalCount)
			{
				m_pWidgetPairs.SetEntryAt(i, currentEntry);
				
				// Check if we have a widget already,
				// if not proceed with creating new one
				auto pair = m_pWidgetPairs.GetAt(i);
				auto widget = pair.m_pWidget;
				if (widget)
				{
					auto position = GetElementPosition(i, incomingCount, radius);
					SetElementPosition(widget, position);
					SetElementData(pair, canBePerformed);
					continue;
				}
			}
			
			// For widgets which do not exist,
			// create new ones
			auto widget = CreateElementWidget(m_wRoot, m_rElementLayoutPath);
			
			// Prepare pair
			ref auto pair = new ref SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry>(null, currentEntry);
			
			// If we created widget, create container with child widgets for it
			if (widget)
			{
				// Set pair value
				pair.m_pEntry = currentEntry;
				pair.m_pWidget = widget;
				
				auto position = GetElementPosition(i, incomingCount, radius);
				SetElementPosition(widget, position);
				SetElementData(pair, canBePerformed);
			}
			
			m_pWidgetPairs.AddEntry(pair);
		}
		
		// Update rm with selected item  data
		SetSelectionData(m_pSelectedEntry, m_vSelectionInput, m_fSelectionAngles);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle internal display state
	override void SetOpen(IEntity owner, bool open)
	{
		if (!m_bIsOpen && open)
		{
			OnOpen(owner);
		}
		else if (m_bIsOpen && !open)
		{
			OnClose(owner);
		}
		
		super.SetOpen(owner, open);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set an element's visible state (if its not null)
	//! Returns true if element is valid, false otherwise
	protected bool SetVisibleSafe(Widget widget, bool show)
	{
		if (widget)
		{
			widget.SetVisible(show);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to create all UI elements before the menu is even open
	protected void OnCreate(IEntity owner)
	{
		if (m_LayoutPath == string.Empty)
		{
			Print("Attempt to create SCR_RadialMenuDisplay with empty layout was caught!", LogLevel.ERROR);
			return;
		}
		
		// Delete old layout
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		// Create new layout
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_LayoutPath);
		if (!m_wRoot)
			return;
		
		// Set references to widgets from new layout
		// and clear their visible flag straight away
		m_wSelectionArrow = ImageWidget.Cast(m_wRoot.FindAnyWidget("SelectionArrow"));
		SetVisibleSafe(m_wSelectionArrow, false);
		
		m_wSelectionTitle = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleText"));
		SetVisibleSafe(m_wSelectionTitle, false);
		
		m_wSelectionDescription = TextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionText"));
		SetVisibleSafe(m_wSelectionDescription, false);
		
		m_wSelectionHint = TextWidget.Cast(m_wRoot.FindAnyWidget("HintText"));
		SetVisibleSafe(m_wSelectionHint, false);
		
		m_wSelector = ImageWidget.Cast(m_wRoot.FindAnyWidget("Selector"));
		SetVisibleSafe(m_wSelector, false);	
		
		SetVisibleSafe(m_wRoot, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to dispose of ALL UI elements
	protected void OnDestroy(IEntity owner)
	{
		// Destroy widgets and 
		// Clear any soft links between entry-widgets
		DestroyElementWidgets();
		m_pWidgetPairs.Clear();
		
		if (!m_wRoot)
			return;
		
		// Delete the layout
		m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyElementWidgetAt(int index)
	{
		if (index < 0 || index >= m_pWidgetPairs.Count())
			return;
		
		auto pair = m_pWidgetPairs.GetAt(index);
		if (pair && pair.m_pWidget)
			pair.m_pWidget.RemoveFromHierarchy();
		
		m_pWidgetPairs.RemoveAt(index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! For each link stored in m_pWidgetPairs destroy not null widgets
	protected void DestroyElementWidgets()
	{
		int count = m_pWidgetPairs.Count();
		for (int i = count-1; i >= 0; i--)
		{
			DestroyElementWidgetAt(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when menu open is requested. It shouldnt be mandatory to recreate all UI elements,
	//! but the content might have changed, so that needs to be handled accordingly
	protected event void OnOpen(IEntity owner)
	{
		// We do not have root widget, therefore
		// we need to create the layout first
		if (!m_wRoot)
		{
			OnCreate(owner);
		}
		
		SetVisibleSafe(m_wRoot, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when menu close if requested. It shouldnt be mandatory to delete all UI elements,
	//! but the root element should be hidden during this event
	protected event void OnClose(IEntity owner)
	{
		SetVisibleSafe(m_wRoot, false);
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuDisplay(IEntity owner)
	{
		OnCreate(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuDisplay()
	{
		OnDestroy(null);
	}
};

//------------------------------------------------------------------------------------------------
//! Wrapper around list of radial menu pairs (entry, widget) to provide easy
//! way of handling entry-widget linking in the radial menu
//! ValueType should inherit from Widget
//! EntryType should inherit from BaseSelectionMenuEntry
class SCR_RadialMenuWidgetPairList
{
	protected ref array<Widget> m_aWidgets;
	protected ref array<BaseSelectionMenuEntry> m_aEntries;
	protected int m_iCount;
	
	//------------------------------------------------------------------------------------------------
	protected void RecalculateCount()
	{
		m_iCount = m_aWidgets.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aWidgets.Clear();
		m_aEntries.Clear();
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	int Count(bool recalculate = false)
	{
		if (recalculate)
			RecalculateCount();
		
		return m_iCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveAt(int index)
	{
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aWidgets.Remove(index);
		m_aEntries.Remove(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(SCR_RadialMenuPair pair)
	{
		if (pair == null)
			return;
		
		m_aWidgets.Insert(pair.m_pWidget);
		m_aEntries.Insert(pair.m_pEntry);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(Widget widget, BaseSelectionMenuEntry entry)
	{
		ref auto pair = new SCR_RadialMenuPair(widget, entry);
		AddEntry(pair);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveByWidget(Widget widget)
	{
		if (widget == null)
			return;
		
		auto index = m_aWidgets.Find(widget);
		if (index == -1)
			return;
		
		RemoveAt(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveByEntry(BaseSelectionMenuEntry entry)
	{	
		if (entry == null)
			return;
		
		auto index = m_aEntries.Find(entry);
		if (index == -1)
			return;
		
		RemoveAt(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryAt(int index, BaseSelectionMenuEntry entry)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aEntries[index] = entry;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWidgetAt(int index, Widget value)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aWidgets[index] = value;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetWidgetAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		return m_aWidgets[index];
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuEntry GetEntryAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		return m_aEntries[index];
	}	
	
	//------------------------------------------------------------------------------------------------
	ref SCR_RadialMenuPair GetAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		ref auto pair = new ref SCR_RadialMenuPair(m_aWidgets[index], m_aEntries[index]);
		return pair;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuWidgetPairList()
	{
		m_aWidgets = new ref array<Widget>();
		m_aEntries = new ref array<BaseSelectionMenuEntry>();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuWidgetPairList()
	{
		m_aWidgets.Clear();
		m_aEntries.Clear();
		
		m_aWidgets = null;
		m_aEntries = null;
	}
};

//------------------------------------------------------------------------------------------------
//! Container or SCR_RadialMenu, keeps entry and widget pair
//! Both widget and entry can be null
//! Value type should most likely inherit from any Widget class
class SCR_RadialMenuPair
{
	Widget m_pWidget;
	BaseSelectionMenuEntry m_pEntry;
	
	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return (!m_pWidget && !m_pEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuPair(Widget widget, BaseSelectionMenuEntry entry)
	{
		m_pWidget = widget;
		m_pEntry = entry;
	}
};