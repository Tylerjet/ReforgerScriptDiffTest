//------------------------------------------------------------------------------------------------
/*!
Scripted base for selection menu used in HUD
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenu
{
	// Attributes
	[Attribute()]
	protected ref SCR_SelectionMenuInputs m_Inputs;

	[Attribute("")]
	protected string m_sSelectionSound;

	[Attribute("")]
	protected string m_sPerformSound;

	// Variables
	protected ref array<ref SCR_SelectionMenuEntry> m_aRootEntries = {}; // 1st layer of entries
	protected ref array<ref SCR_SelectionMenuEntry> m_aEntries = {}; // Current layer entries
	protected SCR_SelectionMenuEntry m_SelectedEntry;

	protected ref SCR_SelectionMenuControllerInputs m_ControllerInputs;
	protected bool m_bOpened;
	protected bool m_bEntryPerformed;
	protected bool m_bUsingAlternativeToggle;

	// Categories and multi layering
	protected ref array<SCR_SelectionMenuCategoryEntry> m_aSelectedCategories = {};

	// Events
	protected ref ScriptInvoker<SCR_SelectionMenu> m_OnOpen;
	protected ref ScriptInvoker<SCR_SelectionMenu> m_OnClose;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry, int> m_OnSelect;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnPerform;

	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnAddEntry;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnRemoveEntry;
	protected ref ScriptInvoker<SCR_SelectionMenu, array<ref SCR_SelectionMenuEntry>> m_OnUpdateEntries;

	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuControllerInputs> m_OnControllerChanged;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnOpen()
	{
		if (m_OnOpen)
			m_OnOpen.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnOpen()
	{
		if (!m_OnOpen)
			m_OnOpen = new ScriptInvoker();

		return m_OnOpen;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnClose()
	{
		if (m_OnClose)
			m_OnClose.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnClose()
	{
		if (!m_OnClose)
			m_OnClose = new ScriptInvoker();

		return m_OnClose;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnSelect(SCR_SelectionMenuEntry entry, int id)
	{
		if (m_OnSelect)
			m_OnSelect.Invoke(this, entry, id);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnSelect()
	{
		if (!m_OnSelect)
			m_OnSelect = new ScriptInvoker();

		return m_OnSelect;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnPerform(SCR_SelectionMenuEntry entry)
	{
		if (m_OnPerform)
			m_OnPerform.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPerform()
	{
		if (!m_OnPerform)
			m_OnPerform = new ScriptInvoker();

		return m_OnPerform;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnAddEntry(SCR_SelectionMenuEntry entry)
	{
		if (m_OnAddEntry)
			m_OnAddEntry.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnAddEntry()
	{
		if (!m_OnAddEntry)
			m_OnAddEntry = new ScriptInvoker();

		return m_OnAddEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnRemoveEntry(SCR_SelectionMenuEntry entry)
	{
		if (m_OnRemoveEntry)
			m_OnRemoveEntry.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnRemoveEntry()
	{
		if (!m_OnRemoveEntry)
			m_OnRemoveEntry = new ScriptInvoker();

		return m_OnRemoveEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnUpdateEntries(array<ref SCR_SelectionMenuEntry> entries)
	{
		foreach (SCR_SelectionMenuEntry entry : m_aEntries)
		{
			if (entry)
				entry.Update();
		}

		if (m_OnUpdateEntries)
			m_OnUpdateEntries.Invoke(this, entries);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnUpdateEntries()
	{
		if (!m_OnUpdateEntries)
			m_OnUpdateEntries = new ScriptInvoker();

		return m_OnUpdateEntries;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnControllerChanged(SCR_SelectionMenuControllerInputs inputs)
	{
		if (m_OnControllerChanged)
			m_OnControllerChanged.Invoke(this, inputs);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnControllerChanged()
	{
		if (!m_OnControllerChanged)
			m_OnControllerChanged = new ScriptInvoker();

		return m_OnControllerChanged;
	}

	//------------------------------------------------------------------------------------------------
	// Custom methods
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Open()
	{
		m_bOpened = true;
		m_bEntryPerformed = false;

		OnOpen();

		InvokeEventOnOpen();
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on open ready for override
	protected void OnOpen(){}

	//------------------------------------------------------------------------------------------------
	//! Callback when close is requested
	void Close()
	{
		// Perform
		if (m_ControllerInputs.m_bPerformOnClose && m_SelectedEntry && !m_bEntryPerformed)
			PerformEntry(m_SelectedEntry);

		m_bOpened = false;
		m_bUsingAlternativeToggle = false;
		OnClose();

		InvokeEventOnClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on close ready for override
	protected void OnClose(){}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		// Context
		if (m_bOpened)
			GetGame().GetInputManager().ActivateContext(m_Inputs.m_sContext);

		if (m_ControllerInputs)
			GetGame().GetInputManager().ActivateContext(m_ControllerInputs.m_sControllerContext);

		OnUpdate(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on update ready for override
	protected void OnUpdate(float timeSlice){}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string sound)
	{
		if (!sound.IsEmpty())
			SCR_UISoundEntity.SoundEvent(sound);
	}

	//------------------------------------------------------------------------------------------------
	// Entries handling
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Generic method for custom entry selection based on used interface
	protected void SelectEntry() {}

	//------------------------------------------------------------------------------------------------
	void PerformEntry(notnull SCR_SelectionMenuEntry entry)
	{
		if (!entry.IsEnabled())
			return;

		// Is category entry
		SCR_SelectionMenuCategoryEntry category = SCR_SelectionMenuCategoryEntry.Cast(entry);

		entry.Perform();

		PlaySound(m_sPerformSound);

		// Generic entry
		if (!category && m_ControllerInputs.m_bCloseOnPerform)
		{
			m_bEntryPerformed = true;
			Close();
		}

		// Category
		if (category)
		{
			OpenCategoryEntry(category);

			// Perform default action on closing
		}

		InvokeEventOnPerform(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! Specific peform action for category to change menu content
	protected void OpenCategoryEntry(notnull SCR_SelectionMenuCategoryEntry category)
	{
		//AddEntries(category.GetEntries(), true);

		m_aSelectedCategories.Insert(category);
		InvokeEventOnUpdateEntries(category.GetEntries());
	}

	//------------------------------------------------------------------------------------------------
	//! Add new empty entry and update all entries if not declined
	void AddEntry(SCR_SelectionMenuEntry entry = null, bool update = true)
	{
		if (!entry)
			SCR_SelectionMenuEntry newEntry = new SCR_SelectionMenuEntry();

		m_aEntries.Insert(entry);

		InvokeEventOnAddEntry(entry);

		if (update)
			InvokeEventOnUpdateEntries(m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	// TODO: Category adding logic once categories are ready
	void AddCategoryEntry(SCR_SelectionMenuCategoryEntry category = null)
	{
		SCR_SelectionMenuEntry entry = category;

		if (!entry)
			entry = new SCR_SelectionMenuCategoryEntry();

		AddEntry(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! Add multiple entries in array
	//! Replace true will clear menu and use given entries
	void AddEntries(notnull array<ref SCR_SelectionMenuEntry> entries, bool replace = false)
	{
		// Clear
		if (replace)
			m_aEntries.Clear();

		// Add
		for (int i = 0, count = entries.Count(); i < count; i++)
			AddEntry(entries[i], false);

		// Stacked update
		UpdateEntries()
	}

	//------------------------------------------------------------------------------------------------
	//! Invoke data update for all entries
	void UpdateEntries()
	{
		InvokeEventOnUpdateEntries(m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoke data update for selected entries
	void UpdateEntries(notnull array<ref SCR_SelectionMenuEntry> entries)
	{
		InvokeEventOnUpdateEntries(entries);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove selected entry and invoke data update
	void RemoveEntry(notnull SCR_SelectionMenuEntry entry)
	{
		m_aEntries.RemoveItem(entry);
		InvokeEventOnUpdateEntries(m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! Clear all entries and invoke data update
	void ClearEntries()
	{
		m_aEntries.Clear();
		InvokeEventOnUpdateEntries(m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	// Inputs
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Add all action listeners for basic menu control
	protected void AddActionListeners()
	{
		if (m_ControllerInputs.m_bCloseOnReleaseOpen)
			GetGame().GetInputManager().AddActionListener(m_ControllerInputs.m_sOpenAction, EActionTrigger.UP, OnOpenInputRelease);

		if (!m_ControllerInputs.m_sToggleActionAlternative.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_ControllerInputs.m_sToggleActionAlternative, EActionTrigger.DOWN, OnAlternativeToggleInput);

		if (!m_Inputs.m_sPerformAction.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_Inputs.m_sPerformAction, EActionTrigger.DOWN, OnPerformInput);

		if (!m_Inputs.m_sBackAction.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_Inputs.m_sBackAction, EActionTrigger.DOWN, OnBackInput);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all action listeners for basic menu control
	protected void RemoveActionListeners()
	{
		GetGame().GetInputManager().RemoveActionListener(m_ControllerInputs.m_sOpenAction, EActionTrigger.UP, OnOpenInputRelease);
		GetGame().GetInputManager().RemoveActionListener(m_ControllerInputs.m_sToggleActionAlternative, EActionTrigger.DOWN, OnAlternativeToggleInput);
		GetGame().GetInputManager().RemoveActionListener(m_Inputs.m_sPerformAction, EActionTrigger.DOWN, OnPerformInput);
		GetGame().GetInputManager().RemoveActionListener(m_Inputs.m_sBackAction, EActionTrigger.DOWN, OnBackInput);
	}

	//------------------------------------------------------------------------------------------------
	//! On open input handle menu closing
	protected void OnOpenInputRelease(float value, EActionTrigger reason)
	{
		if (m_bUsingAlternativeToggle)
			return;

		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! On select input handle entry selection and moving into layers
	protected void OnPerformInput()
	{
		if (m_SelectedEntry)
			PerformEntry(m_SelectedEntry);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBackInput()
	{
		// Prevent entry performing
		m_bEntryPerformed = true;

		// Should close on top level
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAlternativeToggleInput()
	{
		m_bUsingAlternativeToggle = true;
	}

	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Set controller entity and controls
	void SetController(IEntity owner, SCR_SelectionMenuControllerInputs controls)
	{
		m_ControllerInputs = controls;

		if (m_ControllerInputs)
			m_ControllerInputs.m_Owner = owner;

		InvokeOnControllerChanged(controls);
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuControllerInputs GetControllerInputs()
	{
		return m_ControllerInputs;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuEntry GetSelectionEntry()
	{
		return m_SelectedEntry;
	}

	//------------------------------------------------------------------------------------------------
	// Debug
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void DebugPrint(string method, string msg)
	{
		Print(string.Format("[SCR_SelectionMenu] - %1() - '%2'", method, msg), LogLevel.DEBUG);
	}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable selection menu inputs for separating menu controls actions
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuInputs
{
	[Attribute("", desc: "Input context used for menu controls")]
	string m_sContext;

	[Attribute("", desc: "Action used for closing menu or moving back between the layers")]
	string m_sBackAction;

	[Attribute("", desc: "Input action used for performing selected entry")]
	string m_sPerformAction;

	//------------------------------------------------------------------------------------------------
	//! Initialization method for setting up inputs at runtime
	void Init() {}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable menu controls for controlling entity
This controls are defined at controlling entity - e.g. Player, GM entity, Map, etc.
E.g. For Quick slots menu and Editor actions menu will be different keys to open
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuControllerInputs
{
	IEntity m_Owner;

	[Attribute("", desc: "Input context used for extra menu controls from controller entity")]
	string m_sControllerContext;

	[Attribute("", desc: "Action for opening (and closing) radial menu")]
	string m_sOpenAction;

	[Attribute("", desc: "Alternative action for always toggling menu")]
	string m_sToggleActionAlternative;

	[Attribute("1", desc: "If this field is checked - Menu is closed if open input is released. Otherwise separate close input has to be used")]
	bool m_bCloseOnReleaseOpen;

	[Attribute("1", desc: "If this field is checked - Perform entry(call entry action) when closing menu and having selected entry")]
	bool m_bPerformOnClose;

	[Attribute("0", desc: "If this field is checked - Close the menu after performing the entry (selecting enabled entry)")]
	bool m_bCloseOnPerform;

	//------------------------------------------------------------------------------------------------
	void SCR_SelectionMenuOpening(string openAction = "")
	{
		if (!openAction.IsEmpty())
			m_sOpenAction = openAction;
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if given entity is controlling current menu
	bool IsControllingMenu(IEntity controller)
	{
		return controller == m_Owner;
	}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable selection menu entries list
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuData
{
	[Attribute()]
	protected ref array<ref SCR_SelectionMenuEntry> m_aEntries;

	//------------------------------------------------------------------------------------------------
	array<ref SCR_SelectionMenuEntry> GetEntries()
	{
		array<ref SCR_SelectionMenuEntry> entries = {};

		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return entries;
	}
};
