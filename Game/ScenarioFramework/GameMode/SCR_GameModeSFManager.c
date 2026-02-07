[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeSFManagerClass : SCR_BaseGameModeComponentClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
enum CP_EActivationType
{
	SAME_AS_PARENT = 0,
	ON_TRIGGER_ACTIVATION,
	ON_AREA_TRIGGER_ACTIVATION,
	ON_INIT,						//when the game mode is initiated
	ON_TASKS_INIT,					//when the  game mode starts creating tasks
	CUSTOM1,						//won't spawn until something will try to spawn the object with CUSTOM as parameter
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
};

enum ESFTaskType
{
	NONE,
	DELIVER,
	DESTROY,
	CLEAR_AREA,
	LAST,
	EXTRACTION,
	DEFAULT
};

class SCR_GameModeSFManager : SCR_BaseGameModeComponent
{	
	[Attribute( "Available Tasks for the Scenario" )];
	protected ref array<ref CP_TaskType> m_aTaskTypesAvailable;
	
	[Attribute( desc: "Name of the Area which will be only one to spawn", category: "Debug" )];
	protected string			m_sForcedArea;
	[Attribute( desc: "Name of the task layer to be only one to create", category: "Debug" )];
	protected string			m_sForcedTaskLayer;
	
	protected ref array<CP_Area> m_aAreas = {};		//all areas will be registered into this array
	protected ref ScriptInvoker m_pOnAllAreasInitiated;
	protected ref ScriptInvoker m_pOnTaskStateChanged;
	protected SCR_BaseTask		m_pExtractionAreaTask;
	protected SCR_BaseTask		m_pLastFinishedTask;
	protected CP_LayerBase		m_pLastFinishedTaskLayer;
	protected bool				m_bInitialized = false;
	
	//------------------------------------------------------------------------------------------------
	[RplRpc( RplChannel.Reliable, RplRcver.Broadcast )]
	void RpcDo_PlaySoundOnEntity( EntityID pEntID, string sSndName )
	{
		if ( !pEntID )
			return;		
		IEntity pEnt = GetGame().GetWorld().FindEntityByID( pEntID );
		if ( !pEnt )
			return;
		SCR_CommunicationSoundComponent pSndComp = SCR_CommunicationSoundComponent.Cast( pEnt.FindComponent( SCR_CommunicationSoundComponent ) );
		if ( !pSndComp )
			return;
		pSndComp.PlayStr( sSndName );
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnEntity( IEntity pEnt, string sSndName )
	{
		if ( !pEnt )
			pEnt = GetOwner();		//play it on game mode if any entity is passed

		if ( !pEnt )
			return;
		if ( IsMaster() )
			Rpc( RpcDo_PlaySoundOnEntity, pEnt.GetID(), sSndName );
		
		RpcDo_PlaySoundOnEntity( pEnt.GetID(), sSndName );		
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnPlayer( string sSndName )
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!pc)
			return;
		IEntity player = pc.GetMainEntity();
		
		if (!player)
			return;
		
		PlaySoundOnEntity( player, sSndName );
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask GetLastFinishedTask() { return m_pLastFinishedTask; }
	
	//------------------------------------------------------------------------------------------------
	CP_LayerBase GetLastFinishedTaskLayer() { return m_pLastFinishedTaskLayer; }
	
	//------------------------------------------------------------------------------------------------
	void OnTaskFinished( SCR_BaseTask pTask )
	{
		//CP_Task.Cast( pTask ).ShowPopUpMessage( "#AR-Tasks_StatusFinished-UC" );
		//GetOnTaskStateChanged().Invoke( pTask ); 
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskCreated( SCR_BaseTask pTask )
	{
		PopUpMessage( pTask.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC" );
		//GetOnTaskStateChanged().Invoke( pTask );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskCancelled( SCR_BaseTask pTask )
	{
		//GetOnTaskStateChanged().Invoke( pTask );
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnTaskFailed( SCR_BaseTask pTask )
	{
		//GetOnTaskStateChanged().Invoke( pTask );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskUpdate(SCR_BaseTask pTask, SCR_ETaskEventMask mask)
	{
		if ( !pTask ) 
			return;
		if ( pTask.GetTaskState() == SCR_TaskState.FINISHED )
		{
			m_pLastFinishedTaskLayer = CP_Task.Cast( pTask ).GetTaskLayer(); 
			m_pLastFinishedTask = pTask;
		}
		
		if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
		{
			PopUpMessage(pTask.GetTitle(), "#AR-Workshop_ButtonUpdate");
			
			CP_LayerTask pTaskLayer = CP_Task.Cast(pTask).GetTaskLayer();
			CP_SlotTask pSubject = pTaskLayer.GetTaskSubject();
			if (pSubject)
				pSubject.OnTaskStateChanged(SCR_TaskState.UPDATED)
		}
		
		//CP_Task.Cast( pTask ).ShowPopUpMessage( "#AR-Tasks_Objective" + " " + "#AR-Workshop_ButtonUpdate" );		//TODO: localize properly
		GetOnTaskStateChanged().Invoke(pTask, mask);
	}
			
	//------------------------------------------------------------------------------------------------
	void Finish()
	{
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_TIMELIMIT, 0,0);
		SCR_BaseGameMode.Cast( GetOwner() ).EndGameMode( endData );
		if (RplSession.Mode() == RplMode.Dedicated)
		{
			GetGame().GetCallqueue().CallLater( QuitGameMode, 12000 );	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void QuitGameMode()
	{
		GetGame().RequestClose();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiated()
	{
		if ( !m_pOnAllAreasInitiated )
			m_pOnAllAreasInitiated = new ScriptInvoker();
		
		return m_pOnAllAreasInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnTaskStateChanged()
	{
		if ( !m_pOnTaskStateChanged )
			m_pOnTaskStateChanged = new ScriptInvoker();
		
		return m_pOnTaskStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	CP_Area SelectNearestAreaByTaskType( ESFTaskType eTaskType )
	{
		if ( m_aAreas.IsEmpty() )
			return null;
		CP_Area pSelectedArea = null;
		if ( !m_pLastFinishedTask )
			return null;
		vector vTaskPos = m_pLastFinishedTask.GetOrigin();
		float fMinDistance = float.MAX;
		float fDistance = 0;
		for ( int i = 0; i < m_aAreas.Count(); i++ )
		{
			if ( !m_aAreas[i].GetIsTaskSuitableForArea( eTaskType ) )
				continue;
			
			fDistance = vector.Distance( vTaskPos, m_aAreas[i].GetOwner().GetOrigin() );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;
				pSelectedArea = m_aAreas[i];
			}
		}
		return pSelectedArea;
	}

	
	//------------------------------------------------------------------------------------------------	
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if ( !comp )
			return false;			//by purpose - debug
		return !comp.IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterArea( CP_Area pArea )
	{
		if ( m_aAreas.Find( pArea ) == -1 )
			m_aAreas.Insert( pArea );
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		Print( "CP: ------------------ player connected ---------------" );
		if (!IsMaster())
			return false;
		Print( "CP: ------------------ server only exec ---------------" );
		// Spawn everything inside the Area except the task layers
		foreach( CP_Area pArea : m_aAreas )
		{
			if ( m_sForcedArea.IsEmpty() )		//for debug purposes
			{
				pArea.Init();	
			}
			else
			{
				if ( pArea.GetOwner().GetName() == m_sForcedArea )
					pArea.Init( pArea );
			}
		}
				
		SCR_BaseTaskManager.s_OnTaskFinished.Insert( OnTaskFinished );		
		SCR_BaseTaskManager.s_OnTaskCreated.Insert( OnTaskCreated );	
		SCR_BaseTaskManager.s_OnTaskCancelled.Insert( OnTaskCancelled );
		SCR_BaseTaskManager.s_OnTaskFailed.Insert( OnTaskFailed );
		SCR_BaseTaskManager.s_OnTaskUpdate.Insert( OnTaskUpdate );
		
		//if someone registered for the event, then call it
		if ( m_pOnAllAreasInitiated )
			m_pOnAllAreasInitiated.Invoke();
		
		PostInit();
		
		m_bInitialized = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PostInit();
	
	//------------------------------------------------------------------------------------------------
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	protected void GenerateTasks();
	
	//------------------------------------------------------------------------------------------------
	// Mainly for debug purposes
	protected void GenerateSingleTask();
	
	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	CP_Area GetParentArea( IEntity pChild ) 
	{ 
		if ( !pChild )
			return null;
		CP_Area pLayer;
		IEntity pEnt = pChild.GetParent();
		while ( pEnt )
		{
			pLayer = CP_Area.Cast( pEnt.FindComponent( CP_Area ) );
			if ( pLayer )
				return pLayer;
			
			pEnt = pEnt.GetParent();
		}
		
		return pLayer;
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE,false);
		GetGame().GetCallqueue().CallLater(Init,1000,false); //TODO: make the init order properly ( the init should start after all Areas are registered )
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PopUpMessage( string sTitle, string sSubtitle )
	{
		if ( IsMaster() )
			SCR_PopUpNotification.GetInstance().PopupMsg( sTitle, text2: sSubtitle );
		
		Rpc( RpcDo_PopUpMessage, sTitle, sSubtitle );
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc( RplChannel.Reliable, RplRcver.Broadcast )]
	void RpcDo_PopUpMessage( string sTitle, string sSubtitle )
	{
		SCR_PopUpNotification.GetInstance().PopupMsg( sTitle, text2: sSubtitle );
	}
	
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//GetGame().GetCallqueue().CallLater(Init,2500,false);
	}
}
