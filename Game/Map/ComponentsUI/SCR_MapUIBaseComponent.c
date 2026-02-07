//------------------------------------------------------------------------------------------------
// SCR_MapUIBaseComponent handles the context menu, dragging and clicks.
[BaseContainerProps()]
class SCR_MapUIBaseComponent : ScriptedWidgetComponent
{		
	protected bool m_bHookToRoot = false;							// determine whether this component is hooked to the root widget for use of ScriptedWidgetEventHandler events
	protected Widget m_RootWidget;									// map layout root widget	
	protected SCR_MapEntity m_MapEntity;	
	protected static ref map<Widget, ref Event> m_aOnClickEventMap;	// map of all the events created for specific widgets
	
	//------------------------------------------------------------------------------------------------
	// WIDGET EVENTS
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		Event clickEvent;

		// event not found
		if (!m_aOnClickEventMap.Find(w, clickEvent)) 
			return true;

		if (clickEvent) 
			clickEvent.Emit(w);
			
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Fetch custom widget event, if it doesnt exist, create and add it to a map with the widget name as a key
	//! \param widgetName is name of the widget to add a callback to
	//! \param rootManual is widget root from which widget name is searched
	//! \return Returns the Event 
	protected Event AddOrFindClickEvent(string widgetName, Widget rootManual = null)
	{
		Widget root = m_RootWidget;
		if (rootManual)
			root = rootManual;

		if (!root)
			return null;
		
		Widget widget = root.FindAnyWidget(widgetName);
		if (!widget) 
			return null;
		
		// create Event, add to a map with key being the widgetName
		Event onClick = new Event();
		if (!m_aOnClickEventMap.Contains(widget))
			m_aOnClickEventMap.Insert(widget, onClick);
		
		return onClick;
	}
					
	//------------------------------------------------------------------------------------------------
	// BASE METHODS
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapOpen(MapConfiguration config)
	{	
		if (m_bHookToRoot)
			m_RootWidget.AddHandler(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapClose(MapConfiguration config)
	{
		if (m_bHookToRoot)
			m_RootWidget.RemoveHandler(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable open/close events, called every time component is activated, usually on map open
	void SetActive(bool active)
	{
		if (active)
		{
			m_RootWidget = m_MapEntity.GetMapConfig().RootWidgetRef; // Needs to be refreshed here
			
			m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		}
		else 
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init method for cases where all modules and components should be loaded already so constructor cannot be used, called once after creation
	void Init()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Update method for frame operations
	void Update()
	{}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapUIBaseComponent()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
						
		if (!m_aOnClickEventMap)
	 		m_aOnClickEventMap = new map<Widget, ref Event>();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapUIBaseComponent() 
	{
		if (m_MapEntity)
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
		}
		
		if (m_aOnClickEventMap)
			m_aOnClickEventMap.Clear();
	}
};