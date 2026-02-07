//------------------------------------------------------------------------------------------------
class SCR_TaskKillClass: CP_TaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskKill : CP_Task
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
		if ( !GetTaskManager().FindSupportEntity( SCR_CP_TaskKillSupportEntity ) )
		{
			Print( "CP: Task Kill support entity not found in the world, task won't be created!" );
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskKillSupportEntity.Cast( GetTaskManager().FindSupportEntity( SCR_CP_TaskKillSupportEntity ) );
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
