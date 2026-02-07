//------------------------------------------------------------------------------------------------
class SCR_TaskDestroyObjectClass: CP_TaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskDestroyObject : CP_Task
{	
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if ( state != EDamageState.DESTROYED )
			return;
		
		ScriptedDamageManagerComponent pObjectDmgManager = ScriptedDamageManagerComponent.Cast( m_pAsset.FindComponent( ScriptedDamageManagerComponent ) );
		if ( pObjectDmgManager )
	 		pObjectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_pSupportEntity.FinishTask( this );				
	}
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if ( !GetTaskManager().FindSupportEntity( SCR_CP_TaskDestroySupportEntity ) )
		{
			Print( "CP: Task Destroy support entity not found in the world, task won't be created!" );
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDestroySupportEntity.Cast( GetTaskManager().FindSupportEntity( SCR_CP_TaskDestroySupportEntity ) );
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		if ( !m_pAsset )
			return;		
		ScriptedDamageManagerComponent pObjectDmgManager = ScriptedDamageManagerComponent.Cast( m_pAsset.FindComponent( ScriptedDamageManagerComponent ) );
		if ( pObjectDmgManager )
			pObjectDmgManager.GetOnDamageStateChanged().Insert( OnObjectDamage );
	}
}
