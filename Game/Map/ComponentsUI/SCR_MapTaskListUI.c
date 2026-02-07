//------------------------------------------------------------------------------------------------
class SCR_MapTaskListUI : SCR_MapUIBaseComponent
{
	const string TASK_LIST_FRAME = "MapTaskList";
	
	protected Widget m_wUI;
	protected Widget m_wTaskListFrame;
	protected bool m_bWidgetFocused;

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bWidgetFocused = true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bWidgetFocused = false;		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMapOpen(MapConfiguration config)
	{		
		SCR_UITaskManagerComponent uiTaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!uiTaskManager)
			return;	
		
		uiTaskManager.ClearWidget();
		
		m_wTaskListFrame = Widget.Cast(m_RootWidget.FindAnyWidget(TASK_LIST_FRAME));
		if (!m_wTaskListFrame)
			return;
		
		if (!m_wUI)
			GenerateTaskListUI();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMapClose(MapConfiguration config)
	{	
		SCR_UITaskManagerComponent uiTaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!uiTaskManager)
			return;	

		m_wTaskListFrame.SetVisible(false);	
		uiTaskManager.ClearWidget();
		uiTaskManager.CreateTaskList();	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_bWidgetFocused && m_wUI.IsVisible())
			GetGame().GetInputManager().ActivateContext("TaskListMapContext");
	}
	
	//------------------------------------------------------------------------------------------------
	void GenerateTaskListUI()
	{
		SCR_UITaskManagerComponent uiTaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!uiTaskManager)
			return;	
		
		Widget overlay = Widget.Cast(m_RootWidget.FindAnyWidget(TASK_LIST_FRAME));
		if (!overlay)
			return;
		
		m_wUI = uiTaskManager.CreateTaskList(overlay);
		uiTaskManager.Action_ShowTasks();

		overlay.AddHandler(this);
		
		SCR_NavigationButtonComponent hideTasks = SCR_NavigationButtonComponent.GetNavigationButtonComponent("HideTasksButton", m_wUI);
		if (hideTasks)
			hideTasks.m_OnActivated.Insert(uiTaskManager.Action_TasksClose);
		
		m_wTaskListFrame.SetVisible(true);
	}
};