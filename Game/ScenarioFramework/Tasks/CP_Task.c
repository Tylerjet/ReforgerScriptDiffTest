//------------------------------------------------------------------------------------------------
class CP_TaskClass: SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class CP_Task : SCR_BaseTask
{	
	protected CP_Area 								m_pArea;
	protected IEntity 								m_pAsset;
	protected SCR_CP_TaskSupportEntity		 		m_pSupportEntity;
	protected CP_LayerTask							m_pLayer;
	protected bool									m_bInitiated = false;
		
	//------------------------------------------------------------------------------------------------
	//!		
	/*
	void ShowPopUpMessage( string subtitle )
	{
		if ( !m_bInitiated )
			return;
		
		SCR_GameModeSFManager pGameModeManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pGameModeManager )
			return;		
		else
		{
			pGameModeManager.PopUpMessage( GetTitle(), subtitle );
		}
		//if ( m_bInitiated )
			//SCR_PopUpNotification.GetInstance().PopupMsg( GetTitle(), text2: subtitle );
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void SetTaskLayer( CP_LayerTask pLayer )
	{
		m_pLayer = pLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	CP_LayerTask GetTaskLayer() { return m_pLayer; }
	
	//------------------------------------------------------------------------------------------------
	//! An event called when the state of this task has been changed. 
	override void OnStateChanged( SCR_TaskState previousState, SCR_TaskState newState )
	{
		//super.OnStateChanged( previousState, newState );
		
		SCR_GameModeSFManager pGameModeManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pGameModeManager )
			return;
		
		pGameModeManager.PopUpMessage( GetTitle(), GetTitle() );
				
		if ( !pGameModeManager.IsMaster() )
			return;
		
		if ( !m_pLayer )
			return;
		m_pLayer.GetTaskSubject().OnTaskStateChanged( newState );				
		m_pLayer.OnTaskStateChanged( previousState, newState );
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void SetTaskSubject( IEntity pObj )
	{
		m_pAsset = pObj;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTaskSubject() { return m_pAsset; }	
	
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
	void Init()
	{
		
		if (SCR_Global.IsEditMode(this))
			return;
		
		//m_pSupportEntity = SCR_CP_TaskSupportEntity.Cast( GetTaskManager().FindSupportEntity( SCR_CP_TaskSupportEntity ) );
		SetSupportEntity();
		
		m_pAsset = m_pSupportEntity.GetTaskEntity();
		if ( !m_pAsset )
		{
			if ( m_pSupportEntity )
				m_pSupportEntity.CancelTask( this.GetTaskID() );	
			Print( "CP: Task subject not found!" );
	 		return;
		}
		
		if ( !m_pSupportEntity )
			return;
		
		SCR_GameModeSFManager pGameModeManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pGameModeManager )
			return;		
				
		m_bInitiated = true;
	}

	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
    {
        super.EOnInit( owner );
		if ( !GetTaskManager() )
			return;
		if ( GetTaskManager().IsProxy() )
			return;
		Init();
    }
	
	//------------------------------------------------------------------------------------------------
	void CP_Task(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~CP_Task()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
	}
	
}
