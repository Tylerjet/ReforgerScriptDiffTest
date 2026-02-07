[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of editor context actions", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ContextActionsEditorComponentClass: SCR_BaseActionsEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Component to hold all context actions for each editor mode and filter out actions depending on context

Intended use is for this component to be added to each editor mode entity which should have specific actions available

These actions are defined on the editor mode prefab entity as types of SCR_BaseContextAction
*/
class SCR_ContextActionsEditorComponent : SCR_BaseActionsEditorComponent
{	
	protected ref ScriptInvoker m_OnMenuOpen = new ScriptInvoker();
	
	/*!
	Get event called for the user when the menu is opened.
	\return Script invoker
	*/
	ScriptInvoker GetOnMenuOpen()
	{
		return m_OnMenuOpen;
	}
	
	override protected int ValidateSelection(bool isInstant)
	{
		super.ValidateSelection(isInstant);
		
		EEditorContextActionFlags flags;
		//--- Cache flags
		if (GetManager().IsLimited())
		{
			flags |= EEditorContextActionFlags.LIMITED;
		}
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			flags |= EEditorContextActionFlags.USING_MOUSE_AND_KEYBOARD;
		}
		return flags;
	}
	override void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		super.EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
		m_OnMenuOpen.Invoke(actions, cursorWorldPosition, filteredActions, flags);
	}
	
	override protected bool ActionCanBeShown(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.GetInfo() && action.CanBeShown(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}
	
	override protected bool ActionCanBePerformed(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.CanBePerformed(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}
	
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU, "Actions Menu", "Editor");
	}
	
	override void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU);
	}
};
