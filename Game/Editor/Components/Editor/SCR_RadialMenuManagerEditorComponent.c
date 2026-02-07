[ComponentEditorProps(category: "GameScripted/Editor", description: "Component which will spawn radialmenu prefab for editor, using Editor component logic", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_RadialMenuManagerEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/*

*/
class SCR_RadialMenuManagerEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of Editor Radialmenu entity", "et")]
	private ResourceName m_EditorRadialMenuPrefab;
	
	private IEntity m_RadialMenuEntity
	
	private ref map<EEditorRadialMenuType, SCR_RadialMenuHandler> m_RadialMenuHandlers = new map<EEditorRadialMenuType, SCR_RadialMenuHandler>;
	
	private ref ScriptInvoker Event_EditorRadialMenuOpened = new ScriptInvoker();
	
	
	/*!
	Event for when any editor radial menu is opened, useful for hiding UI
	\return ScriptInvoker which is invoked by each RadialMenu OnMenuToggle event, parameters (IEntity owner, bool isOpened)
	*/
	ScriptInvoker GetEditorRadialMenuOpened()
	{
		return Event_EditorRadialMenuOpened;
	}
	
	bool GetRadialMenuHandler(EEditorRadialMenuType radialMenuType, out SCR_RadialMenuHandler handler)
	{
		handler = m_RadialMenuHandlers.Get(radialMenuType);
		return handler != null;
	}
	
	bool IsRadialMenuOpen()
	{
		 foreach(EEditorRadialMenuType type, SCR_RadialMenuHandler menu : m_RadialMenuHandlers)
		 {
		      if (menu && menu.IsOpen())
				return true;
		 }
		
		return false;
	}
	
	protected void OnRadialMenuToggled(IEntity owner, bool isOpened)
	{
		Event_EditorRadialMenuOpened.Invoke(owner, isOpened);	
	}
	
	override void EOnEditorActivate()
	{
		if (m_EditorRadialMenuPrefab.IsEmpty())
		{
			Print("Editor radial menu prefab not defined on SCR_RadialMenuManagerEditorComponent", LogLevel.ERROR);
		}
			
		m_RadialMenuEntity = GetGame().SpawnEntityPrefabLocal(Resource.Load(m_EditorRadialMenuPrefab));	
		
		array<Managed> radialMenuComponents = {};
		m_RadialMenuEntity.FindComponents(SCR_RadialMenuComponent, radialMenuComponents);
		
		foreach	(Managed component : radialMenuComponents)
		{
			SCR_RadialMenuComponent radialMenuComponent = SCR_RadialMenuComponent.Cast(component);
			SCR_RadialMenuHandler handler = radialMenuComponent.GetRadialMenuHandler();
			
			SCR_RadialMenuEditorCommands commandsHandler = SCR_RadialMenuEditorCommands.Cast(handler);
			SCR_RadialMenuEditorActions actionsHandler = SCR_RadialMenuEditorActions.Cast(handler);
			
			EEditorRadialMenuType radialMenuType = -1;
			
			if (actionsHandler)
			{
				radialMenuType = EEditorRadialMenuType.ACTIONS;
			}
			else if (commandsHandler)
			{
				radialMenuType = EEditorRadialMenuType.COMMANDS;
			}
			
			if (radialMenuType != -1)
			{
				handler.onMenuToggleInvoker.Insert(OnRadialMenuToggled);
				m_RadialMenuHandlers.Set(radialMenuType, handler);
			}
		}
	}
	
	override void EOnEditorDeactivate()
	{
		if (m_RadialMenuEntity)
		{
			array<Managed> radialMenuComponents = {};
			if (m_RadialMenuEntity.FindComponents(SCR_RadialMenuComponent, radialMenuComponents) > 0)
			{
				foreach	(Managed component : radialMenuComponents)
				{
 					SCR_RadialMenuComponent radialMenuComponent = SCR_RadialMenuComponent.Cast(component);
					SCR_RadialMenuHandler handler = radialMenuComponent.GetRadialMenuHandler();
					if (handler)
					{
						handler.onMenuToggleInvoker.Remove(OnRadialMenuToggled);	
					}
				}
			}
			
			m_RadialMenuHandlers.Clear();
			
			SCR_Global.DeleteEntityAndChildren(m_RadialMenuEntity);
		}
	}
};