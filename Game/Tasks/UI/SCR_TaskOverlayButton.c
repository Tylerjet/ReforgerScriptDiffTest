class SCR_TaskOverlayButton : ScriptedWidgetComponent
{
	protected SCR_MapUITask m_MapUiTask; 
	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_MapUiTask)
			return false; 
		
		ButtonWidget assignButton = ButtonWidget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("TaskTitleButton"));
		ButtonWidget overlayButton = ButtonWidget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("OverlayButton"));
		Widget assignee = Widget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("Assignee"));
		Widget iconHover = Widget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("TaskIconHover"));
		
		if (!assignButton || !overlayButton || !assignee || !iconHover)
			return false;
		
		if (enterW != overlayButton && enterW != assignButton)
		{
			assignButton.SetVisible(false);
			
			assignee.SetEnabled(false);
			assignee.SetOpacity(0);
			
			iconHover.SetEnabled(false);
			iconHover.SetOpacity(0);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------
	void SetRootWidgetHandler(SCR_MapUITask mapUiTask)
	{
		m_MapUiTask = mapUiTask;
	}
}