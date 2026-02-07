//------------------------------------------------------------------------------------------------
/*!
Radial menu class for handling HUD part of menu.
Displays menu circle and distributes entries into the circle
*/
[BaseContainerProps()]
class SCR_RadialMenuDisplay : SCR_SelectionMenuDisplay
{		
	protected const string HINT_BASE = "<action name='%1'/>";
	
	[Attribute("150", UIWidgets.Slider, "Entries distance from menu center in pixels", "0 1000 1")]
	protected int m_iEntriesRadius;
	
	[Attribute("0")]
	protected float m_fSelectedIndicatorOffset;
	
	// Widget name refs 
	[Attribute("")]
	protected string m_sBaseWidget;
	
	[Attribute("1", desc: "Adjust base size in callback based on invoker from SCR_RadialMenu")]
	protected bool m_bForceSize;
	
	[Attribute("")]
	protected string m_sDividersParent;
	
	[Attribute("", ".edds", desc: "Texture used for creating dividing lines")]
	protected ResourceName m_sDividerTexture;
	
	[Attribute("0", UIWidgets.Slider, "Dividers distance from menu center in pixels", "0 1000 1")]
	protected int m_iDividersRadius;
	
	[Attribute("120")]
	protected int m_iDividerSize;
	
	[Attribute("1", UIWidgets.Slider, "Dividers texture opacity", "0 1 0.01")]
	protected float m_fDividersOpacity;
	
	[Attribute("")]
	protected string m_sSegmentsParent;
	
	[Attribute("", ".layout", desc: "Layout used for creating segment. Serves  for entry split and colorize")]
	protected ResourceName m_sSegmentLayout;
	
	[Attribute("")]
	protected string m_sSelectorArrow;
	
	[Attribute("")]
	protected string m_sSelectorLine;
	
	[Attribute("")]
	protected string m_sSelectedIndicator;
	
	[Attribute("")]
	protected string m_sSelectedName;
	
	[Attribute("")]
	protected string m_sActionHint;
	
	[Attribute("")]
	protected string m_sDescription;
	
	protected Widget m_wBase;
	protected Widget m_wDividersParent;
	protected ref array<Widget> m_aDividers = {};
	protected Widget m_wSegmentsParent;
	protected ref array<Widget> m_aSegments = {};
	
	protected ImageWidget m_wSelectorArrow;
	protected ImageWidget m_wSelectorLine;
	protected ImageWidget m_wSelectedIndicator;
	protected TextWidget m_wSelectedName;
	protected TextWidget m_wActionHint;
	protected TextWidget m_wDescription;
	
	protected SCR_RadialMenu m_RadialMenu;
	
	protected float m_fSelectorArrowRadius;
	
	// Values to dynamic
	protected float m_fDynamicEntriesRadius;
	protected float m_fDynamicDividerSize;
	protected float m_fDynamicDividersRadius;

	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Setup menu		
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		
		// Find widgets
		m_wBase = GetRootWidget().FindAnyWidget(m_sBaseWidget);
		m_wDividersParent = GetRootWidget().FindAnyWidget(m_sDividersParent);
		m_wSegmentsParent = GetRootWidget().FindAnyWidget(m_sSegmentsParent);
		
