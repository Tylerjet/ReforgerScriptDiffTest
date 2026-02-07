[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_VisibleEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected bool m_bCanShow = true;
	
	protected void OnEntityVisibilityChanged(SCR_EditableEntityComponent entity)
	{
		Toggle(entity);
	}
	
	protected void OnVisiblityChange(bool isVisible)
	{
		m_bCanShow = isVisible;
		if (isVisible)
			SetFromPredecessor();
		else
			Clear();
	}
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return m_bCanShow && entity.GetVisibleInHierarchy();
	}
	override void EOnEditorActivate()
	{
		if (!GetCore())
			return;
		
		GetCore().Event_OnEntityVisibilityChanged.Insert(OnEntityVisibilityChanged);
		
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (editorMenuManager)
		{
			editorMenuManager.GetOnVisibilityChange().Insert(OnVisiblityChange);
			m_bCanShow = editorMenuManager.IsVisible();
		}
	}
	override void EOnEditorDeactivate()
	{
		if (!GetCore())
			return;
		
		GetCore().Event_OnEntityVisibilityChanged.Remove(OnEntityVisibilityChanged);
		
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (editorMenuManager)
		{
			editorMenuManager.GetOnVisibilityChange().Remove(OnVisiblityChange);
		}
	}
};