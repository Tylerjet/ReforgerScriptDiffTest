//------------------------------------------------------------------------------------------------
class SCR_RadialMenuVisuals : SCR_InfoDisplayExtended
{
	protected static const string RADIALMENU_LAYOUT_DEFAULT = "{8DE7D0C6EB809121}UI/layouts/Common/RadialMenu/radialMenu.layout";
	protected static const string RADIALMENU_ELEMENT_LAYOUT_DEFAULT = "{E5AEDB99A9A14C25}UI/layouts/Common/RadialMenu/radialElement.layout";
	static const string RADIALMENU_PAGE_INDICATION = "";
	
	// Widget default names - old
	const static string OLD_RADIALMENU_SELECTION = "Selection";
	const static string OLD_RADIALMENU_SELECTOR = "Selector";
	const static string OLD_RADIALMENU_TEXT_HINT = "HintText";
	const static string OLD_RADIALMENU_TEXT_TITLE = "TitleText";
	
	// Widget default names constants 
	const static string WIDGET_LAYOUT_WRAP = "OverlayWrap";
	const static string WIDGET_LAYOUT_BASE = "OverlayMenuBase";
	const static string WIDGET_LAYOUT_CONTET = "OverlayContent";
	const static string WIDGET_LAYOUT_INFO = "VInfo"; 
	const static string WIDGET_TEXT_ACTION = "TxtAction";
	const static string WIDGET_TEXT_ITEM_NAME = "TxtItemName";
	const static string WIDGET_IMAGE_SELECTOR = "ImgSelector";
	
	const static string WIDGET_SELECT_LAST = "SelectLast";
	const static string WIDGET_SELECT_CURRENT = "SelectCurrent";
	const static string WIDGET_SELECTED_PART_LEFT = "ImgLeft";
	const static string WIDGET_SELECTED_PART_RIGHT = "ImgRight";
	const static string WIDGET_SELECTED_PART_FILL = "ImgFill";
	
	const string WIDGET_LINE_IMAGE = "LineImage";
	
	// Page widgets 
	const string WIDGET_PAGES = "HPages";
	const string WIDGET_PAGE_ROOT = "HPageIndications";
	const string WIDGET_PAGE_ICON = "ImgIcon";
	const string WIDGET_PAGE_SELECT = "ImgSelect";
	const string WIDGET_PAGE_ACTION_PREV = "ActionPagePrev";
	const string WIDGET_PAGE_ACTION_NEXT = "ActionPageNext";

	
	//[Attribute("", UIWidgets.Object)]
	protected ref SCR_RadialMenuHandler m_RadialMenuHandler;
	
