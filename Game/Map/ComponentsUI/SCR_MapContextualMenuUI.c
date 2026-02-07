//------------------------------------------------------------------------------------------------
class SCR_MapContextualMenuRequestedTaskEntry : SCR_MapContextualMenuEntry
{
	SCR_RequestedTaskSupportClass m_SupportClass;
	
	//------------------------------------------------------------------------------------------------
	void SetSupportClass(SCR_RequestedTaskSupportClass supportClass)
	{
		m_SupportClass = supportClass;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		return m_SupportClass.CanRequest();
	}
};

//------------------------------------------------------------------------------------------------
//! Entry for map contextual menu
class SCR_MapContextualMenuEntry
{
	bool m_bDebugEntry = false;		// wont appear in the list of options unless debug entries are enabled
	int m_iHash;
	string m_sName;
	ref ScriptInvoker m_OnClick = new ScriptInvoker();	// entry callbacks
	ref ScriptInvoker m_OnShow = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	bool CanBeShown()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Open(notnull Widget button)
	{
		//! get the widget handler component & set data
		SCR_ButtonTextComponent buttonComp = SCR_ButtonTextComponent.Cast(button.FindHandler(SCR_ButtonTextComponent));
		if (buttonComp)
		{
			// TODO some of the settings here like colors should be moved to own button layout prefab once the requirements are settled
			buttonComp.SetText(m_sName);
			buttonComp.m_OnClicked = m_OnClick;
			if (!m_bDebugEntry)
				buttonComp.m_BackgroundDefault = Color.FromSRGBA(0, 0, 0, 190);
			else 
				buttonComp.m_BackgroundDefault = Color.FromSRGBA(150, 0, 0, 190);
			
			buttonComp.m_BackgroundHovered = Color.FromSRGBA(0, 0, 0, 220);
			buttonComp.m_bShowBorderOnHover = true;
			buttonComp.m_bUseColorization = true;
			buttonComp.ColorizeBackground(false);
		}
		
		m_OnShow.Invoke(this, button);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapContextualMenuEntry(string name = "")
	{
		if (name == string.Empty) return;

		m_sName = name;
		m_iHash = name.Hash();
	}
};

//------------------------------------------------------------------------------------------------
//! Fulscreen map contextual menu UI
class SCR_MapContextualMenuUI: SCR_MapUIBaseComponent
{
	// TODO visual overhaul
	// TODO SCR_UIRequestEvacTaskComponent & SCR_TransportTaskSupportClass should have simplified button registration through this, ideally one liner without the need to handle button creation etc
	
	const string WIDGET_NAME = "MapContextualMenu";
	const string ENTRY_LAYOUT_NAME = "VLayout";
	const string BUTTON_RESOURCE = "{75C912A1C89BE6C2}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonText.layout";
	
	protected WorkspaceWidget 		m_Workspace;
	protected Widget 				m_CtxMenuWidget;
	
	protected bool m_bRefresh;
	protected bool m_bEntriesUpdate = false;		// entries updated instead of entry selected
	
	protected ref array<ref Widget> m_aMenuEntriesWidgets = {};						// entry widgets
	protected ref array<ref SCR_MapContextualMenuEntry>		m_aCtxMenuEntries = {};	// entries to be built												
	protected ref map<int, ref SCR_MapContextualMenuEntry>	m_aCtxMenuEntriesDynamic = new map<int, ref SCR_MapContextualMenuEntry>;	// lookup of entries to avoid duplicates
	
	protected int m_iPosX;
	protected int m_iPosY;
	
	//------------------------------------------------------------------------------------------------
	static SCR_MapContextualMenuUI GetInstance()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return null;
		
