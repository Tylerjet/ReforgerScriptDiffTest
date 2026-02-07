//------------------------------------------------------------------------------------------------
class SCR_ContextMenuActionsEditorUIComponent : SCR_BaseContextMenuEditorUIComponent
{
	[Attribute(UISounds.FOCUS, UIWidgets.EditBox)]
	protected string m_sSoundOnOpen;
	
	[Attribute("SOUND_E_TRAN_CANCEL", UIWidgets.EditBox)]
	protected string m_sSoundOnCancelClose;
	
	//~ Makes sure context action is never opened if not selection state
	protected bool m_bEditorIsSelectingState = true;
	
	override void PopulateContextMenu(vector cursorWorldPosition)
	{
		//--- Selection rules
		SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		SCR_BaseEditableEntityFilter hoveredFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
		if (hoveredFilter && selectedFilter)
		{
			SCR_EditableEntityComponent hoveredEntity = hoveredFilter.GetFirstEntity();
			if (hoveredEntity)
			{
				m_HoveredEntityReference = hoveredEntity;
				//--- Open menu over entity outside of the current selection - select it instead
				if (!selectedFilter.Contains(hoveredEntity))
					selectedFilter.Replace(hoveredEntity);
			}
			else
			{
				//--- Opened menu without any entity under cursor - clear the selection
				selectedFilter.Clear();
			}
		}
		
		super.PopulateContextMenu(cursorWorldPosition);
	}
	
	override protected void OnEditorModeChanged()
	{
		m_EditorActionsComponent = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent));
	}
	
	protected override void OpenContextMenu()
	{
		super.OpenContextMenu();
		
		if (m_ContextMenu && m_ContextMenu.IsVisible())
			SCR_UISoundEntity.SoundEvent(m_sSoundOnOpen);
	}
	
	override void CloseContextMenu()
	{
		if (m_ContextMenu && m_ContextMenu.IsVisible())
			GetGame().GetCallqueue().CallLater(DelayedCloseSound, 100);
			
		super.CloseContextMenu();
	}
	
	protected void DelayedCloseSound()
	{
		if (!IsContextMenuOpen())
			SCR_UISoundEntity.SoundEvent(m_sSoundOnCancelClose);
	}
	
	protected void OnOpenActionsMenuUp()
	{
		//~ Check if can open
		if (!OnCancelUp() || IsContextMenuOpen() || !m_bEditorIsSelectingState) 
		{
			//~ Reset the state
			m_bEditorIsSelectingState = true;
			return;
		}
		
		//~ Open
		OpenContextMenu();
	}
	

	
	protected override void OnOpenActionsMenuDown()
	{		
		super.OnOpenActionsMenuDown();
		
		//~ Is not selecting state so make sure on up context menu is not opened
		SCR_StatesEditorComponent statesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (statesManager && statesManager.GetState() != EEditorState.SELECTING) 
			m_bEditorIsSelectingState = false;
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (m_InputManager)
		{
			m_InputManager.AddActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnOpenActionsMenuDown);
			m_InputManager.AddActionListener("EditorActionsMenu", EActionTrigger.UP, OnOpenActionsMenuUp);
		}
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU_SHOWDISABLED, string.Empty, "Show disabled actions", "Actions Menu");
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnOpenActionsMenuDown);
			m_InputManager.RemoveActionListener("EditorActionsMenu", EActionTrigger.UP, OnOpenActionsMenuUp);
		}
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU_SHOWDISABLED);
	}
};
