//------------------------------------------------------------------------------------------------
//! Menu page that can contain multiple entries 
class SCR_MenuPage : ScriptedSelectionMenu
{	
	const static string PAGENAME_DEFAULT = "Page";
		
	protected string m_sName;
	protected string m_sIconName;
	
	protected int m_iLastSelected = -1;
	
	//------------------------------------------------------------------------------------------------
	void SCR_MenuPage(string name = PAGENAME_DEFAULT)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MenuPage() {}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name) { m_sName = name; }
	
	//------------------------------------------------------------------------------------------------
	string GetName() { return m_sName; }
	
	//------------------------------------------------------------------------------------------------
	void SetIconName(string iconName) { m_sIconName = iconName; }
	
	//------------------------------------------------------------------------------------------------
	string GetIconName() { return m_sIconName; }
	
	//------------------------------------------------------------------------------------------------
	void SetLastSelected(int lastSelected) { m_iLastSelected = lastSelected; }
	
	//------------------------------------------------------------------------------------------------
	int GetLastSelected() { return m_iLastSelected; } 
};

//------------------------------------------------------------------------------------------------
//! Base class for radial menu
class SCR_RadialMenuHandler : ScriptedSelectionMenu
{		
	[Attribute("0.45", UIWidgets.EditBox, "Minimal amout to point in direction to select item. Anything less will reuslt in null selectiom.")]
	protected float m_fSelectInputRadiusMin;
	
	[Attribute(UISounds.FOCUS, UIWidgets.EditBox)]
	protected string m_sSoundOnOpen;
	
	[Attribute("SOUND_E_TRAN_CANCEL", UIWidgets.EditBox)]
	protected string m_sSoundOnClose;

	protected IEntity m_pOwner;
	
	//! Source of data 
	protected IEntity m_pSource;
	
	//! Reference on interaction and input handling
	ref SCR_RadialMenuInteractions m_pRadialMenuInteractions;
	protected SCR_RadialMenuVisuals m_RadialMenuVisuals;
	
	//! Last selected element or null if none
	protected ref BaseSelectionMenuEntry m_pCurrentSelection;
	
	protected ref BaseSelectionMenuEntry m_pLastSelected;
	
	protected ref SCR_RadialMenuFilter m_pFilter = new SCR_RadialMenuFilter();

	protected ref array<ref SCR_MenuPage> m_aSCR_MenuPages = new ref array<ref SCR_MenuPage>;

	protected int m_iActivePage;
	protected ref ScriptedSelectionMenuEntry m_ActiveGroupEntry;
	
	//! Current input in angle
	protected float m_fSelectorAngle;
	
	//! Angle of current selescted entry
	protected float m_fSelectedAngle;
	
	// angle of previuous selection
	protected float m_fLastSelectedAngle;
	
	//! should selector be visible
	protected bool m_bShowSelector;
	
	//! Selecting behavior setup
	protected ERadialMenuSelectionBehavior m_iSelectionBehavior;
	
	//! Modify if placement of entry is by fixed position or evenly by entries count
	protected bool m_bEvenlyPlacedEntries;
	
	//! Distance angle in degrees between each entry
	protected float m_fEntryDistance;
	
	//! Offset of entries initial position in degrees
	protected float m_fEntryInitialOffset;
	
	//! emtpy place between each entry
	protected float m_fEntryOffset;
	
	//! 
	protected float m_fMouseRadius;
	
	//! Alow to show entries that has no data
	protected bool m_bShowEmptyEntries;
	
	// Free selection delay
	protected bool m_bFreeDelayRelease;
	protected float m_fFreeDelay = 0.25;
	protected float m_fFreeDelayTimer;
	
	//! Script invokers
	ref ScriptInvoker<IEntity, bool> onMenuToggleInvoker;
	
	ref ScriptInvoker<float, float> onPositionChangeInvoker;
	
	ref ScriptInvoker<BaseSelectionMenuEntry, float> onSelectedEntryChangeInvoker;
	
	ref ScriptInvoker<bool, bool, float, float, BaseSelectionMenuEntry> onSelectionUpdateInvoker;
	
	ref ScriptInvoker<array<BaseSelectionMenuEntry>, array<BaseSelectionMenuEntry>> m_OnEntriesUpdate;
	
	ref ScriptInvoker<BaseSelectionMenuEntry, int> m_OnActionPerformed = new ref ScriptInvoker<BaseSelectionMenuEntry, int>;
	