		return SCR_MapContextualMenuUI.Cast(mapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetContextMenuWorldPosition()
	{		
		float worldX, worldY, worldZ;
		
		int x = GetGame().GetWorkspace().DPIScale(GetPosX());
		int y = GetGame().GetWorkspace().DPIScale(GetPosY());
		
		m_MapEntity.ScreenToWorld(x, y, worldX, worldZ);
		
		worldY = GetGame().GetWorld().GetSurfaceY(worldX, worldZ);
		
		return Vector(worldX, worldY, worldZ);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPosY()
	{
		return m_iPosY;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPosX()
	{
		return m_iPosX;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPosY(int y)
	{
		m_iPosY = y;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPosX(int x)
	{
		m_iPosX = x;
	}
	
	//------------------------------------------------------------------------------------------------
	//! opens modal window and shows the menu widget
	//! \return false if cannot open due to not having any entries
	bool OpenMenu(int posX, int posY)
	{
		if(!BuildContextualEntries())
			return false;	// no entries

		m_Workspace.RemoveModal(m_Workspace.GetModal());
		m_Workspace.AddModal(m_CtxMenuWidget, m_CtxMenuWidget);
		m_Workspace.SetFocusedWidget(m_CtxMenuWidget);
		FrameSlot.SetPos(m_CtxMenuWidget, posX, posY);
		
		SetPosX(posX);
		SetPosY(posY);

		m_CtxMenuWidget.SetVisible(true);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Close modal window and hide the widget
	void CloseMenu()
	{
		if (m_CtxMenuWidget)
			m_CtxMenuWidget.SetVisible(false);	
		
		m_Workspace.RemoveModal(m_CtxMenuWidget);
		m_Workspace.SetFocusedWidget(m_RootWidget);
		
		RemoveWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MapContextualMenuEntry ContextInsertDynamic(SCR_MapContextualMenuEntry entry)
	{
		if (m_aCtxMenuEntriesDynamic.Contains(entry.m_iHash))
			return new SCR_MapContextualMenuEntry();
		
		m_aCtxMenuEntriesDynamic.Set(entry.m_iHash, entry);
		
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register dynamic ctx menu entry
	SCR_MapContextualMenuEntry ContextRegisterDynamic(string entryName, bool debugEntry = false)
	{
		SCR_MapContextualMenuEntry entry = new SCR_MapContextualMenuEntry(entryName);
		
		if (debugEntry)
			entry.m_bDebugEntry = true;
		
		return ContextInsertDynamic(entry);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Create context menu entry widgets
	protected bool BuildContextualEntries()
	{
		m_aCtxMenuEntries.Clear();

		for (int i = 0, count = m_aCtxMenuEntriesDynamic.Count(); i < count; i++)
		{
			m_aCtxMenuEntries.Insert(m_aCtxMenuEntriesDynamic.GetElement(i));
		}
		
		if (m_aCtxMenuEntries.IsEmpty())
			return false;
		
		Widget vLayout = m_RootWidget.FindAnyWidget(ENTRY_LAYOUT_NAME);
		
		WorkspaceWidget workspace = m_CtxMenuWidget.GetWorkspace();
		bool focused = false;
		Widget button;
		
		for (int i = 0, count = m_aCtxMenuEntries.Count(); i < count; i++)
		{			
			button = workspace.CreateWidgets(BUTTON_RESOURCE, vLayout);
			m_aCtxMenuEntries[i].Open(button);
			
			if (!focused)
			{
				workspace.SetFocusedWidget(button);
				focused = true;
			}
			
			m_aMenuEntriesWidgets.Insert(button);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear all registered callbacks
	protected void ContextUnregisterDynamic()
	{
		int count = m_aCtxMenuEntriesDynamic.Count();
		
		for (int i = 0; i < count; i++)
		{
			m_aCtxMenuEntries.RemoveItem(m_aCtxMenuEntriesDynamic.GetElement(i));
		}
		
		m_aCtxMenuEntriesDynamic.Clear();
		m_aCtxMenuEntries.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear the context entry widgets
	protected void RemoveWidgets()
	{
		if (m_aMenuEntriesWidgets)
		{
			int cnt = m_aMenuEntriesWidgets.Count();
			for (int i = cnt-1; i >= 0; i--)
			{
				m_aMenuEntriesWidgets[i].RemoveFromHierarchy();
				m_aMenuEntriesWidgets[i] = null;
			}
			
			m_aMenuEntriesWidgets.Clear();
		}
	}
			
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override bool OnModalClickOut(Widget modalRoot, int x, int y, int button)
	{
		CloseMenu();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// Close the menu after button click or expand options
		if (m_bEntriesUpdate)
			m_bEntriesUpdate = false;
		else 
			GetGame().GetCallqueue().CallLater(CloseMenu);	// delayed by frame because its possible for this to trigger before buttons onClick events which would close the menu and not perform them
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_Workspace = GetGame().GetWorkspace();
		m_CtxMenuWidget = m_RootWidget.FindAnyWidget(WIDGET_NAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		ContextUnregisterDynamic();
		CloseMenu();
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapContextualMenuUI()
	{
		m_bHookToRoot = true;
	}
};