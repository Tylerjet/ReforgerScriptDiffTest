[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskClass : CP_LayerBaseClass
{
	// prefab properties here
}


class CP_LayerTask : CP_LayerBase
{		
		
	[Attribute( defvalue: "", desc: "Name of the task in list of tasks", category: "Task" )]		
	protected string 		m_sTaskTitle;
	
	[Attribute( defvalue: "", desc: "Description of the task", category: "Task",  )]			//TODO: make config, memory
	protected string 		m_sTaskDescription;

	//[Attribute( "Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum( ESFTaskType ), category: "Task" )];
	protected ESFTaskType 	m_eTypeOfTask = ESFTaskType.DEFAULT;
	
	
	[Attribute( defvalue: "", desc: "Task prefab", category: "Task" )]		
	protected ResourceName	m_sTaskPrefab;
	
	[Attribute( defvalue: "0", desc: "Marker on map is placed directly on the task subject Slot or on layer Slot", category: "Task" )]		
	protected bool			m_bPlaceMarkerOnSubjectSlot;
	
	protected CP_Task	m_pTask;
	protected SCR_CP_TaskSupportEntity m_pSupportEntity;
	
	static ref ScriptInvoker s_OnTaskSetup = new ref ScriptInvoker();
		
	//------------------------------------------------------------------------------------------------
	CP_Task GetTask() { return m_pTask; }
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetTaskPrefab() { return m_sTaskPrefab; }
	
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle() { return m_sTaskTitle; }
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription()
	{ 
		if (!m_pEntity)
			return m_sTaskDescription;
		
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(m_pEntity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return m_sTaskDescription;
		
		return string.Format(WidgetManager.Translate(m_sTaskDescription, editableEntityComp.GetDisplayName()));
	}	
	
	//------------------------------------------------------------------------------------------------
	ESFTaskType GetTaskType() { return m_eTypeOfTask; }
		
	protected CP_SlotTask	m_pTaskSubject; 				//storing this one in order to get the task title and description
	
	[Attribute( UIWidgets.Auto, category: "OnTaskFinished" )];
	protected ref array<ref CP_ActionBase>	m_aTriggerActionsOnFinish;
		
		
	//------------------------------------------------------------------------------------------------
	override void Init( CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true )
	{
		if ( m_EActivationType != EActivation )
			return;
 		super.Init( pArea, EActivation );
		if ( !GetTaskManager() )
		{
			Print( "CP: Task manager not found in the world, any task won't be created!" );
			return;
		}
		if ( SetSupportEntity() )
			if( SetTaskPrefab() )
				if ( CreateTask() )
				{
					SetupTask();
					m_pTask.SetTaskLayer( this );
					if ( m_pTaskSubject ) 	
						m_pTaskSubject.OnTaskStateChanged( SCR_TaskState.OPENED );
				}
		
//		SCR_GameModeSFManager pGameModeComp = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		//pGameModeComp.GetOnTaskStateChanged().Invoke( m_pTask );	//Calling this again since the Task Layer is stored into the task after the Task Manager invokes On Task Changed event. And we need to inform other consequent listeners about the task layer name.
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged( SCR_TaskState previousState, SCR_TaskState newState )
	{
		if ( newState == SCR_TaskState.FINISHED )
		{
			foreach( CP_ActionBase pTrigAction : m_aTriggerActionsOnFinish )
				pTrigAction.OnActivate( GetOwner()	 );
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool SetTaskPrefab()
	{
		if ( !m_pSupportEntity.GetTaskPrefab() )
		{
			if ( m_sTaskPrefab.IsEmpty() )
			{
				Print( "CP: Task prefab not set, task won't be created!" );
				return false;
			}
			m_pSupportEntity.SetTaskPrefab( m_sTaskPrefab );
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskSubject( CP_SlotTask pTaskSubject )
	{
		m_pTaskSubject = pTaskSubject;
	}
	
	//------------------------------------------------------------------------------------------------
	CP_SlotTask GetTaskSubject() { return m_pTaskSubject; }
	
	//------------------------------------------------------------------------------------------------
	protected bool SetSupportEntity()
	{
		if ( !GetTaskManager().FindSupportEntity( SCR_CP_TaskSupportEntity ) )
		{
			Print( "CP: Default Task support entity not found in the world, task won't be created!" );
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskSupportEntity.Cast( GetTaskManager().FindSupportEntity( SCR_CP_TaskSupportEntity ) );
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CreateTask()
	{
		m_pTask = CP_Task.Cast( m_pSupportEntity.CreateTask( this ) );			
		if ( !m_pTask )
		{
			PrintFormat( "CP: Creating of task %1 failed! Task manager refused to create it.", m_sTaskTitle );
			return false;
		}
		//TODO: make selection of the faction dynamic
		array<Faction> aPlayableFactions = {};
		SCR_Faction pFactionSelected;
		FactionManager pManager = GetGame().GetFactionManager();
		if ( !pManager )
		{
			PrintFormat( "CP: Creating of task %1 failed! Faction manager doesn't exist", m_sTaskTitle );
			return false;
		}
		
		pManager.GetFactionsList( aPlayableFactions );
		foreach( Faction pFaction : aPlayableFactions )
		{
			if ( !SCR_Faction.Cast( pFaction ) )
				continue;
			if ( SCR_Faction.Cast( pFaction ).IsPlayable() )
			{
				pFactionSelected = SCR_Faction.Cast( pFaction );
				break;
			}
		}
		
		if ( !pFactionSelected )
		{
			PrintFormat( "CP: Creating of task %1 failed! No playable faction available", m_sTaskTitle );
			return false;
		}
				
		m_pSupportEntity.SetTargetFaction( m_pTask, pFactionSelected );
		vector vOrigin;
		if ( m_bPlaceMarkerOnSubjectSlot )
			vOrigin = m_pTaskSubject.GetOwner().GetOrigin();
		else
			vOrigin = GetOwner().GetOrigin();
		m_pSupportEntity.MoveTask( vOrigin, m_pTask.GetTaskID() );
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupTask()
	{
		//if title and description is provided, fill it in
		if ( !m_sTaskTitle.IsEmpty() ) 
			m_pTask.SetTitle( GetTaskTitle() );
		if ( !m_sTaskDescription.IsEmpty() ) 
			m_pTask.SetDescription( GetTaskDescription() );
		
		//if title and description is provided on the Task Subject, fill it in ( overrides Layer Task )
		if ( m_pTaskSubject )
		{
			if ( !m_pTaskSubject.GetTaskTitle().IsEmpty() )
				m_pTask.SetTitle( m_pTaskSubject.GetTaskTitle() );
			if ( !m_pTaskSubject.GetTaskDescription().IsEmpty() )
				m_pTask.SetDescription( m_pTaskSubject.GetTaskDescription() );
		}
		else
		{
			Print( "CP: ->Task: Task Subject not found doesn't exist" );
		}
		
		s_OnTaskSetup.Invoke(m_pTask);
		
		PrintFormat( "CP: ->Task: Generating task %1", m_pTask.GetTitleText() );
	}

		
	//------------------------------------------------------------------------------------------------
	void CP_LayerTask(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	} 

	//------------------------------------------------------------------------------------------------
	void ~CP_LayerTask()
	{
	}
}
