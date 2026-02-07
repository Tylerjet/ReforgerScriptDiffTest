//------------------------------------------------------------------------------------------------
class SCR_PasswordDialogComponent : ScriptedWidgetComponent
{
	// Widget Names 
	protected const string EDIT_BOX = "EditBox";
	
	// Widgets 
	protected SCR_EditBoxComponent m_EditBox;

	//------------------------------------------------------------------------------------------------
	// Dialog override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_EditBox = SCR_EditBoxComponent.GetEditBoxComponent(EDIT_BOX, w);
			
		// Focus password editbox next frame to ensure activation will happened over UI behavior
		GetGame().GetCallqueue().CallLater(ActivateWriteMode);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateWriteMode()
	{
		if (!m_EditBox)
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(m_EditBox.GetRootWidget());
		m_EditBox.ActivateWriteMode();
	}
};