	//! Layout used for each element
	[Attribute(RADIALMENU_ELEMENT_LAYOUT_DEFAULT, UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_rElementLayoutPath;
	
	//! Layout used for each element
	[Attribute(SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY, UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected ResourceName m_rEmptyElementIconPath;
	
	//! Texture for border line between entries
	[Attribute(RADIALMENU_ELEMENT_LAYOUT_DEFAULT, UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_rBorderLinePath;
	
	[Attribute(RADIALMENU_PAGE_INDICATION, UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_sPageIndicationLayout;
	
	//! Image set for pages 
	[Attribute("", UIWidgets.ResourceNamePicker, "", "")]
	protected ResourceName m_sPageIconsSet;
	
	// Layout widget name references 
	[Attribute(WIDGET_LAYOUT_WRAP, UIWidgets.CheckBox, "Widget name for wrapping layout determinating size to find")]
	protected string m_sLayoutWrap;
	
	[Attribute(WIDGET_LAYOUT_BASE, UIWidgets.CheckBox, "Widget name of menu base background layout to find")]
	protected string m_sLayoutBase;
	
	[Attribute(WIDGET_LAYOUT_CONTET, UIWidgets.CheckBox, "Widget name of content layout to find")]
	protected string m_sLayoutContent;
	
	[Attribute(WIDGET_LAYOUT_INFO, UIWidgets.CheckBox, "Widget name for text wrap")]
	protected string m_sLayoutInfo;
	
	[Attribute(OLD_RADIALMENU_TEXT_HINT, UIWidgets.CheckBox, "Widget name of center text displaying actions")]
	protected string m_sTxtAction;
	
	[Attribute(OLD_RADIALMENU_TEXT_TITLE, UIWidgets.CheckBox, "Widget name of center text dispaly item name")]
	protected string m_sTxtItemName;

	[Attribute(OLD_RADIALMENU_SELECTOR, UIWidgets.CheckBox, "Widget name of pointing arrow selector")]
	protected string m_sImageSelector;
	
	[Attribute(OLD_RADIALMENU_SELECTION, UIWidgets.CheckBox, "Widget name of last selection")]
	protected string m_sSelectLast;
	
	[Attribute(WIDGET_SELECT_CURRENT, UIWidgets.CheckBox, "Widget name of current selection")]
	protected string m_sSelectCurrent;
	
	[Attribute("cancel", UIWidgets.CheckBox, "Default empty icon name")]
	protected string m_sEmtpyIconName;
	
	// Sizes and numbers 
	[Attribute("750", UIWidgets.CheckBox, "Size of radial menu right")]
	protected float m_fSizeMenu;
	
	[Attribute("750", UIWidgets.CheckBox, "Size of entry icons")]
	protected float m_fSizeEntries;
	
	[Attribute("256", UIWidgets.CheckBox, "Distance of entries from center")]
	protected float m_fRadiusEntries;
	
	[Attribute("150", UIWidgets.CheckBox, "")]
	protected float m_fSelectorDistance;
	
	[Attribute("10", UIWidgets.CheckBox, "")]
	protected float m_fFadeInSpeed;
	
	[Attribute("10", UIWidgets.CheckBox, "")]
	protected float m_fFadeOutSpeed;
	
	// Widges 
	protected Widget m_wLayoutWrap;
	protected Widget m_wLayoutBase;
	protected Widget m_wLayoutContent;
	protected Widget m_wLayoutInfo;
	protected Widget m_wSelectLast;
	protected Widget m_wSelectCurrent;
	
	protected TextWidget m_wTxtAction;
	protected TextWidget m_wTxtItemName;
	
	protected ImageWidget m_wImgSelector;
	
	protected Widget m_wPages;
	protected RichTextWidget m_RichPageActionPrev;
	protected RichTextWidget m_RichPageActionNext;
	
	//! The selection arrow widget found in the created layout
	protected ImageWidget m_wSelectionArrow;
	
	//! Outline for selected entry
	protected ImageWidget m_wSelectionOutLine;
	
	//! Left border line for selection higlight outline
	protected ImageWidget m_wSelectionLine;
	
	//! Right border line for selection higlight outline
	protected ImageWidget m_wSelectionLine2;
	
	//! Part of circle that highlights last selected entry
	protected ImageWidget m_wSelectedEntry;
	
	//! Widget that rolls on the inside ring of the radial menu showing current input
	protected ImageWidget m_wSelector;
	
	//! Title of selected element widget
	protected TextWidget m_wSelectionTitle;
	
	//! Description of selected element widget
	protected TextWidget m_wSelectionDescription;
	
	//! Container for entry-widget linking
	protected ref SCR_RadialMenuWidgetPairList m_pWidgetPairs = new ref SCR_RadialMenuWidgetPairList();	
	
	//! Currently selected entry or null if none
	protected ref BaseSelectionMenuEntry m_pSelectedEntry;
	
	//! Last input for selection, X = horizontal, Y = vertical
	protected vector m_vSelectionInput;
	
	//! Last input in angles around the "radial" circle
	protected float m_fSelectionAngles;
	
	//! Angle of last selected entry
	protected float m_fLastSelectedAngles;
	
	//! Minimum magnitude of input for selection to be valid
	protected float m_fMinInputMagnitude;
	
	//! Distance in angle between each entry
	protected float m_fEntryAngleDistance;
	
	//! Beggining angle of entries
	protected float m_fEntryAngleOffset;
	
	//! Is open - Should menu be visible
	protected bool m_bIsOpen;
	
	//! For checking if menu is opened for first time
	protected bool m_bInitialOpen;
	
	//! Will determinate if border lines between entries should be drawn or redrawn
	protected bool m_bDrawLine;
	
	protected ref array<ref SCR_SelectionEntryWidgetComponent> m_aEntryElements = new ref array<ref SCR_SelectionEntryWidgetComponent>; 
	
	//! Array for holding dadta about border lines image widgets
	protected ref array<ref ImageWidget> m_aBorderLines = new array<ref ImageWidget>();
	
	// Page variables 
	protected ref array<ref Widget> m_aPageWidgets = new ref array<ref Widget>;
	
	protected float m_fPageInacativeOpacity = 0.5;
	protected int m_iPagePadding = 2; 
	
	//------------------------------------------------------------------------------------------------
	void SetMenuHandler(SCR_RadialMenuHandler radialMenuHandler)
	{
		m_RadialMenuHandler = radialMenuHandler;
		
		if (m_RadialMenuHandler)
		{
			m_RadialMenuHandler.onMenuToggleInvoker.Insert(SetOpen);
			m_RadialMenuHandler.m_OnPageSwitch.Insert(HighlighPageIndicator);
			
			m_RadialMenuHandler.SetRadialMenuVisuals(this);
		}
		
		m_bIsOpen = false;
		m_bInitialOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectionAngle(float angle)
	{
		m_fSelectionAngles = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastSelectedAngle(float angle)
	{
		m_fLastSelectedAngles = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryDistance(float angle)
	{
		m_fEntryAngleDistance = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryOffset(float angle)
	{
		m_fEntryAngleOffset = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create instance of display with default UpdateValues
	void SetDefault()
	{
		m_LayoutPath = RADIALMENU_LAYOUT_DEFAULT;
		m_rElementLayoutPath = RADIALMENU_ELEMENT_LAYOUT_DEFAULT;
		m_rEmptyElementIconPath = SCR_RadialMenuIcons.RADIALMENU_ICON_EMPTY;
		
		m_bIsOpen = false;
		m_bInitialOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayInit(IEntity owner)
	{	
		m_bIsOpen = false;
		m_bInitialOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		AddListeners();
		
		m_bDrawLine = true;

		// Setting size
		if (m_wLayoutWrap)
			FrameSlot.SetSize(m_wLayoutWrap, m_fSizeMenu, m_fSizeMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{

	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddListeners()
	{
		if(!m_RadialMenuHandler)
			return;
		
		m_RadialMenuHandler.m_OnEntriesUpdate.Insert(SetContent);
		m_RadialMenuHandler.onPositionChangeInvoker.Insert(SetPositioningAngles);
		m_RadialMenuHandler.onSelectionUpdateInvoker.Insert(UpdateSelection);
		m_RadialMenuHandler.onSelectedEntryChangeInvoker.Insert(UpdateLastSelected);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveListeners()
	{
		if(!m_RadialMenuHandler)
			return;
		
		m_RadialMenuHandler.m_OnEntriesUpdate.Remove(SetContent);
		m_RadialMenuHandler.onPositionChangeInvoker.Remove(SetPositioningAngles);
		m_RadialMenuHandler.onSelectionUpdateInvoker.Remove(UpdateSelection);
		m_RadialMenuHandler.onSelectedEntryChangeInvoker.Remove(UpdateLastSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set element's frame slot in desired way, called when element is created
	protected void PrepareElementSlot(Widget widget)
	{
		if (!widget)
			return;
		
		const float elementSize = 256.0;
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
	//! Set size for multiple menu widget
	protected void SetWidgetChildrenSize(Widget root, string layoutName, float size)
	{
		Widget layout = root.FindAnyWidget(layoutName);
		
		if(!layout)
			return;
		
		FrameSlot.SetSize(layout, size, size);
		
		// Apply size on children
		Widget w = layout.GetChildren();
		
		int i = 0;
		
		while(true)
		{
			if(!w || !w.GetSibling() || i > 100)
				break;

			FrameSlot.SetSize(w, size, size);
			w = w.GetSibling();
			i++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set angle positioning for visuals 
	protected void SetPositioningAngles(float distanceAngle, float offsetAngle)
	{
		m_fEntryAngleDistance = distanceAngle;
		m_fEntryAngleOffset = offsetAngle;
		
		// Update selection size
		float maskProgress = m_fEntryAngleDistance / 360;		
		
		if(!m_wSelectionArrow)
			return;
		
		m_wSelectionArrow.SetMaskProgress(maskProgress);
		m_wSelectionOutLine.SetMaskProgress(maskProgress);
		
		if(!m_wSelectedEntry)
			return;
		
		m_wSelectedEntry.SetMaskProgress(maskProgress);
	}

	//------------------------------------------------------------------------------------------------
	//! Create menu entry element based on data of selection entry component 
	protected Widget CreateElementWidget(Widget root, ScriptedSelectionMenuEntry entry)
	{
		if (!root || !entry)
			return null;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget wEntry = workspace.CreateWidgets(entry.GetEntryLayout(), root);
		if (!wEntry)
			return null;
		
		// Find component 
		SCR_SelectionEntryWidgetComponent we = SCR_SelectionEntryWidgetComponent.Cast(wEntry.FindHandler(SCR_SelectionEntryWidgetComponent));
		
		entry.SetEntryComponent(we);
		
		//setup element positon 
		PrepareElementSlot(wEntry);		
		return wEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create single bordeline with given angle
	protected void CreateBorderLine(Widget root, float angle, string lineLayout)
	{
		if(!root)
			return;
		
		// Find widget parts 
		Widget w = GetGame().GetWorkspace().CreateWidgets(lineLayout, root);
		if(!w)
			return;
		
		ImageWidget wImage = ImageWidget.Cast(w.FindAnyWidget(WIDGET_LINE_IMAGE));	
		if(!wImage)
			return;
		
		// Settign properties
		float radius = FrameSlot.GetSizeX(m_wLayoutWrap);
		FrameSlot.SetSize(wImage, radius , radius);
		wImage.SetRotation(angle);
		m_aBorderLines.Insert(wImage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove borderline from radial base 
	protected void RemoveBorderLine(ImageWidget image)
	{
		if(!image)
			return;
		
		m_aBorderLines.RemoveItem(image);
		image.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updat border lines rotation and distances 
	protected void UpdateBorderLinesAngle()
	{
		int id = 0;
		
		foreach(ImageWidget line : m_aBorderLines)
		{
			float rot = id * m_fEntryAngleDistance - m_fEntryAngleDistance/2;
			
			line.SetRotation(rot);
			id++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Vector X Y positions from radisu (distance) and degrees (angle)
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
		float degs = m_fEntryAngleDistance;
		degs *= Math.DEG2RAD;
		float degsOffset = m_fEntryAngleOffset * Math.DEG2RAD;
		vector point = GetPointOnCircle(radius, degs * index + degsOffset);
		return point;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Applies providede element data (from entry) to the widget (value)
	protected void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed, SCR_SelectionEntryWidgetComponent widgetComp)
	{
		if (!element)
			return;

		if (element.IsEmpty())
			return;
		
		// Set entry default icon 
		ScriptedSelectionMenuEntry entryGrouped = ScriptedSelectionMenuEntry.Cast(element.m_pEntry);
		if (!entryGrouped)
			return;
		
		SCR_SelectionEntryWidgetComponent entryWidget = entryGrouped.GetEntryComponent();
		if (!entryWidget)
			return;
		
		entryWidget.SetIcon(m_sPageIconsSet, SetEmptyIconByType(entryGrouped));
		
		if (widgetComp)
		{
			entryWidget.SetIcon(widgetComp.GetImageTexture(), widgetComp.GetImageName());
		}
		else
			entryWidget.SetIcon(element.m_pEntry.GetEntryIconPath());
		
		//entryWidget.SetIconFromData();
	
		// Setting of icon
		ImageWidget icon = ImageWidget.Cast(element.m_pWidget.FindAnyWidget("Icon"));
		
		if (!icon)
			return;
		
		BaseSelectionMenuEntry entry = element.m_pEntry;
		
		if(!entry)
			return;
		
		UIInfo uiInfo = entry.GetUIInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set icon names by type of the entry  
	protected string SetEmptyIconByType(ScriptedSelectionMenuEntry entry)
	{
		return m_sEmtpyIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updating data for selection - selector angle, currently selected entry
	protected void UpdateSelection(bool showSelector, bool performable, float selectorAngle, float selectedAngle, BaseSelectionMenuEntry selectedEntry)
	{
		// Update selector position
		if (showSelector)
		{
			if (SetVisibleSafe(m_wImgSelector, true))
			{
				vector vec = GetPointOnCircle(m_fSelectorDistance, selectorAngle * Math.DEG2RAD);
				FrameSlot.SetPos(m_wImgSelector, vec[0], vec[1]);
				m_wImgSelector.SetRotation(selectorAngle);
			}
		}
		else
		{
			SetVisibleSafe(m_wImgSelector, false);
		}
		
		// Set layout info and current selection by actual selectino
		bool isSelected = (selectedEntry != null);
		SetVisibleSafe(m_wLayoutInfo, isSelected);
		SetVisibleSafe(m_wSelectCurrent, isSelected);
		
		// If no selection is active, disable all other elements
		if (!selectedEntry)
		{
			SetVisibleSafe(m_wSelectionTitle, false);
			SetVisibleSafe(m_wSelectionDescription, false);
			SetVisibleSafe(m_wSelectionArrow, false);
			SetVisibleSafe(m_wSelectionOutLine, false);
			SetVisibleSafe(m_wSelectionLine, false);
			SetVisibleSafe(m_wSelectionLine2, false);
			return;
		}
		
		// Update selection arrow rotation and visibility
		if(performable)
		{
			if (SetVisibleSafe(m_wSelectCurrent, true))
			{
				float angle = selectedAngle;
				HandleWidgetSelected(m_wSelectCurrent, angle);
				SetVisibleSafe(m_wSelectCurrent, true);
			}
		}
		else
		{
			SetVisibleSafe(m_wSelectCurrent, false);
		}
		
		// Entry info
		ShowEntryInfo(selectedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show description of selected entry
	protected void ShowEntryInfo(BaseSelectionMenuEntry selection)
	{
		
		string title = selection.GetEntryName();
		string description = selection.GetEntryDescription();
		
		// Title
		if (m_wTxtItemName) 
		{
			bool showTitle = title != string.Empty;
			m_wTxtItemName.SetVisible(showTitle);
			m_wTxtItemName.SetText(title);
		}	
		
		// Description of actions 
		if (m_wTxtAction)
		{
			bool showDesc = description != string.Empty;
			m_wTxtAction.SetVisible(showDesc);
			m_wTxtAction.SetText(description);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Showlast selected entry 
	protected void UpdateLastSelected(BaseSelectionMenuEntry selectedEntry, float angle)
	{
		m_pSelectedEntry = selectedEntry;
		if(m_pSelectedEntry)
		m_fLastSelectedAngles = angle;
		
		if(m_pSelectedEntry)
		{
			// show selected 
			SetVisibleSafe(m_wSelectedEntry, true);
			if(m_wSelectedEntry)
			{
				vector vec = GetPointOnCircle(m_fRadiusEntries, angle * Math.DEG2RAD);
				FrameSlot.SetPos(m_wSelectedEntry, vec[0], vec[1]);
				
				m_wSelectedEntry.SetRotation(angle);
			}
		}
		else
		{
			// hide if no selected 
			SetVisibleSafe(m_wSelectedEntry, false);
			return;
		}

		// Rotate selection indication widget 
		HandleWidgetSelected(m_wSelectLast, angle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Udpate last selection angle only by given id
	protected void UpdateLastSelectedById(int id)
	{	
		// Hide if selection is invalid 
		if (id < 0)
		{
			SetVisibleSafe(m_wSelectedEntry, false);
			return;
		}
	
		float angle = id * m_fEntryAngleDistance;
		HandleWidgetSelected(m_wSelectLast, angle);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetContent(array<BaseSelectionMenuEntry> allEntries, array<BaseSelectionMenuEntry> disabledEntries, bool clearData = false)
	{
		// clearing items 
		if (clearData)
			DestroyElementWidgets();
		
		for (int i = 0; i < m_aBorderLines.Count(); i++)
		{
			RemoveBorderLine(m_aBorderLines[i]);
		}
		
		// Adding items  
		int originalCount = m_pWidgetPairs.Count();
		int incomingCount = allEntries.Count();
		
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
			SCR_SelectionEntryWidgetComponent entryWidget = null;
			if (m_aEntryElements.Count() > i)
			{
				entryWidget = m_aEntryElements[i];
			}
			
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
					auto position = GetElementPosition(i, incomingCount, m_fRadiusEntries);
					SetElementPosition(widget, position);
					SetElementData(pair, canBePerformed, entryWidget);
					continue;
				}
			}
			
			// For widgets which do not exist,
			// create new ones
			ScriptedSelectionMenuEntry selectionEntry = ScriptedSelectionMenuEntry.Cast(currentEntry);
			
			Widget widget = CreateElementWidget(m_wRoot, selectionEntry);
			
			// Prepare pair
			ref auto pair = new ref SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry>(null, currentEntry);
			
			// If we created widget, create container with child widgets for it
			if (widget)
			{
				// Set pair value
				pair.m_pEntry = currentEntry;
				pair.m_pWidget = widget;
				
				auto position = GetElementPosition(i, incomingCount, m_fRadiusEntries);
				SetElementPosition(widget, position);
				SetElementData(pair, canBePerformed, entryWidget);
				currentEntry.GetEntryIconPath();
			}
			
			m_pWidgetPairs.AddEntry(pair);
			
			// Add lines between entries
			if (!m_rBorderLinePath.IsEmpty())
			{
				float angle = m_fEntryAngleDistance * i + m_fEntryAngleDistance/2;
				CreateBorderLine(m_wLayoutContent, angle, m_rBorderLinePath);
			}
		}
		
		if(originalCount != incomingCount)
		{
			// Update angle on change
			UpdateBorderLinesAngle();
			
			float sizeAngle = m_fEntryAngleDistance / 360 * 0.5;
			SetWidgetSelectSize(m_wSelectCurrent, sizeAngle);
			SetWidgetSelectSize(m_wSelectLast, sizeAngle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle internal display state
	void SetOpen(IEntity owner, bool open)
	{
		// Opening
		m_bIsOpen = open;

		if (m_bIsOpen)
		{
			OnOpen(owner);
		}
		else
		{
			OnClose(owner);
		}
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
		
		// Get Widgets 
		m_wLayoutWrap = m_wRoot.FindAnyWidget(m_sLayoutWrap);
		m_wLayoutBase = m_wRoot.FindAnyWidget(m_sLayoutBase);
		m_wLayoutContent = m_wRoot.FindAnyWidget(m_sLayoutContent);
		m_wLayoutInfo = m_wRoot.FindAnyWidget(m_sLayoutInfo);
		m_wSelectLast = m_wRoot.FindAnyWidget(m_sSelectLast);
		m_wSelectCurrent = m_wRoot.FindAnyWidget(m_sSelectCurrent);
		
		m_wTxtAction = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTxtAction));
		m_wTxtItemName = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTxtItemName));

		m_wImgSelector = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sImageSelector));
		
		m_wPages = m_wRoot.FindAnyWidget(WIDGET_PAGES);
		m_RichPageActionPrev = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_PAGE_ACTION_PREV));
		SetActionIconToWidget(m_RichPageActionPrev, SCR_RadialMenuInteractions.INPUT_PAGE_PREVIOUS);
		m_RichPageActionNext = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_PAGE_ACTION_NEXT));
		SetActionIconToWidget(m_RichPageActionNext, SCR_RadialMenuInteractions.INPUT_PAGE_NEXT);
	
		// Old widget TODO: Remove with editor visual change
		m_wSelectionTitle = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleText"));
		SetVisibleSafe(m_wSelectionTitle, false);
		
		m_wSelectionDescription = TextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionText"));
		SetVisibleSafe(m_wSelectionDescription, false);
		
		m_wSelector = ImageWidget.Cast(m_wRoot.FindAnyWidget("Selector"));
		SetVisibleSafe(m_wSelector, false);	
		
		SetVisibleSafe(m_wRoot, false);
		// Old widget---------------------------------------
		
		// Add pages 
		array<ref SCR_MenuPage> pages = m_RadialMenuHandler.GetSCR_MenuPages();
		int pageCount = pages.Count();
		
		if (pageCount > 1)
		{
			AddPages();
			HighlighPageIndicator(0, -1);
		}
		else
		{
			if (m_wPages)
				m_wPages.SetVisible(false);
		}
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
			DisplayStartDraw(owner);
		}
		
		// Inital opening - to avoid calls from start draw
		if(m_bInitialOpen)
		{	
			UpdateLastSelected(m_pSelectedEntry, m_fLastSelectedAngles);
			m_bInitialOpen = false;
		}
		
		SetVisibleSafe(m_wRoot, true);
		FadeAnimation(m_wRoot, 0, 1, m_fFadeInSpeed, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when menu close if requested. It shouldnt be mandatory to delete all UI elements,
	//! but the root element should be hidden during this event
	protected event void OnClose(IEntity owner)
	{
		SetVisibleSafe(m_wRoot, false);
		FadeAnimation(m_wRoot, 1, 0, m_fFadeOutSpeed, false);
		
		RemoveIcons();
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		m_bIsOpen = false;
		m_bInitialOpen = true;
		
		OnClose(owner);
		RemoveListeners();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Animation for changing opacity to faid in and out
	protected void FadeAnimation(Widget widget, float valueStart, float valueEnd, float speed, bool visible)
	{
		if(!widget)
			return;
		
		SetVisibleSafe(widget, true);
		
		widget.SetOpacity(valueStart);
		WidgetAnimator.PlayAnimation(widget, WidgetAnimationType.Opacity, valueEnd, speed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle both parts of widget indicating selection to poiting right angles 
	protected void HandleWidgetSelected(Widget parent, float angleSelection)
	{	
		if (!parent)
			return;
		
		// Find widget parts 
		ImageWidget left = ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_LEFT));
		ImageWidget right = ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_RIGHT));
		ImageWidget fill =  ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_FILL));
		
		if (!left || !right)
			return;
		
		// Move angles 
		left.SetRotation(angleSelection);
		right.SetRotation(angleSelection + 180);
		
		// Move fill 
		if (!fill)
			return;
		
		fill.SetRotation(angleSelection + 180);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set amount of angle selection
	protected void SetWidgetSelectSize(Widget parent, float angleSize)
	{
		if (!parent)
			return;
		
		// Find widget parts 
		ImageWidget left = ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_LEFT));
		ImageWidget right = ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_RIGHT));
		ImageWidget fill =  ImageWidget.Cast(parent.FindAnyWidget(WIDGET_SELECTED_PART_FILL));
		
		if (!left || !right)
			return;
		
		// Move angles 
		left.SetMaskProgress(1 - angleSize);
		right.SetMaskProgress(angleSize);
		
		left.SetMaskTransitionWidth(angleSize);
		right.SetMaskTransitionWidth(angleSize);
		
		// Set fill 
		if (!fill)
			return;
		
		//fill.SetMaskProgress(angleSize * 2);
		fill.SetMaskRange(angleSize);
		fill.SetMaskTransitionWidth(angleSize);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddPages()
	{
		// comp variables 		
		protected string m_sWidgetPageRoot = WIDGET_PAGE_ROOT;
		
		protected Widget m_wPageRoot;
		m_wPageRoot = m_wRoot.FindAnyWidget(m_sWidgetPageRoot);
		// comp variables 
		

		array<ref SCR_MenuPage> pages = m_RadialMenuHandler.GetSCR_MenuPages();
		int pageCount = pages.Count();
		
		for (int i = 0; i < pageCount; i++)
		{
			// Create widgets
			Widget wPage = GetGame().GetWorkspace().CreateWidgets(m_sPageIndicationLayout, m_wPageRoot);
			m_aPageWidgets.Insert(wPage);
			
			// Setting icon 
			ImageWidget wIcon = ImageWidget.Cast(wPage.FindAnyWidget(WIDGET_PAGE_ICON));
			string iconName = pages[i].GetIconName();
			
			bool visible = !m_sPageIconsSet.IsEmpty();
			wIcon.SetVisible(visible);
			
			if (wIcon && wIcon.IsVisible())
				wIcon.LoadImageFromSet(0, m_sPageIconsSet, iconName);
			
			// Inactivate opacity 
			wPage.SetOpacity(m_fPageInacativeOpacity); 
			
			// Padding
			if (i > 0)
				HorizontalLayoutSlot.SetPadding(wPage, m_iPagePadding, 0, 0, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Highlight page indicator on page switch 
	protected void HighlighPageIndicator(int pageId, int lastSelect)
	{
		for (int i = 0; i < m_aPageWidgets.Count(); i++)
		{
			Widget wPage = m_aPageWidgets[i];
			Widget wSelect = wPage.FindAnyWidget(WIDGET_PAGE_SELECT);
			
			if (!wSelect)
				continue;
			
			if (pageId == i)
			{
				// Active 
				wPage.SetOpacity(1);
				wSelect.SetOpacity(1);
				wSelect.SetColor(UIColors.CONTRAST_COLOR);
			}
			else
			{
				//Inactive 
				wPage.SetOpacity(m_fPageInacativeOpacity);
				wSelect.SetOpacity(m_fPageInacativeOpacity);
				wSelect.SetColor(Color.White);
			}
		}
		
		// Update last selection 
		UpdateLastSelectedById(lastSelect);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActionIconToWidget(RichTextWidget actionWidget, string action)
	{
		if (!actionWidget)
			return; 
		
		actionWidget.SetText(string.Format("<action name='%1' scale='1.25'/>", action));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Insert new entry widget into entry widget setup array  
	void InsertEntryWidget(SCR_SelectionEntryWidgetComponent entryWidget)
	{
		m_aEntryElements.Insert(entryWidget);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveIcons()
	{
		foreach (SCR_SelectionEntryWidgetComponent entry : m_aEntryElements)
		{
			entry.GetRootWidget().RemoveFromHierarchy();
		}
		
		m_aEntryElements.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuVisuals(IEntity owner) {}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuVisuals()
	{
		// Remove listeners
		RemoveListeners();
		if (m_RadialMenuHandler)
		m_RadialMenuHandler.onMenuToggleInvoker.Remove(SetOpen);
		
		OnDestroy(null);
	}
};