		// Selection arrow is FrameWidget with ImageWidget child, Frame changes position and Image rotation 
		m_wSelectorArrow = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectorArrow));
		m_wSelectorLine = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectorLine));
		m_wSelectedIndicator = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectedIndicator));
		
		m_wSelectedName = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectedName));
		m_wDescription = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sDescription));
		
		m_wActionHint = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sActionHint));
		
		// Setup menu 
		m_RadialMenu = SCR_RadialMenu.Cast(m_Menu);
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnDisplaySizeChange().Insert(OnDisplaySizeChange);
			m_RadialMenu.GetOnSetActionHint().Insert(OnSetActionHint);
		}
		
		if (m_wSelectedIndicator)
			m_wSelectedIndicator.SetVisible(false);
		
		if (m_wDescription)
			m_wDescription.SetVisible(false);
		
		if (m_wActionHint)
			m_wActionHint.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);
		
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnDisplaySizeChange().Remove(OnDisplaySizeChange);
			m_RadialMenu.GetOnSetActionHint().Remove(OnSetActionHint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (m_bShown)
			VisualizeSelection(m_RadialMenu.GetPointingAngle());
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuOpen()
	{
		// Setup menu 
		if (m_RadialMenu && m_wBase)
		{
			// Screen pos
			float posX, posY;
			m_wBase.GetScreenPos(posX, posY);
			
			float sizeX, sizeY;
			m_wBase.GetScreenSize(sizeX, sizeY);
			
			m_RadialMenu.SetMenuCenterPos(Vector(posX + sizeX * 0.5, posY + sizeY * 0.5, 0));
		}
		
		super.OnMenuOpen();
		SetupSelectionVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuEntriesUpdate(SCR_SelectionMenu menu, array<ref SCR_SelectionMenuEntry> entries)
	{
		super.OnMenuEntriesUpdate(menu, entries);
		
		SetupSelectionVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void FindMenu()
	{
		// Find radial menu component in game mode
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_RadialMenuGameModeComponent rmComp = SCR_RadialMenuGameModeComponent.Cast(
			gameMode.FindComponent(SCR_RadialMenuGameModeComponent));
		
		if (rmComp)
			m_Menu = rmComp.GetMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void SetupEntryWidget(notnull SCR_SelectionMenuEntry entry, notnull Widget widget, int id)
	{
		SetupFrameSlotPosition(widget, id, m_fDynamicEntriesRadius, m_RadialMenu.GetEntriesAngleDistance());

		// Set icon size  
		SCR_SelectionMenuEntryIconComponent iconEntryCmp = SCR_SelectionMenuEntryIconComponent.Cast(
			widget.FindHandler(SCR_SelectionMenuEntryIconComponent));
		
		if (iconEntryCmp)
		{
			float entrySize = iconEntryCmp.GetOriginalSize() * m_SizeRatio;
			iconEntryCmp.SetLayoutSize(entrySize);
		}
				
		// Add elements for entry 
		CreateEntrySegment(entry, widget, id)
	}
	
	//------------------------------------------------------------------------------------------------
	//! React on selected entry change
	override protected void OnMenuEntrySelected(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry, int id) 
	{
		// Selection
		if (m_wSelectedIndicator)
		{
			m_wSelectedIndicator.SetVisible(entry != null && entry.IsEnabled());
				
			float angle = m_RadialMenu.GetEntriesAngleDistance() * id;
			int offset = m_fSelectedIndicatorOffset - m_RadialMenu.GetEntriesAngleDistance() * 0.5;
			m_wSelectedIndicator.SetRotation(angle + offset);
		}
		
		// Info
		if (m_wSelectedName)
		{
			m_wSelectedName.SetVisible(entry != null);
			
			if (entry)
				m_wSelectedName.SetText(entry.GetName());
		}
		
		// Visualize segments selection 
		if (m_aSegments.IsIndexValid(m_iLastSelectedId))
		{
			if (m_LastSelectedEntry && m_LastSelectedEntry.IsEnabled())
				m_aSegments[m_iLastSelectedId].SetVisible(true);
		}
		
		if (m_aSegments.IsIndexValid(id))
			m_aSegments[id].SetVisible(false);
		
		// Update selection 
		m_LastSelectedEntry = entry;
		m_iLastSelectedId = id;
		
		// Display description 
		if (m_wDescription)
		{	
			m_wDescription.SetVisible(entry && entry.GetDescription() != string.Empty);
			
			if (entry)
				m_wDescription.SetText(entry.GetDescription());
		}
		
		// Display action hint
		if (m_wActionHint)
			m_wActionHint.SetVisible(entry && m_wActionHint.GetText() != string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear segments and lines
	override protected void ClearEntryWidgets()
	{
		super.ClearEntryWidgets();
			
		if (m_aSegments.Count() != m_aDividers.Count())
		{
			DebugPrint("ClearEntryWidgets", "Different entries");
		}
		
		// Clear segments and dividers
		for (int i = 0, count = m_aSegments.Count(); i < count; i++)
		{
			m_aSegments[i].RemoveFromHierarchy();
			m_aDividers[i].RemoveFromHierarchy();
		}
		
		m_aSegments.Clear();
		m_aDividers.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Vector X Y positions from radisu (distance) and degrees (angle)
	protected vector GetPointOnCircle(float radius, float degrees)
	{
		// - half PI because 90 deg offset left
		return Vector(
			radius * Math.Cos(degrees - 0.5 * Math.PI), 
			radius * Math.Sin(degrees - 0.5 * Math.PI), 
			0.0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VisualizeSelection(float selectionAngle)
	{
		// Check widgets 
		bool useMouse = GetGame().GetInputManager().IsUsingMouseAndKeyboard();
		
		if (m_wSelectorArrow)
			m_wSelectorArrow.SetVisible(!useMouse && !m_RadialMenu.IsPointingToCenter());
		
		m_wSelectorLine.SetVisible(!m_RadialMenu.IsPointingToCenter());
		m_wSelectorLine.SetRotation(selectionAngle - 180);
		
		// Mouse 
		if (useMouse)
		{
			return;
		}
		
		// Gamepad 
		if (!m_wSelectorArrow)
			return;
		
		if (m_fSelectorArrowRadius == 0 && m_wSelectorArrow)
			m_fSelectorArrowRadius = FrameSlot.GetPosY(m_wSelectorArrow);

		if (m_fSelectorArrowRadius == 0)
			return;
		
		// Check center 
		vector vec = GetPointOnCircle(-m_fSelectorArrowRadius, selectionAngle * Math.DEG2RAD);
		FrameSlot.SetPos(m_wSelectorArrow, vec[0], vec[1]);
		
		m_wSelectorArrow.SetRotation(selectionAngle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup widget position and angle in frame slot 
	protected float SetupFrameSlotPosition(out notnull Widget widget, int id, float distance, float angle, float angleOffset = 0)
	{
		// Calculate
		float degs = angle * id + angleOffset;
		vector point = GetPointOnCircle(distance, degs * Math.DEG2RAD);
		
		// Set position - set entry layout to menu center to more easily place entry around the circle
		FrameSlot.SetPos(widget, point[0], point[1]);
		FrameSlot.SetAlignment(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMin(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMax(widget, 0.5, 0.5);
		
		return degs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add to circle segments filling entry area
	protected void CreateEntrySegment(notnull SCR_SelectionMenuEntry entry, notnull Widget widget, int id)
	{	
		if (!m_wSegmentsParent || m_sSegmentLayout.IsEmpty())
		{
			DebugPrint("CreateEntrySegment", "Can't create segments!");
			return;
		}
		
		ImageWidget segment = ImageWidget.Cast(
			GetGame().GetWorkspace().CreateWidgets(m_sSegmentLayout, m_wSegmentsParent));
		
		if (!segment)
			return;
		
		// Register 
		m_aSegments.Insert(segment);
		
		float angleDist = m_RadialMenu.GetEntriesAngleDistance();
		float range = angleDist / 360;
		segment.SetMaskProgress(range);
		// distance * list position - half -> to be in entry center
		float rot = angleDist * id - angleDist * 0.5;
		segment.SetRotation(rot);
		
		// Add divider
		if (m_wDividersParent && !m_sDividerTexture.IsEmpty())
		{
			CreateDivider(id);
		}
		
		SCR_SelectionMenuEntryComponent comp = SCR_SelectionMenuEntryComponent.Cast(
			widget.FindHandler(SCR_SelectionMenuEntryComponent));
		
		// Visual setup 
		VisualizeEnableEntry(comp, id, entry.IsEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add divider after entry segments
	protected void CreateDivider(int id)
	{
		if (!m_wDividersParent || m_sDividerTexture.IsEmpty())
		{
			DebugPrint("CreateDivider", "Can't create segments!");
			return;
		}
		
		Widget divider = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, 
			WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.BLEND | WidgetFlags.INHERIT_CLIPPING,
			Color.White,
			0,
			m_wDividersParent
		);
		
		// Setup divider widget
		ImageWidget imgDivider = ImageWidget.Cast(divider);
		
		imgDivider.LoadImageTexture(0, m_sDividerTexture);
		imgDivider.SetOpacity(m_fDividersOpacity);
		FrameSlot.SetSize(divider, m_fDynamicDividerSize, m_fDynamicDividerSize);
		
		float angleDist = m_RadialMenu.GetEntriesAngleDistance();
		float angle = SetupFrameSlotPosition(divider, id, m_fDynamicDividersRadius, angleDist, angleDist * 0.5);

		imgDivider.SetRotation(angle);
		
		// Register
		m_aDividers.Insert(divider)
	}
	
	//------------------------------------------------------------------------------------------------
	void VisualizeEnableEntry(notnull SCR_SelectionMenuEntryComponent entry, int id, bool enable)
	{
		if (!m_aSegments.IsIndexValid(id))
			return;
		
		m_aSegments[id].SetVisible(enable);
		entry.SetEnabled(enable);
	}
	
	protected float m_SizeRatio;
	
	//------------------------------------------------------------------------------------------------
	//! Callback reacting to changing size from SCR_RadialMenu 
	protected void OnDisplaySizeChange(SCR_RadialMenu menu, float size)
	{
		// Ratio to menu large size
		m_SizeRatio = 1;
		
		if (m_bForceSize)
			m_SizeRatio = size / menu.SIZE_LARGE;
		
		// Setup sizes
		m_fDynamicEntriesRadius = m_iEntriesRadius * m_SizeRatio;
		m_fDynamicDividerSize = m_iDividerSize * m_SizeRatio;
		m_fDynamicDividersRadius = m_iDividersRadius * m_SizeRatio;
		
		if (m_bForceSize)
			FrameSlot.SetSize(m_wBase, size, size);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback to display action hint icon
	protected void OnSetActionHint(SCR_RadialMenu menu, string action)
	{
		if (!m_wActionHint)
			return;
		
		// Check if action has binding
		InputBinding input = GetGame().GetInputManager().CreateUserBinding();
		array<string> bindings = {};
		 
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			// Keyboard and mouse
			input.GetBindings(action, bindings, EInputDeviceType.KEYBOARD);
			if (bindings.IsEmpty())
				input.GetBindings(action, bindings, EInputDeviceType.MOUSE);
		}
		else
		{
			// Gamepad 
			input.GetBindings(action, bindings, EInputDeviceType.GAMEPAD); 
		}
		
		// Setup widget
		if (!action.IsEmpty() && !bindings.IsEmpty())
			m_wActionHint.SetTextFormat(HINT_BASE, action);
		else
			m_wActionHint.SetTextFormat(string.Empty);
		
		m_wActionHint.SetVisible(m_RadialMenu.GetSelectionEntry() && m_wActionHint.GetText() != string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSelectionVisuals()
	{
		if (!m_wSelectedIndicator || !m_wSelectorLine)
			return;
		
		// Set selection segment from angles to percents
		float range = m_RadialMenu.GetEntriesAngleDistance() / 360;
		
		m_wSelectedIndicator.SetMaskProgress(range);
		
		m_wSelectorLine.SetMaskRange(range);
		m_wSelectorLine.SetMaskTransitionWidth(range * 0.5);
	}
};