	ref ScriptInvoker m_OnCreated = new ref ScriptInvoker;
	ref ScriptInvoker m_OnUpdate = new ref ScriptInvoker;
	
	// Page invokers 
	ref ScriptInvoker m_OnPageSwitch = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectionUpdate(bool showSelector, bool performable, float selectorAngle,  float selectedAngle, BaseSelectionMenuEntry selectedEntry)
	{
		if(onSelectionUpdateInvoker)
			onSelectionUpdateInvoker.Invoke(showSelector, performable, selectorAngle, selectedAngle, selectedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectedEntryChange(BaseSelectionMenuEntry selectedEntry, float angle)
	{
		if(onSelectedEntryChangeInvoker)
			onSelectedEntryChangeInvoker.Invoke(selectedEntry, angle);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntriesUpdate(array<BaseSelectionMenuEntry> enableEntries, array<BaseSelectionMenuEntry> disableEntries)
	{
		m_OnCreated.Invoke();
		
		if(m_OnEntriesUpdate)
			m_OnEntriesUpdate.Invoke(enableEntries, disableEntries, true);
		
		m_OnUpdate.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPositionChange(float distanceAngle, float offsetAngle)
	{
		if(onPositionChangeInvoker)
			onPositionChangeInvoker.Invoke(distanceAngle, offsetAngle);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenutoggle(IEntity owner, bool isOpen)
	{
		if(onMenuToggleInvoker)
			onMenuToggleInvoker.Invoke(owner, isOpen);
		
		if (isOpen)
			SCR_UISoundEntity.SoundEvent(m_sSoundOnOpen, true);
		else if (!m_pCurrentSelection)
			SCR_UISoundEntity.SoundEvent(m_sSoundOnClose);	
	}

	//------------------------------------------------------------------------------------------------
	SCR_RadialMenuInteractions GetRadialMenuInteraction() { return m_pRadialMenuInteractions; }
	
	//------------------------------------------------------------------------------------------------
	SCR_RadialMenuVisuals GetRadialMenuVisuals() { return m_RadialMenuVisuals; }
	
	//------------------------------------------------------------------------------------------------
	void SetRadialMenuVisuals(SCR_RadialMenuVisuals visuals) { m_RadialMenuVisuals = visuals; }
	
	//------------------------------------------------------------------------------------------------
	void SetSelectFreeDelay(float delay) { m_fFreeDelay = delay; }
	
	//------------------------------------------------------------------------------------------------
	void SetEvenlyPlacedEntries(bool isTrue)
	{
		m_bEvenlyPlacedEntries = isTrue;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryDistance(float angle)
	{
		m_fEntryDistance = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryOffset(float angleInitial, float angle)
	{
		m_fEntryInitialOffset = angleInitial;
		m_fEntryOffset = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetShowEmptyEntries(bool show)
	{
		m_bShowEmptyEntries = show;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadialMenuSelectionBehavior(ERadialMenuSelectionBehavior type)
	{
		m_iSelectionBehavior = type;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetSource()
	{
		return m_pSource;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MenuPage> GetSCR_MenuPages() { return m_aSCR_MenuPages; }
	
	//------------------------------------------------------------------------------------------------
	protected float GetClampedAngle(float x, float y, int elementCount)
	{
		if (elementCount == 0)
			return 0.0;

		float angle = Math.Atan2(x,y) + m_fEntryInitialOffset * Math.DEG2RAD;
		float step = m_fEntryDistance * Math.DEG2RAD;
		float as = angle/step;
		float reg = Math.Round(as);

		return reg * step;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Will translate radian format into degrees in range 0-360 
	protected float RadTo360Deg(float angle)
	{
		angle = angle * Math.RAD2DEG;
		if(angle < 0)
			return 360 + angle;
		
		return angle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all entries that can be performed
	protected ref array<BaseSelectionMenuEntry> PerformableEntries(IEntity owner)
	{
		ref array<BaseSelectionMenuEntry> performable = new array<BaseSelectionMenuEntry>();
			
		for (int i = 0; i < m_pFilter.m_aAllEntries.Count(); i++)
		{
			auto currentEntry = m_pFilter.m_aAllEntries[i];
			
			// Select only perfomable entry
			if (currentEntry && currentEntry.GetUIInfo())
			{
				performable.Insert(currentEntry);
			}
		}
		
		return performable;
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner)
	{	
		AddListeners();
		
		m_pOwner = owner;
		
		// Init other radial menu functions
		if(!m_pRadialMenuInteractions)
			return;

		// Initial setting of entry distances and offset - mainly for static distance
		OnPositionChange(m_fEntryDistance, m_fEntryInitialOffset);
		PageSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PageSetup()
	{
		// Add initial group 
		m_aSCR_MenuPages.Clear();
		
		ref SCR_MenuPage pageInital = new ref SCR_MenuPage();
		m_aSCR_MenuPages.Insert(pageInital);
		m_iActivePage = 0;	
		SetPage(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback when open is requested
	protected override event void OnOpen(IEntity owner)
	{		
		// Call menu opening
		OnMenutoggle(owner, true);
		
		super.OnOpen(owner);
		
		UpdateEntries();
		
		// Reseting free selection delay 
		m_bFreeDelayRelease = false;
		m_fFreeDelayTimer = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add entry to given page 
	void AddEntry(BaseSelectionMenuEntry entry, int page)
	{
		if (page < 0)
			return;
		
		if (m_aSCR_MenuPages.Count() > page)
		{
			m_aSCR_MenuPages[page].AddEntry(entry);
		}
		else
		{
			Print("Radial Menu does not have a default page!", LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove all entries on given page 
	void ClearEntries(int page)
	{
		if (page < 0)
			return;
		
		if (m_aSCR_MenuPages.Count() > page)
		{
			m_aSCR_MenuPages[page].ClearEntries();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update data and show actual entries 
	protected void UpdateEntries()
	{
		ref array<BaseSelectionMenuEntry> enabledEntries = new ref array<BaseSelectionMenuEntry>;
		
		if (m_aSCR_MenuPages.Count() > 0)
			m_aSCR_MenuPages[m_iActivePage].GetEntryList(enabledEntries);	
		
		// Register currently active entries as menu official entries 
		ClearEntries();
		foreach (BaseSelectionMenuEntry entry : enabledEntries)
		{
			AddEntry(entry);
		}

		// Send to display 
		SetEntriesToDisplay(m_pSource);
		SendEntriesDataToVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup data to menu 
	protected void SetEntriesToDisplay(IEntity owner)
	{	
		// Fitler currently selected menu entries
		if (m_aSCR_MenuPages.Count() < 1)
			return;
	
		array<BaseSelectionMenuEntry> entriesCurrent = new array<BaseSelectionMenuEntry>;
		m_aSCR_MenuPages[m_iActivePage].GetEntryList(entriesCurrent);

		if (m_pFilter)
			m_pFilter.DoFilter(owner, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when close is requested
	protected override event void OnClose(IEntity owner)
	{
		OnMenutoggle(owner, false);
		
		m_bShowSelector = false;
		m_pCurrentSelection = null;
		
		super.OnClose(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event void OnUpdate(IEntity owner, float timeSlice)
	{		
		if (!IsOpen())
			return;
		
		// Selecting actions
		m_pRadialMenuInteractions.PerformInteractions(owner);
		
		//Handle free pointer delay
		if (m_bFreeDelayRelease)
		{
			if (m_fFreeDelayTimer < m_fFreeDelay)
				m_fFreeDelayTimer += timeSlice;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//1 Setup important entries data for visualization and send invoke
	protected void SendEntriesDataToVisuals()
	{
		int elementsCount = GetEntriesCount();
		
		// Behavior for evenly spaced entries
		if(m_bEvenlyPlacedEntries && elementsCount > 0)
		{
			m_fEntryDistance = 360 / elementsCount;
			OnPositionChange(m_fEntryDistance, m_fEntryInitialOffset);
			SetLastSelectedEntry();
		}

		// Send entries update 
		array<BaseSelectionMenuEntry> entriesCurrent = new array<BaseSelectionMenuEntry>;
		if (m_aSCR_MenuPages.Count() > m_iActivePage)
			m_aSCR_MenuPages[m_iActivePage].GetEntryList(entriesCurrent);		
		
		OnEntriesUpdate(entriesCurrent, m_pFilter.m_aDisabledEntries);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RecieveVisualSetting(IEntity owner, float radius)
	{
		m_fMouseRadius = radius;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reaction on input for toggling menu
	protected void OpenMenu(IEntity owner, bool isOpen)
	{
		if (owner != m_pOwner)
			return;
		
		if(!isOpen)
		{
			Close(owner);
		}
		else
		{
			Open(owner);
		}

	}

	//------------------------------------------------------------------------------------------------
	//! Update data for each entry
	protected void UpdateEntriesData(IEntity owner) {}

	//------------------------------------------------------------------------------------------------
	//! Fill single entry that can be modified based on its type 
	protected void FillEntry(IEntity owner, BaseSelectionMenuEntry entry, int index) {}
	
	//------------------------------------------------------------------------------------------------
	//! Handle page data update 
	protected void UpdatePage(int pageId, array<BaseSelectionMenuEntry> entries)
	{
		SCR_MenuPage page = m_aSCR_MenuPages[pageId];
		int entriesCount = entries.Count();
		
		// Clear page entry and fill them again 
		page.ClearEntries();
		
		for (int i = 0; i < entriesCount; i++)
		{
			page.AddEntry(entries[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call for performing entry
	protected void TryPerformEntry(IEntity owner)
	{
		if (m_pCurrentSelection && m_pCurrentSelection.CanBePerformed(m_pOwner, this))
		{
			// Perform entry and set as last selected 
			m_pCurrentSelection.Perform(m_pSource, this);
			SetLastSelection(m_pCurrentSelection, m_iActivePage);
			
			array<BaseSelectionMenuEntry> entries = {};
			GetEntryList(entries);
			
			int i = entries.Find(m_pCurrentSelection);
			m_OnActionPerformed.Invoke(m_pCurrentSelection, i);
			
			// Audio
			SCR_UISoundEntity.SoundEvent(UISounds.ITEM_CONFIRMED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set selection id in selected menu page 
	protected void SetLastSelection(BaseSelectionMenuEntry entrySelected, int page)
	{
		if (m_aSCR_MenuPages.Count() < page)
			return;
		
		// Get page entries 
		array<BaseSelectionMenuEntry> entries = new array<BaseSelectionMenuEntry>;
		 m_aSCR_MenuPages[page].GetEntryList(entries);
	
		// Find id of given entry
		int id = -1;
		 
		for (int i = 0; i < entries.Count(); i++)
		{
			if (entries[i] == entrySelected)
			{
				id = i;
				m_aSCR_MenuPages[page].SetLastSelected(id);
				return;
			}
		}
		
		m_aSCR_MenuPages[page].SetLastSelected(id);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return id position of given entry if this entry is in filter
	protected int IDFromFilter(BaseSelectionMenuEntry checkEntry)
	{
		int id = 0;
		
		array<BaseSelectionMenuEntry> entries = new array<BaseSelectionMenuEntry>;
		GetEntryList(entries);
		
		foreach(BaseSelectionMenuEntry entry : entries)
		{
			if(checkEntry == entry)
				return id;
			
			id++;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update selection and invoke selection update callback
	protected void SelectionUpdate(IEntity owner, vector direction, float angle, bool isMouseInput)
	{
		SelectEntry(owner, direction, angle, isMouseInput);
		
		bool performable = false;
		
		if (m_pCurrentSelection)
			performable = m_pCurrentSelection.CanBePerformed(m_pOwner, this);
		
		// Updating selection
		OnSelectionUpdate(m_bShowSelector, performable, RadTo360Deg(m_fSelectorAngle), RadTo360Deg(m_fSelectedAngle), m_pCurrentSelection);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Move selector and set selected entry by given direction and angle
	//! Direction for center check, angle for selection 
	protected void SelectEntry(IEntity owner, vector direction, float angle, bool isMouseInput)
	{
		//int elementsCount = m_aEnabledEntries.Count();
		int elementsCount = GetEntriesCount();
		
		array<BaseSelectionMenuEntry> entriesCurrent = new array<BaseSelectionMenuEntry>;
		if (m_aSCR_MenuPages.Count() > m_iActivePage)
			m_aSCR_MenuPages[m_iActivePage].GetEntryList(entriesCurrent);	
		
		// Get selection index
		int selection = GetSelectedElementIndex(angle, elementsCount);
		
		// Update selector pointing angle
		if(!IsSelectorInCenter(direction))
		{
			m_fSelectorAngle = angle;
		}
		
		// Select entry
		BaseSelectionMenuEntry selectedElement;	
		
		if (selection == -1)
			selectedElement = null;
		else if (selection < elementsCount)
			selectedElement = entriesCurrent[selection];
		
		// Update selction by selection behavior
		if(!IsSelectorInCenter(direction))
			m_bShowSelector = true;
		
		bool validSelection = false;
		
		// Appply selection behavior 
		switch (m_iSelectionBehavior)
		{
			// Free
			case ERadialMenuSelectionBehavior.FreeMove:
			validSelection = SelectionBehaviorFree(selectedElement, direction);
			break; 
			
			// Limited 
			case ERadialMenuSelectionBehavior.StickToLastSection:
			validSelection = SelectionBehaviorWithinEntries(selectedElement, direction);
			break;
			
			// Sticky 
			case ERadialMenuSelectionBehavior.StickToLastSection:
			validSelection = SelectionBehaviorSticky(selectedElement, direction);
			break;
		}
		
		if (!validSelection)
			return;
		
		
		// Update selected entry
		m_fSelectedAngle = selection * m_fEntryDistance + m_fEntryInitialOffset;
		m_fSelectedAngle *= Math.DEG2RAD;
		
		if(m_pCurrentSelection != selectedElement)
		{
			m_pCurrentSelection = selectedElement;
			SCR_UISoundEntity.SoundEvent(UISounds.ITEM_SELECTED);	
		}
	}
	
	// SELECTOR BEHAVIORS 
	
	//------------------------------------------------------------------------------------------------
	//!	Free selection - with cancel selection in center 
	protected bool SelectionBehaviorFree(BaseSelectionMenuEntry selectedElement, vector direction)
	{
		if(!selectedElement)
			return false;	
		
		bool centered = IsSelectorInCenter(direction);
		
		// Pointing to entry that can be performed 
		if(!selectedElement.CanBePerformed(m_pOwner, this) && !centered)
		{
			m_bShowSelector = true;
		}
		
		// Cancel selection later when input close to center 
		if(centered)
		{
			if (!m_bFreeDelayRelease)
			{
				m_bFreeDelayRelease = true;
				m_fFreeDelayTimer = 0;
			}
			else
			{
				if (m_fFreeDelayTimer >= m_fFreeDelay) 
				CancelSelected(direction);
			}
			
			return false;
		}	
		
		// Preventing free delay canceling
		m_bFreeDelayRelease = false;
		m_fFreeDelayTimer = 0;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cancel current selection
	//! Call this with delay to prevent instant canceling 
	protected void CancelSelected(vector direction)
	{
		m_bFreeDelayRelease = false;
		m_fFreeDelayTimer = 0;
		
		if(!IsSelectorInCenter(direction))
			return;
		
		m_bShowSelector = false;
		m_pCurrentSelection = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Limited moving only withing angle with entries 
	protected bool SelectionBehaviorWithinEntries(BaseSelectionMenuEntry selectedElement, vector direction)
	{
		if(IsSelectorInCenter(direction))
		{
			return false;
		}
		
		if(!selectedElement)
			return false;	
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Free sticky selection - with alway picking last selection 
	protected bool SelectionBehaviorSticky(BaseSelectionMenuEntry selectedElement, vector direction)
	{
		if(IsSelectorInCenter(direction))
			return false;
			
		if(!selectedElement)
			return false;		
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! check if input / mouse cursor is in center of radial menu
	protected bool IsSelectorInCenter(vector input)
	{
		bool res = float.AlmostEqual(0, input[0], m_fSelectInputRadiusMin) && float.AlmostEqual(0, input[1], m_fSelectInputRadiusMin);
		
		return res;
	}
	
	//------------------------------------------------------------------------------------------------
	//! check if input / mouse cursor within range
	protected bool IsSelectorWithinRange(vector input, float min, float max)
	{
		float dist = vector.Distance(vector.Zero, input);
		return min < dist && dist < max;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetSelectedElementIndex(float angle, int elementCount)
	{
		if (elementCount == 0)
			return -1;

		float maxAngle = m_fEntryDistance * (elementCount - 1) + m_fEntryInitialOffset;
		float angleDeg = RadTo360Deg(angle) - m_fEntryInitialOffset;
		float overflow = m_fEntryDistance / 2;
		
		// compensate overflowing offset
		if(angleDeg < 0)
		{
			angleDeg = 360 + angleDeg;
		}
		
		// Find entry index withing circle
		int idx = (int)Math.Round((angleDeg) / m_fEntryDistance);
		
		if(idx < 0 || idx >= elementCount)
		{
			idx = -1;
		}
		
		// Count with overflow
		if(idx == -1)
		{
			if(angleDeg > (360 - overflow + m_fEntryInitialOffset))
				return 0;
			else
				return -1;
		}

		int res = (int)Math.Clamp(idx, 0, elementCount);

		return res;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetLastSelectedEntry()
	{				
		if(!m_pLastSelected)
			return;
		
		m_fLastSelectedAngle = IDFromFilter(m_pLastSelected) * m_fEntryDistance;
		
		// Update selected 	
		OnSelectedEntryChange(m_pLastSelected, m_fLastSelectedAngle);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddListeners()
	{
		m_pRadialMenuInteractions.onMenuToggleInvoker.Insert(OpenMenu);
		m_pRadialMenuInteractions.onPerformInputCallInvoker.Insert(TryPerformEntry);
		m_pRadialMenuInteractions.onThumbstickMoveInvoker.Insert(SelectionUpdate);
		m_pRadialMenuInteractions.m_OnPageSwitch.Insert(SwitchPage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveListeners()
	{
		m_pRadialMenuInteractions.onMenuToggleInvoker.Remove(OpenMenu);
		m_pRadialMenuInteractions.onPerformInputCallInvoker.Remove(TryPerformEntry);
		m_pRadialMenuInteractions.onThumbstickMoveInvoker.Remove(SelectionUpdate);
		m_pRadialMenuInteractions.m_OnPageSwitch.Remove(SwitchPage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change active page and handle data update
	protected void SetPage(int pageId)
	{
		m_iActivePage = pageId;
		int lastSelect = m_aSCR_MenuPages[pageId].GetLastSelected();
		
		int elementsCount = m_aSCR_MenuPages[pageId].GetEntriesCount();
		if (elementsCount > 0) 
			m_fEntryDistance = 360 / elementsCount;
		
		OnPositionChange(m_fEntryDistance, m_fEntryInitialOffset);
		m_OnPageSwitch.Invoke(pageId, lastSelect);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Move to page by given amoit 
	protected void SwitchPage(int pageMove)
	{
		// Fail if move is out of bounds 
		if (m_iActivePage + pageMove < 0 || m_iActivePage + pageMove > m_aSCR_MenuPages.Count() - 1)
			return;
		
		// Switch and update for next page 
		SetPage(m_iActivePage + pageMove);
		UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuHandler()
	{
		m_pRadialMenuInteractions = new SCR_RadialMenuInteractions();
		
		// Define invokers
		onMenuToggleInvoker = new ref ScriptInvoker<IEntity, bool>();
		onPositionChangeInvoker = new ref ScriptInvoker<float, float>();
		onSelectedEntryChangeInvoker = new ref ScriptInvoker<BaseSelectionMenuEntry, float>();
		onSelectionUpdateInvoker = new ref ScriptInvoker<bool, bool, float, float, BaseSelectionMenuEntry>();
		m_OnEntriesUpdate = new ref ScriptInvoker<array<BaseSelectionMenuEntry>, array<BaseSelectionMenuEntry>>();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuHandler()
	{
		RemoveListeners();
	}
};

//------------------------------------------------------------------------------------------------
//! Utility class for passing around lists of entries
class SCR_RadialMenuFilter
{	
	//! Contains all entries in order they were received
	//! Can contain items which are disabled too
	ref array<BaseSelectionMenuEntry> m_aAllEntries;
	
	//! Contains entries which cannot be performed (but can be shown)
	ref array<BaseSelectionMenuEntry> m_aDisabledEntries;
	
	//------------------------------------------------------------------------------------------------
	void DoFilter(IEntity owner, BaseSelectionMenu menu)
	{
		m_aAllEntries.Clear();
		m_aDisabledEntries.Clear();
		
		if (!owner || !menu)
			return;
		
		auto entries = new array<BaseSelectionMenuEntry>();
		auto entriesCount = menu.GetEntryList(entries);
		for (int i = 0; i < entriesCount; i++)
		{
			auto currentEntry = entries[i];
			if (!currentEntry)
				continue;			
			
			if (!currentEntry.CanBeShown(owner, menu))
				continue;
			
			m_aAllEntries.Insert(currentEntry);
			
			if (!currentEntry.CanBePerformed(owner, menu))
				m_aDisabledEntries.Insert(currentEntry);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuFilter()
	{
		m_aAllEntries = new ref array<BaseSelectionMenuEntry>();
		m_aDisabledEntries = new ref array<BaseSelectionMenuEntry>();	
	}
};

// Radial menu enums

//------------------------------------------------------------------------------------------------
enum ERadialMenuPerformType
{
	OnClose,
	OnPressPerformInput
};

//------------------------------------------------------------------------------------------------
enum ERadialMenuSelectionBehavior
{
	FreeMove,
	StickToLastSection,
	WithinEntriesMove
};
