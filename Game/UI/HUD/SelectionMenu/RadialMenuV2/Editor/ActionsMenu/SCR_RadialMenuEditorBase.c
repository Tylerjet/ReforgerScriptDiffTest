//------------------------------------------------------------------------------------------------
class SCR_RadialMenuEditorBase: SCR_RadialMenuHandler
{
	protected SCR_MenuLayoutEditorComponent m_MenuLayoutComponent;
	
	protected InputManager m_InputManager;
	protected MenuManager m_MenuManager;
	
	protected int m_ActionFlags;
	
	protected int ValidateSelection()
	{
		m_ActionFlags = 0;
		return m_ActionFlags;
	}
	
	protected void UpdateEditorEntriesData(IEntity owner, vector cursorWorldPosition)
	{

	}
	override protected void OnUpdate(IEntity owner, float timeSlice)
	{
		if (m_MenuManager.IsAnyDialogOpen())
			Close(owner);
		else
			super.OnUpdate(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Init(IEntity owner)
	{
		super.Init(owner);
		
		m_pOwner = owner;
		m_pSource = owner;
		
		m_InputManager = GetGame().GetInputManager();		
		m_MenuManager = GetGame().GetMenuManager();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{
		PageSetup();
		
		m_MenuLayoutComponent = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent));
		UpdateEntriesData(owner);
		
		super.OnOpen(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OpenMenu(IEntity owner, bool isOpen)
	{
		if (m_InputManager.IsUsingMouseAndKeyboard() ||  (isOpen && m_MenuManager.IsAnyDialogOpen()))
		{
			return;
		}
		
		m_ActionFlags = ValidateSelection();
		
		super.OpenMenu(owner, isOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateEntriesData(IEntity owner)
	{
		// Clear radial menu entries
		ClearEntries(0);
		
		vector cursorWorldPosition = vector.Zero;
		if (m_MenuLayoutComponent)
		{
			m_MenuLayoutComponent.GetCursorWorldPos(cursorWorldPosition);
		}
		
		UpdateEditorEntriesData(owner, cursorWorldPosition);
	}
};