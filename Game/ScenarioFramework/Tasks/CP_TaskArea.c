//------------------------------------------------------------------------------------------------
class CP_TaskAreaClass: CP_TaskClass
{
};


//------------------------------------------------------------------------------------------------
class CP_TaskArea : CP_Task
{	
	protected SCR_BaseTriggerEntity 		m_pTrigger;
		
	//------------------------------------------------------------------------------------------------
	override void Init()
	{	
		super.Init();
		m_pTrigger = SCR_BaseTriggerEntity.Cast( m_pAsset );
		if ( !m_pTrigger )
		{
			if ( !m_pSupportEntity )
				return;
			m_pSupportEntity.CancelTask( this.GetTaskID() );
			Print( "CP: Trigger not found! Make sure the IDs match in both task and Slot component", LogLevel.WARNING );
	 		return;
		}
		m_pTrigger.GetOnActivate().Insert( OnTriggerActivated );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTriggerActivated()
	{
		m_pSupportEntity.FinishTask( this );	
	}	

}
