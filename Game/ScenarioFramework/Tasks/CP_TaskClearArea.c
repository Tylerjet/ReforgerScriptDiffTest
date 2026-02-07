//------------------------------------------------------------------------------------------------
class CP_TaskClearAreaClass: CP_TaskAreaClass
{
};


//------------------------------------------------------------------------------------------------
class CP_TaskClearArea : CP_TaskArea
{	
	//------------------------------------------------------------------------------------------------
	override void OnTriggerActivated()
	{
		m_pSupportEntity.FinishTask( this );	
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if ( !GetTaskManager().FindSupportEntity( SCR_CP_TaskClearAreaSupportEntity ) )
		{
			Print( "CP: Task Clear area support entity not found in the world, task won't be created!" );
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskClearAreaSupportEntity.Cast( GetTaskManager().FindSupportEntity( SCR_CP_TaskClearAreaSupportEntity ) );
		return m_pSupportEntity != null;	
	}
}
