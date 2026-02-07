[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
/** @ingroup ManualCamera
*/

/*!
Check if cursor is not on top of another widget
*/
class SCR_EditorCursorManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected SCR_MouseAreaEditorUIComponent m_MouseArea;
	
	protected bool m_IsEditorUiVisible;
	
	protected void OnEditorVisibilityChanged(bool visible)
	{
		m_IsEditorUiVisible = visible;
	}
	
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		param.isCursorEnabled = (m_MouseArea && m_MouseArea.IsMouseOn()) || !m_IsEditorUiVisible;
	}
	override bool EOnCameraInit()
	{
		SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent, true));
		if (!menuManager) return false;
		
		menuManager.GetOnVisibilityChange().Insert(OnEditorVisibilityChanged);
		
		EditorMenuBase menu = menuManager.GetMenu();
		if (!menu) return false;
		
		MenuRootComponent root = menu.GetRootComponent();
		if (!root) return false;
		
		m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(root.FindComponent(SCR_MouseAreaEditorUIComponent, true));
		return m_MouseArea != null;
	}
	override void EOnCameraExit()
	{
		SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuManager) return;
		
		menuManager.GetOnVisibilityChange().Remove(OnEditorVisibilityChanged);
	}
};