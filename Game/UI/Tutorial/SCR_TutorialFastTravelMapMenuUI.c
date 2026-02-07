
//------------------------------------------------------------------------------------------------
//! Tutorial fast travel map menu
class SCR_TutorialFastTravelMapMenuUI: ChimeraMenuBase
{
    protected SCR_MapEntity m_MapEntity;
	protected SCR_TutorialFastTravelSpinBox m_SpinboxComp;
	protected SCR_TutorialCourseTask m_SelectedTask;
	
	//------------------------------------------------------------------------------------------------
	protected void ConfirmSelection()
	{
		if (!m_SelectedTask)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.FastTravelToCourse(m_SelectedTask.m_eCourse);
		
		MenuManager menuMan = GetGame().GetMenuManager();
		if (menuMan)
			menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel).Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SelectTask(SCR_TutorialCourseTask task)
	{
		if (!task)
			return;
		
		if (m_SelectedTask)
			m_SelectedTask.SetSelected(false);
		
		m_SelectedTask = task;
		m_SelectedTask.SetSelected(true);
		
		vector position = task.GetOrigin();
		if (m_MapEntity)
			m_MapEntity.ZoomPanSmooth(1.5, position[0], position[2]);
		
		m_SpinboxComp.SetCurrentItem(m_SpinboxComp.GetItemIndex(task));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{	
		if (!m_MapEntity)
			return;
		
		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.TUTORIALFASTTRAVEL, "{727AAAACDCBB4EFE}Configs/Map/TutorialFastTravelMap.conf", GetRootWidget());
		m_MapEntity.OpenMap(mapConfigFullscreen);
			
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		
		SetupSpinbox();
		
		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;
		
		inputMan.AddActionListener("TutorialFastTravelMapMenuClose", EActionTrigger.DOWN, Close);
		inputMan.AddActionListener("TutorialFastTravelMapMenuConfirm", EActionTrigger.DOWN, ConfirmSelection);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupTaskClicks()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);
		
		SCR_TutorialCourseTask courseTask;
		foreach (SCR_BaseTask task : tasks)
		{			
			courseTask = SCR_TutorialCourseTask.Cast(task);
			if (!courseTask)
				continue;
			
			ScriptInvoker invoker = courseTask.GetOnWidgetSet();
			if (invoker)
				invoker.Insert(AddTaskClickInvoker);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTaskClick(SCR_TutorialCourseTask task)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;
		
		if (inputMan.IsUsingMouseAndKeyboard())
		{
			tutorial.FastTravelToCourse(task.m_eCourse);
		
			MenuManager menuMan = GetGame().GetMenuManager();
			if (menuMan)
				menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel).Close();
			
			return;
		}
		
		SelectTask(task);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddTaskClickInvoker(SCR_TutorialCourseTask task)
	{
		if (!task)
			return;
		
		task.GetOnWidgetSet().Remove(AddTaskClickInvoker);
		task.GetOnClicked().Insert(OnTaskClick);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSpinbox()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("FastTravelSpinBox");
		if (!spinBox)
			return;
		
		m_SpinboxComp = SCR_TutorialFastTravelSpinBox.Cast(spinBox.FindHandler(SCR_TutorialFastTravelSpinBox));
		if (!m_SpinboxComp)
			return;
		
		SCR_BaseTaskManager taskMan = GetTaskManager();
		if (!taskMan)
			return;
		
		//TODO> FIND LESS UGLY WAY TO HIDE TASKS
		Faction faction = GetGame().GetFactionManager().GetFactionByKey("US");
		
		array <SCR_BaseTask> tasks = {};
		taskMan.GetTasks(tasks);
		
		foreach (SCR_BaseTask task : tasks)
		{
			if (task.GetTargetFaction() == faction)
				m_SpinboxComp.AddItem(task.GetTitle(), false, task);
		}
		
		m_SpinboxComp.m_OnChanged.Insert(OnSpinboxChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSpinboxChanged()
	{
		SelectTask(SCR_TutorialCourseTask.Cast(m_SpinboxComp.GetCurrentItemData()));
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		
		m_MapEntity.ZoomPanSmooth(0.35, m_MapEntity.GetMapSizeX()/2, m_MapEntity.GetMapSizeY()/2);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_OPEN);
		
		SetupTaskClicks();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		if (m_MapEntity)
		{
			m_MapEntity.CloseMap();
			
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE);
		}
		
		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;
		
		inputMan.RemoveActionListener("TutorialFastTravelMapMenuClose", EActionTrigger.DOWN, Close);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{		
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
}