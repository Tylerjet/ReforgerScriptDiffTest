[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotPickClass : CP_SlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class CP_SlotPick : CP_SlotTask
{
	
	//TODO: make title and description as Tuple2
	[Attribute( defvalue: "", desc: "Name of the task in list of tasks ( item picked up  )", category: "Task" )]		
	protected string 		m_sTaskTitleUpdated1;
	
	[Attribute( defvalue: "", desc: "Description of the task ( item picked up )", category: "Task",  )]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated1;
	
	[Attribute( defvalue: "", desc: "Name of the task in list of tasks ( item dropped )", category: "Task" )]		
	protected string 		m_sTaskTitleUpdated2;
	
	[Attribute( defvalue: "", desc: "Description of the task ( item dropped )", category: "Task",  )]
	protected string 		m_sTaskDescriptionUpdated2;
	
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskTitle( int iState = 0 ) 
	{ 
		if ( iState == 0 )
			return super.GetTaskTitle();
		else if ( iState == 1 )
			return m_sTaskTitleUpdated1;	
		else if ( iState == 2 )
			return m_sTaskTitleUpdated2;
		else if ( iState == 4 )
			return super.GetTaskTitle();
		else if ( iState == 5 )
			return m_sTaskTitleUpdated1;
		else if ( iState == 6 )
			return m_sTaskTitleUpdated2; 
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskDescription( int iState = 0 )	
	{ 
		if ( iState == 0 )
			return super.GetTaskDescription();
		else if ( iState == 1 )
			return m_sTaskDescriptionUpdated1;	
		else if ( iState == 2 )
			return m_sTaskDescriptionUpdated2;
		else if ( iState == 4 )
			return super.GetTaskDescription();
		else if ( iState == 5 )
			return m_sTaskDescriptionUpdated1;
		else if ( iState == 6 )
			return m_sTaskDescriptionUpdated2; 
		return string.Empty;
	}	
	
	
	//------------------------------------------------------------------------------------------------
	override void Init( CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true )
	{
		if ( m_EActivationType != EActivation )
			return;
		super.Init( pArea, EActivation );
		CP_LayerTaskDeliver pLayer = CP_LayerTaskDeliver.Cast( GetParentTaskLayer() );
		if ( !pLayer )
			return;
		string sTaskName;
		CP_Task pTask = pLayer.GetTask();
		if ( !pTask )
			sTaskName = pLayer.GetOwner().GetName();
		else
			sTaskName = pTask.GetTitleText();
		
		if ( m_pEntity && m_pEntity.GetPrefabData() )
			PrintFormat( "CP: ->Task: The %1 is set as the subject of delivery for task %2.", m_pEntity.GetPrefabData().GetPrefabName(), sTaskName );
	}
					
	//------------------------------------------------------------------------------------------------
	void CP_SlotPick(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}	

	//------------------------------------------------------------------------------------------------
	void ~CP_SlotPick()
	{
	}
}
